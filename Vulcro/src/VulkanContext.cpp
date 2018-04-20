#include "VulkanContext.h"

using namespace vk;

VulkanContext::VulkanContext(vk::Instance instance)
	:_instance(instance)
{
	_physicalDevices = _instance.enumeratePhysicalDevices();

	auto qfps = _physicalDevices[0].getQueueFamilyProperties();

	int familyIndex = -1;

	for (uint32 i = 0; i < qfps.size(); i++) {
		if (qfps[i].queueFlags & vk::QueueFlagBits::eGraphics) {

			familyIndex = i;
			break;
		}
	}

	_familyIndex = familyIndex;

	std::vector<const char*> extensions;

	extensions.push_back("VK_KHR_swapchain");
	//extensions.push_back("VK_KHR_win32_surface");


	auto features = vk::PhysicalDeviceFeatures();

	features.setTessellationShader(true);
	features.setGeometryShader(true);


	float qpriors[1] = { 0.0f };

	auto devQ = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), familyIndex, 1, qpriors);

	_device = _physicalDevices[0].createDevice(
		vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1,
			&devQ,
			0, //Enabled Layers 
			nullptr, 
			static_cast<uint32>(extensions.size()), &extensions[0],
			&features //enabled features
		)
	);


	_queue = _device.getQueue(familyIndex, 0);



}



shared_ptr<ibo> VulkanContext::makeIBO(vector<uint16_t> && indices)
{
	return make_shared<ibo>(this, move(indices));
	
}

VulkanContext::~VulkanContext()
{
	for (auto keyval : _pools)
		_device.destroyCommandPool(keyval.second);

	_device.destroy();

}


/// Helpers


#include "VulkanUniformSetLayout.h"
shared_ptr<VulkanUniformSetLayout> VulkanContext::makeUniformSetLayout(vector<VulkanUniformLayoutBinding> && bindings)
{
	return make_shared<VulkanUniformSetLayout>(this, move(bindings));
}

#include "VulkanVertexLayout.h"
shared_ptr<VulkanVertexLayout> VulkanContext::makeVertexLayout(vector<vk::Format> fields)
{
	return make_shared<VulkanVertexLayout>(fields);
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

VulkanComputePipelineRef VulkanContext::makeComputePipeline(const char * computePath, vector<VulkanUniformSetLayoutRef>&& setLayouts)
{
	return makeComputePipeline(
		makeComputeShader(computePath, move(setLayouts))
	);
}


#include "VulkanSwapchain.h"
VulkanSwapchainRef VulkanContext::makeSwapchain(vk::SurfaceKHR surface)
{
	return make_shared<VulkanSwapchain>(this, surface);
}

#include "VulkanShader.h"
VulkanShaderRef VulkanContext::makeShader(const char * vertPath, const char * fragPath, vector<VulkanVertexLayoutRef>&& vertexLayouts, vector<VulkanUniformSetLayoutRef>&& uniformLayouts)
{
	return make_shared<VulkanShader>(this, vertPath, fragPath, std::move(vertexLayouts), std::move(uniformLayouts));
}

VulkanShaderRef VulkanContext::makeTessShader(const char * vertPath, const char * tessControlPath, const char * tessEvalPath, const char * tessGeomPath, const char * fragPath, vector<VulkanVertexLayoutRef>&& vertexLayouts, vector<VulkanUniformSetLayoutRef>&& uniformLayouts)
{
	return make_shared<VulkanShader>(this, vertPath, tessControlPath, tessEvalPath, tessGeomPath, fragPath, std::move(vertexLayouts), std::move(uniformLayouts));
}

VulkanShaderRef VulkanContext::makeComputeShader(const char * computePath, vector<VulkanUniformSetLayoutRef>&& uniformLayouts)
{
	return make_shared<VulkanShader>(this, computePath, std::move(uniformLayouts));
}

#include "VulkanBuffer.h"
VulkanBufferRef VulkanContext::makeBuffer(vk::BufferUsageFlags usage, uint64 size, vk::MemoryPropertyFlags flags, void * data)
{
	return make_shared<VulkanBuffer>(this, usage, size, flags, data);
}

VulkanBufferRef VulkanContext::makeLocalStorageBuffer(uint64 size)
{
	return makeBuffer(VulkanBuffer::STORAGE_BUFFER, size, VulkanBuffer::CPU_NEVER);
}


#include "VulkanUniformSet.h"
VulkanUniformSetRef VulkanContext::makeUniformSet(VulkanUniformSetLayoutRef layout)
{
	return make_shared<VulkanUniformSet>(this, layout);
}

VulkanUniformSetRef VulkanContext::makeUniformSet(vector<VulkanUniformLayoutBinding>&& bindings)
{
	return make_shared<VulkanUniformSet>(this, VulkanContext::makeUniformSetLayout(move(bindings)));

}

#include "VulkanTask.h"
VulkanTaskRef VulkanContext::makeTask(uint32 poolIndex, bool autoReset)
{
	if (_pools.count(poolIndex) == 0) {
		_pools[poolIndex] = _device.createCommandPool(
			vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(), _familyIndex)
		);
	}
	return make_shared<VulkanTask>(this, _pools[poolIndex], autoReset);
}

#include "VulkanTaskGroup.h"
VulkanTaskGroupRef VulkanContext::makeTaskGroup(uint32 numTasks, uint32 poolIndex)
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
	return make_shared<VulkanImage>(this, usage, size, format);
}

VulkanImageRef VulkanContext::makeImage(vk::Image image, glm::ivec2 size, vk::Format format)
{
	return make_shared<VulkanImage>(this, image, size, format);
}
