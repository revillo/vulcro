#include "VulkanContext.h"
#include "VulkanSetLayout.h"
#include "VulkanVertexLayout.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"
#include "VulkanTask.h"
#include "VulkanTaskPool.h"
#include "VulkanTaskGroup.h"
#include "VulkanSet.h"
#include "VulkanImage.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "../vulkan-rtx/RTPipeline.h"
#include "../vulkan-rtx/RTAccelerationStructure.h"

VulkanContext::VulkanContext(vk::Instance instance, vk::PhysicalDevice& pDevice, const std::vector<const char *> & deviceExtensions)
	:_instance(instance),
    _physicalDevice(pDevice)
{

	auto qfps = pDevice.getQueueFamilyProperties();

	int familyIndex = -1;

    for (uint32_t i = 0; i < qfps.size(); ++i)
    {
		if (qfps[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {

			familyIndex = i;
			break;
		}
	}

    _queueCount = qfps[familyIndex].queueCount;

	_familyIndex = familyIndex;

	std::vector<const char*> extensions;

	auto es = pDevice.enumerateDeviceExtensionProperties();
	std::unordered_map<std::string, bool> extensionLookup;

	for (auto & e : es) {
		//std::cout << e.extensionName << std::endl;
		extensionLookup[e.extensionName] = true;
	}


	auto addExtensionSafe = [&](const char * extensionName)
    {
		if (extensionLookup[extensionName])
        {
			extensions.push_back(extensionName);
        }
        else
        {
            std::cout << "Warning: " << extensionName << " is not supported on this device!" << std::endl;
        }
	};

	for (auto * ext : deviceExtensions)
    {
		addExtensionSafe(ext);
	}

    auto features = vk::PhysicalDeviceFeatures();

	features.setTessellationShader(true);
	features.setGeometryShader(true);
	features.setShaderStorageImageExtendedFormats(true);
	features.setFragmentStoresAndAtomics(true);
	features.setVertexPipelineStoresAndAtomics(true);
    features.setShaderUniformBufferArrayDynamicIndexing(true);

    auto features2 = vk::PhysicalDeviceFeatures2();
    features2.setFeatures(features);

    auto indexingFeatures = vk::PhysicalDeviceDescriptorIndexingFeaturesEXT();
    indexingFeatures.setRuntimeDescriptorArray(true);
    indexingFeatures.setDescriptorBindingPartiallyBound(true);
    indexingFeatures.setDescriptorBindingVariableDescriptorCount(true);
    
    indexingFeatures.setPNext(nullptr);


    features2.setPNext(&indexingFeatures);


	float qpriors[1] = { 0.0f };

	auto devQ = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), familyIndex, 1, qpriors);

    vk::DeviceCreateInfo devCreateInfo;
    devCreateInfo.flags = vk::DeviceCreateFlags();
    devCreateInfo.pNext = &features2;
    devCreateInfo.enabledLayerCount = 0;
    devCreateInfo.queueCreateInfoCount = 1;
    devCreateInfo.pQueueCreateInfos = &devQ;
    devCreateInfo.ppEnabledLayerNames = nullptr;
    devCreateInfo.enabledExtensionCount = extensions.size();
    devCreateInfo.ppEnabledExtensionNames = &extensions[0];
    devCreateInfo.pEnabledFeatures = nullptr;
   

	_device = pDevice.createDevice(devCreateInfo);

    
    mPhysicalDeviceProperties = pDevice.getProperties();

    /*
    vk::DynamicLoader         dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    

	_dynamicDispatch = new vk::DispatchLoaderDynamic(_instance, vkGetInstanceProcAddr, _device);
	    */


    _dynamicDispatch = new vk::DispatchLoaderDynamic(_instance, _device);

    //_queue = _device.getQueue(familyIndex, 0);
    _queues.resize(_queueCount);

    for (int i = 0; i < _queueCount; i++)
    {
        _queues[i] = _device.getQueue(familyIndex, 0);
    }

    getLinearSampler();
    getShadowSampler();
    getNearestSampler();
}

vk::Sampler const & VulkanContext::getShadowSampler() {

    if (!_shadowSampler) {

        _shadowSampler = getDevice().createSampler(
            vk::SamplerCreateInfo(
                vk::SamplerCreateFlags(),
                vk::Filter::eLinear, //Mag Filter
                vk::Filter::eLinear, //Min Filter
                vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eClampToBorder, //U
                vk::SamplerAddressMode::eClampToBorder,  //V
                vk::SamplerAddressMode::eClampToBorder, //W
                0.0, //mip lod bias
                0, //Anisotropy Enable
                1.0f,
                VK_TRUE, //Compare Enable
                vk::CompareOp::eLessOrEqual,
                0.0f, //min lod
                100.0f, //max lod
                vk::BorderColor::eFloatOpaqueWhite,
                0 //Unnormalized Coordinates

            )
        );
    }

    return _shadowSampler;
}

vk::Sampler const & VulkanContext::getLinearSampler()
{		
	if (!_linearSampler) {
		_linearSampler = createSampler2D(vk::Filter::eLinear);
	}

	return _linearSampler;
	
}


vk::Sampler const & VulkanContext::getNearestSampler()
{
	if (!_nearestSampler) {
		_nearestSampler = createSampler2D(vk::Filter::eNearest);
	}

	return _nearestSampler;
}




VulkanContext::~VulkanContext()
{
	if (_linearSampler) getDevice().destroySampler(_linearSampler);
	if (_nearestSampler) getDevice().destroySampler(_nearestSampler);
	if (_shadowSampler) getDevice().destroySampler(_shadowSampler);

    mOneTimePool = nullptr;

	_device.destroy();
	
	delete _dynamicDispatch;

}

vk::Sampler VulkanContext::createSampler2D(vk::Filter filter)
{
    return getDevice().createSampler(
        vk::SamplerCreateInfo(
            vk::SamplerCreateFlags(),
            filter, //Mag Filter
            filter, //Min Filter
            filter == vk::Filter::eNearest ? vk::SamplerMipmapMode::eNearest : vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat, //U
			vk::SamplerAddressMode::eRepeat,  //V
			vk::SamplerAddressMode::eRepeat, //W
			0.0, //mip lod bias
			0, //Anisotropy Enable
			1.0f,
			0, //Compare Enable
			vk::CompareOp::eAlways,
			0.0f, //min lod
			100.0f, //max lod
			vk::BorderColor::eFloatOpaqueBlack,
			0 //Unnormalized Coordinates

		)
	);
}


/// Helpers




VulkanSetLayoutRef VulkanContext::makeSetLayout(vk::ArrayProxy<const VulkanSetLayoutBinding> bindings, uint32_t maxSets)
{
	return make_shared<VulkanSetLayout>(this, bindings, maxSets);
}

shared_ptr<VulkanVertexLayout> VulkanContext::makeVertexLayout(vk::ArrayProxy<const vk::Format> fields)
{
	return make_shared<VulkanVertexLayout>(fields);
}

shared_ptr<VulkanRenderer> VulkanContext::makeRenderer()
{
	return make_shared<VulkanRenderer>(this);
}

shared_ptr<VulkanRenderPipeline> VulkanContext::makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer, PipelineConfig config,
	vector<ColorBlendConfig> colorBlendConfigs, uint32_t pushConstantSize)
{
	return make_shared<VulkanRenderPipeline>(this, shader, renderer, config, colorBlendConfigs, pushConstantSize);
}

VulkanComputePipelineRef VulkanContext::makeComputePipeline(VulkanShaderRef shader, uint32_t pushConstantSize)
{
	return make_shared<VulkanComputePipeline>(this, shader, pushConstantSize);
}

VulkanComputePipelineRef VulkanContext::makeComputePipeline(const char * computePath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts, uint32_t pushConstantSize)
{
	return makeComputePipeline(
		makeComputeShader(computePath, setLayouts), pushConstantSize
	);
}


VulkanSwapchainRef VulkanContext::makeSwapchain(vk::SurfaceKHR surface)
{
	return make_shared<VulkanSwapchain>(this, surface);
}

VulkanShaderRef VulkanContext::makeShader(const char * vertPath, const char * fragPath, vk::ArrayProxy<const VulkanVertexLayoutRef>vertexLayouts, vk::ArrayProxy<const VulkanSetLayoutRef> uniformLayouts)
{
	return make_shared<VulkanShader>(this, vertPath, fragPath, vertexLayouts, uniformLayouts);
}

VulkanShaderRef VulkanContext::makeTessShader(const char * vertPath, const char * tessControlPath, const char * tessEvalPath, const char * tessGeomPath, const char * fragPath, vk::ArrayProxy<const VulkanVertexLayoutRef> vertexLayouts, vk::ArrayProxy<const VulkanSetLayoutRef> uniformLayouts)
{
	return make_shared<VulkanShader>(this, vertPath, tessControlPath, tessEvalPath, tessGeomPath, fragPath, vertexLayouts, uniformLayouts);
}

VulkanShaderRef VulkanContext::makeComputeShader(const char * computePath, vk::ArrayProxy<const VulkanSetLayoutRef> uniformLayouts)
{
	return make_shared<VulkanShader>(this, computePath, uniformLayouts);
}


shared_ptr<ibo> VulkanContext::makeIBO(vk::ArrayProxy<const VulkanBuffer::IndexType> indices)
{
	return make_shared<ibo>(this, indices);

}

shared_ptr<ibo> VulkanContext::makeIBO(shared_ptr<ibo> sourceIbo, uint32_t indexOffset, uint32_t numIndices)
{
	return make_shared<ibo>(sourceIbo, indexOffset, numIndices);
}

VulkanBufferRef VulkanContext::makeBuffer(vk::BufferUsageFlags usage, uint64_t size, vk::MemoryPropertyFlags flags, void * data)
{
	return make_shared<VulkanBuffer>(this, usage, size, flags, data);
}

VulkanBufferRef VulkanContext::makeDeviceBuffer(vk::BufferUsageFlags usage, uint64_t size, void * data)
{
    // If no data needs to be staged then just create the buffer
	if (data == nullptr)
    {
        return makeBuffer(usage, size, VulkanBuffer::CPU_NEVER);
	}

    // Allocate the gpu only buffer that we'll be returning, we add 'eTransferDst' as a usage flag
    // so that we can target this buffer as the destination when copying from the staging buffer.
    auto buffer = makeBuffer(usage | vk::BufferUsageFlagBits::eTransferDst, size, VulkanBuffer::CPU_NEVER);

    // Create a staging buffer that's cpu accessible, we'll use this buffer to copy our initial data into the
    // gpu only buffer
    auto staging = makeBuffer(vk::BufferUsageFlagBits::eTransferSrc, size, VulkanBuffer::CPU_ALOT, data);

    // A config describing the copy command, we'll be copying thfrom the start of data into the start
    // of the new buffer
    auto BufferCopy = vk::BufferCopy(0, 0, size);

    // Create a new task and record our copy command
    auto task = makeTask();
    task->record([&](vk::CommandBuffer* cmd)
    {
        cmd->copyBuffer(staging->getBuffer(), buffer->getBuffer(), { BufferCopy });
    });

    // Execute the copy task
    task->execute(true);

    return buffer;
}


VulkanBufferRef VulkanContext::makeStagingBuffer(vk::BufferUsageFlags usage, uint64_t sizeBytes, void* data)
{
    return makeBuffer(usage | vk::BufferUsageFlagBits::eTransferSrc, sizeBytes, VulkanBuffer::CPU_ALOT, data);
}


VulkanBufferRef VulkanContext::makeStagingStorageBuffer(uint64_t sizeBytes, void * data)
{
    return makeBuffer(VulkanBuffer::TRANSFER_SRC_STORAGE_BUFFER, sizeBytes, VulkanBuffer::CPU_ALOT);
}

VulkanBufferRef VulkanContext::makeDeviceStorageBuffer(uint64_t size, void * data)
{
    return makeDeviceBuffer(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, size, data);
}



VulkanSetRef VulkanContext::makeSet(VulkanSetLayoutRef layout)
{
	return make_shared<VulkanSet>(this, layout);
}

VulkanSetRef VulkanContext::makeSet(vk::ArrayProxy<const VulkanSetLayoutBinding> bindings)
{
	return make_shared<VulkanSet>(this, VulkanContext::makeSetLayout(bindings));

}

VulkanTaskRef VulkanContext::makeTask()
{

    if (mOneTimePool == nullptr)
    {
        mOneTimePool = makeTaskPool(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    }

    return makeTask(mOneTimePool);
}

VulkanTaskGroupRef VulkanContext::makeTaskGroup(uint32_t numTasks)
{
    if (mOneTimePool == nullptr)
    {
        mOneTimePool = makeTaskPool(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    }

    return VulkanTaskGroupRef(new VulkanTaskGroup(this, numTasks, mOneTimePool));
}

VulkanTaskGroupRef VulkanContext::makeTaskGroup(uint32_t numTasks, VulkanTaskPoolRef pool)
{
    return VulkanTaskGroupRef(new VulkanTaskGroup(this, numTasks, pool));
}

VulkanTaskPoolRef VulkanContext::makeTaskPool(vk::CommandPoolCreateFlags createFlags)
{
    return VulkanTaskPoolRef(new VulkanTaskPool(this, createFlags));
}

VulkanTaskRef VulkanContext::makeTask(VulkanTaskPoolRef taskPool)
{
    return VulkanTaskRef(new VulkanTask(this, taskPool));
}


VulkanImage1DRef VulkanContext::makeImage1D(vk::ImageUsageFlags usage, vk::Format format, float size)
{
	auto r = make_shared<VulkanImage1D>(this, usage, format, size);
	r->setSampler(getNearestSampler());
	return r;
}

VulkanImage1DRef VulkanContext::makeImage1D(vk::Image image, vk::Format format, float size)
{
	return make_shared<VulkanImage1D>(this, image, format, size);
}

VulkanImage2DRef VulkanContext::makeImage2D(vk::ImageUsageFlags usage, vk::Format format, glm::uvec2 size, vk::MemoryPropertyFlags memFlags)
{
	auto r = make_shared<VulkanImage2D>(this, usage, format, size, memFlags);
	r->setSampler(getNearestSampler());
	return r;
}

VulkanImage2DRef VulkanContext::makeImage2D(vk::ImageUsageFlags usage, vk::Format format, glm::uvec2 size, uint16_t mipLevels, vk::MemoryPropertyFlags memFlags)
{
	auto r = make_shared<VulkanImage2D>(this, usage, format, size, mipLevels, memFlags);
	r->setSampler(getNearestSampler());
	return r;
}

VulkanImage2DRef VulkanContext::makeImage2D(vk::Image image, vk::Format format, glm::uvec2 size, uint16_t mipLevels)
{
	return make_shared<VulkanImage2D>(this, image, format, size);
}

VulkanImage2DRef VulkanContext::makeTexture2D_RGBA(glm::uvec2 size, uint16_t mipLevels, void * pixelData)
{
    auto res = makeImage2D(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::Format::eR8G8B8A8Unorm, size, mipLevels);

    if (pixelData)
    {
        res->loadFromMemory(pixelData, size.x * size.y * sizeof(uint8_t) * 4);
    }

    return res;
}

VulkanImage3DRef VulkanContext::makeImage3D(vk::ImageUsageFlags usage, vk::Format format, glm::uvec3 size)
{
	auto r = make_shared<VulkanImage3D>(this, usage, format, size);
	r->setSampler(getNearestSampler());
	return r;
}

VulkanImage3DRef VulkanContext::makeImage3D(vk::Image image, vk::Format format, glm::uvec3 size)
{
	return make_shared<VulkanImage3D>(this, image, format, size);
}

VulkanImageCubeRef VulkanContext::makeImageCube(vk::ImageUsageFlags usage, glm::uvec2 size, vk::Format format)
{
	return make_shared<VulkanImageCube>(this, usage, size, format);
}

RTGeometryRepoRef VulkanContext::makeRayTracingGeometryRepo()
{
    return RTGeometryRepoRef(new RTGeometryRepo(this));
}

RTGeometryRef VulkanContext::makeRayTracingGeometry(iboRef indexBuffer, vboRef vertexBuffer)
{
	return make_shared<RTGeometry>(indexBuffer, vertexBuffer);
}

RTGeometryRef VulkanContext::makeRayTracingGeometry(uint64_t aabbCount, uint64_t aabbOffset, VulkanBufferRef aabbBuffer)
{
    return make_shared<RTGeometry>(aabbCount, aabbOffset, aabbBuffer);
}

RTGeometryRef VulkanContext::makeRayTracingGeometry(VulkanBufferRef vertexBuffer, uint64_t vertexCount, uint32_t vertexStride, uint32_t positionOffset, vk::Format positionFormat)
{
    return make_shared<RTGeometry>(vertexBuffer, vertexCount, vertexStride, positionOffset, positionFormat);
}

shared_ptr<RTShaderBuilder> VulkanContext::makeRayTracingShaderBuilder(const char * raygenPath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts)
{
	return make_shared<RTShaderBuilder>(this, raygenPath, setLayouts);
}

shared_ptr<RTPipeline> VulkanContext::makeRayTracingPipeline(RTShaderBuilderRef shader)
{
	return make_shared<RTPipeline>(this, shader);
}

shared_ptr<RTScene> VulkanContext::makeRayTracingScene()
{
	return make_shared<RTScene>(this);
}