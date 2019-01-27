#include "VulkanContext.h"

using namespace vk;

VulkanContext::VulkanContext(vk::Instance instance)
	:_instance(instance)
{
	//vk::HackyGlobalInstance = (VkInstance)instance;

	_physicalDevices = _instance.enumeratePhysicalDevices();

	auto qfps = _physicalDevices[0].getQueueFamilyProperties();

	int familyIndex = -1;

	for (uint32_t i = 0; i < qfps.size(); i++) {
		if (qfps[i].queueFlags & vk::QueueFlagBits::eGraphics) {

			familyIndex = i;
			break;
		}
	}

	_familyIndex = familyIndex;

	std::vector<const char*> extensions;

	auto es = _physicalDevices[0].enumerateDeviceExtensionProperties();
	std::unordered_map<std::string, bool> extensionLookup;

	for (auto & e : es) {
		//std::cout << e.extensionName << std::endl;
		extensionLookup[e.extensionName] = true;
	}


	auto addExtensionSafe = [&](const char * extensionName) {
		if (extensionLookup[extensionName]) {
			extensions.push_back(extensionName);
		}
	};

	addExtensionSafe("VK_KHR_swapchain");
	addExtensionSafe("VK_KHR_get_memory_requirements2");
	addExtensionSafe("VK_NV_ray_tracing");


	auto features = vk::PhysicalDeviceFeatures();

	features.setTessellationShader(true);
	features.setGeometryShader(true);
	features.setShaderStorageImageExtendedFormats(true);
	features.setFragmentStoresAndAtomics(true);
	features.setVertexPipelineStoresAndAtomics(true);

	float qpriors[1] = { 0.0f };

	auto devQ = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), familyIndex, 1, qpriors);

	_device = _physicalDevices[0].createDevice(
		vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1,
			&devQ,
			0, //Enabled Layers 
			nullptr, 
			static_cast<uint32_t>(extensions.size()), &extensions[0],
			&features //enabled features
		)
	);

	_dynamicDispatch = new vk::DispatchLoaderDynamic(_instance, _device);
	_queue = _device.getQueue(familyIndex, 0);
}

vk::Sampler VulkanContext::getShadowSampler() {

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
                0.0f, //max lod
                vk::BorderColor::eFloatOpaqueWhite,
                0 //Unnormalized Coordinates

            )
        );
    }

    return _shadowSampler;
}



vk::Sampler VulkanContext::getLinearSampler()
{		
	if (!_linearSampler) {
		_linearSampler = createSampler2D(vk::Filter::eLinear);
	}

	return _linearSampler;
	
}


vk::Sampler VulkanContext::getNearestSampler()
{
	if (!_nearestSampler) {
		_nearestSampler = createSampler2D(vk::Filter::eNearest);
	}

	return _nearestSampler;
}




VulkanContext::~VulkanContext()
{
	for (auto keyval : _pools)
		_device.destroyCommandPool(keyval.second);

	if (_linearSampler) getDevice().destroySampler(_linearSampler);
	if (_nearestSampler) getDevice().destroySampler(_nearestSampler);
	if (_shadowSampler) getDevice().destroySampler(_shadowSampler);

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
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eClampToEdge, //U
			vk::SamplerAddressMode::eClampToEdge,  //V
			vk::SamplerAddressMode::eClampToEdge, //W
			0.0, //mip lod bias
			0, //Anisotropy Enable
			1.0f,
			0, //Compare Enable
			vk::CompareOp::eAlways,
			0.0f, //min lod
			0.0f, //max lod
			vk::BorderColor::eFloatOpaqueBlack,
			0 //Unnormalized Coordinates

		)
	);
}


/// Helpers


#include "VulkanSetLayout.h"


VulkanSetLayoutRef VulkanContext::makeSetLayout(vk::ArrayProxy<const VulkanUniformLayoutBinding> bindings)
{
	return make_shared<VulkanSetLayout>(this, bindings);
}

#include "VulkanVertexLayout.h"
shared_ptr<VulkanVertexLayout> VulkanContext::makeVertexLayout(vk::ArrayProxy<const vk::Format> fields)
{
	return make_shared<VulkanVertexLayout>(fields);
}

#include "VulkanRenderer.h"
shared_ptr<VulkanRenderer> VulkanContext::makeRenderer()
{
	return make_shared<VulkanRenderer>(this);
}

#include "VulkanPipeline.h"
shared_ptr<VulkanRenderPipeline> VulkanContext::makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer, PipelineConfig config,
	vector<ColorBlendConfig> colorBlendConfigs
)
{
	return make_shared<VulkanRenderPipeline>(this, shader, renderer, config, colorBlendConfigs);
}

VulkanComputePipelineRef VulkanContext::makeComputePipeline(VulkanShaderRef shader)
{
	return make_shared<VulkanComputePipeline>(this, shader);
}

VulkanComputePipelineRef VulkanContext::makeComputePipeline(const char * computePath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts)
{
	return makeComputePipeline(
		makeComputeShader(computePath, setLayouts)
	);
}


#include "VulkanSwapchain.h"
VulkanSwapchainRef VulkanContext::makeSwapchain(vk::SurfaceKHR surface)
{
	return make_shared<VulkanSwapchain>(this, surface);
}

#include "VulkanShader.h"
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

#include "VulkanBuffer.h"

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

VulkanBufferRef VulkanContext::makeFastBuffer(vk::BufferUsageFlags usage, uint64_t size, void * data)
{

	if (data != nullptr) {
		auto buffer = makeBuffer(usage | vk::BufferUsageFlagBits::eTransferDst, size, VulkanBuffer::CPU_NEVER);

		auto staging = makeBuffer(vk::BufferUsageFlagBits::eTransferSrc, size, VulkanBuffer::CPU_ALOT, data);
		auto task = makeTask(0);

		auto BufferCopy = vk::BufferCopy(0, 0, size);

		task->record([&](vk::CommandBuffer *cmd) {
			cmd->copyBuffer(staging->getBuffer(), buffer->getBuffer(), { BufferCopy });
		});

		task->execute(true);
		return buffer;
	}
	else {
		return makeBuffer(usage, size, VulkanBuffer::CPU_NEVER);
	}

}

VulkanBufferRef VulkanContext::makeLocalStorageBuffer(uint64_t size)
{
	return makeBuffer(VulkanBuffer::STORAGE_BUFFER, size, VulkanBuffer::CPU_NEVER);
}


#include "VulkanSet.h"
VulkanSetRef VulkanContext::makeSet(VulkanSetLayoutRef layout)
{
	return make_shared<VulkanSet>(this, layout);
}

VulkanSetRef VulkanContext::makeSet(vk::ArrayProxy<const VulkanUniformLayoutBinding> bindings)
{
	return make_shared<VulkanSet>(this, VulkanContext::makeSetLayout(bindings));

}

#include "VulkanTask.h"
VulkanTaskRef VulkanContext::makeTask(uint32_t poolIndex, bool autoReset)
{
	if (_pools.count(poolIndex) == 0) {
		_pools[poolIndex] = _device.createCommandPool(
			vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(), _familyIndex)
		);
	}
	return make_shared<VulkanTask>(this, _pools[poolIndex], autoReset);
}

#include "VulkanTaskGroup.h"
VulkanTaskGroupRef VulkanContext::makeTaskGroup(uint32_t numTasks, uint32_t poolIndex)
{
	if (_pools.count(poolIndex) == 0) {
		_pools[poolIndex] = _device.createCommandPool(
			vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(), _familyIndex)
		);
	}
	return make_shared<VulkanTaskGroup>(this, numTasks, _pools[poolIndex]);
}

#include "VulkanImage.h"

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

VulkanImage2DRef VulkanContext::makeImage2D(vk::ImageUsageFlags usage, vk::Format format, glm::ivec2 size)
{
	auto r = make_shared<VulkanImage2D>(this, usage, format, size);
	r->setSampler(getNearestSampler());
	return r;
}

VulkanImage2DRef VulkanContext::makeImage2D(vk::Image image, vk::Format format, glm::ivec2 size)
{
	return make_shared<VulkanImage2D>(this, image, format, size);
}

VulkanImage3DRef VulkanContext::makeImage3D(vk::ImageUsageFlags usage, vk::Format format, glm::ivec3 size)
{
	auto r = make_shared<VulkanImage3D>(this, usage, format, size);
	r->setSampler(getNearestSampler());
	return r;
}

VulkanImage3DRef VulkanContext::makeImage3D(vk::Image image, vk::Format format, glm::ivec3 size)
{
	return make_shared<VulkanImage3D>(this, image, format, size);
}

VulkanImageCubeRef VulkanContext::makeImageCube(vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format)
{
	return make_shared<VulkanImageCube>(this, usage, size, format);
}

#include "rtx/RTPipeline.h"
#include "rtx/RTAccelerationStructure.h"

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