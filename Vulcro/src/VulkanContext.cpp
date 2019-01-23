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
shared_ptr<VulkanSetLayout> VulkanContext::makeSetLayout(temps<VulkanUniformLayoutBinding> bindings)
{
	return make_shared<VulkanSetLayout>(this, std::move(bindings));
}

VulkanSetLayoutRef VulkanContext::makeSetLayout(vector<VulkanUniformLayoutBinding>& bindings)
{
	return make_shared<VulkanSetLayout>(this, std::move(bindings));
}

#include "VulkanVertexLayout.h"
shared_ptr<VulkanVertexLayout> VulkanContext::makeVertexLayout(temps<vk::Format> fields)
{
	return make_shared<VulkanVertexLayout>(move(fields));
}

#include "VulkanRenderer.h"
shared_ptr<VulkanRenderer> VulkanContext::makeRenderer()
{
	return make_shared<VulkanRenderer>(this);
}

#include "VulkanPipeline.h"
shared_ptr<VulkanPipeline> VulkanContext::makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer, PipelineConfig config,
	vector<ColorBlendConfig> colorBlendConfigs
)
{
	return make_shared<VulkanPipeline>(this, shader, renderer, config, colorBlendConfigs);
}

VulkanComputePipelineRef VulkanContext::makeComputePipeline(VulkanShaderRef shader)
{
	return make_shared<VulkanComputePipeline>(this, shader);
}

VulkanComputePipelineRef VulkanContext::makeComputePipeline(const char * computePath, vector<VulkanSetLayoutRef>&& setLayouts)
{
	return makeComputePipeline(
		makeComputeShader(computePath, std::move(setLayouts))
	);
}


#include "VulkanSwapchain.h"
VulkanSwapchainRef VulkanContext::makeSwapchain(vk::SurfaceKHR surface)
{
	return make_shared<VulkanSwapchain>(this, surface);
}

#include "VulkanShader.h"
VulkanShaderRef VulkanContext::makeShader(const char * vertPath, const char * fragPath, vector<VulkanVertexLayoutRef>&& vertexLayouts, vector<VulkanSetLayoutRef>&& uniformLayouts)
{
	return make_shared<VulkanShader>(this, vertPath, fragPath, std::move(vertexLayouts), std::move(uniformLayouts));
}

VulkanShaderRef VulkanContext::makeTessShader(const char * vertPath, const char * tessControlPath, const char * tessEvalPath, const char * tessGeomPath, const char * fragPath, vector<VulkanVertexLayoutRef>&& vertexLayouts, vector<VulkanSetLayoutRef>&& uniformLayouts)
{
	return make_shared<VulkanShader>(this, vertPath, tessControlPath, tessEvalPath, tessGeomPath, fragPath, std::move(vertexLayouts), std::move(uniformLayouts));
}

VulkanShaderRef VulkanContext::makeComputeShader(const char * computePath, vector<VulkanSetLayoutRef>&& uniformLayouts)
{
	return make_shared<VulkanShader>(this, computePath, std::move(uniformLayouts));
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

		auto staging = makeBuffer(usage | vk::BufferUsageFlagBits::eTransferSrc, size, VulkanBuffer::CPU_ALOT, data);
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

VulkanSetRef VulkanContext::makeSet(temps<VulkanUniformLayoutBinding>&& bindings)
{
	return make_shared<VulkanSet>(this, VulkanContext::makeSetLayout(std::move(bindings)));

}

VulkanSetRef VulkanContext::makeSet(vector<VulkanUniformLayoutBinding>& bindings)
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
VulkanImageRef VulkanContext::makeImage(vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format)
{
	auto r = make_shared<VulkanImage>(this, usage, size, format);
	r->createImage();
	r->setSampler(getNearestSampler());
	return r;
}

VulkanImageRef VulkanContext::makeImage(vk::Image image, glm::ivec2 size, vk::Format format)
{
	return make_shared<VulkanImage>(this, image, size, format);
}

#include "rtx/RTPipeline.h"
#include "rtx/RTAccelerationStructure.h"

shared_ptr<RTShaderBuilder> VulkanContext::makeRayTracingShaderBuilder(const char * raygenPath, vector<VulkanSetLayoutRef>&& setLayouts)
{
	return make_shared<RTShaderBuilder>(this, raygenPath, move(setLayouts));
}

shared_ptr<RTPipeline> VulkanContext::makeRayTracingPipeline(RTShaderBuilderRef shader)
{
	return make_shared<RTPipeline>(this, shader);
}

shared_ptr<RTScene> VulkanContext::makeRayTracingScene()
{
	return make_shared<RTScene>(this);
}