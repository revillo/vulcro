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


	std::vector<const char*> extensions;

	extensions.push_back("VK_KHR_swapchain");
	//extensions.push_back("VK_KHR_win32_surface");

	float qpriors[1] = { 0.0f };

	auto devQ = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), familyIndex, 1, qpriors);

	_device = _physicalDevices[0].createDevice(
		vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1,
			&devQ, 0, nullptr, extensions.size(), &extensions[0], nullptr)
	);


	_queue = _device.getQueue(familyIndex, 0);

	_commandPool = _device.createCommandPool(
		vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(), familyIndex)
	);
}



shared_ptr<ibo> VulkanContext::makeIBO(vector<uint16_t> indices)
{
	
	return make_shared<ibo>(this, indices);
	
}

VulkanContext::~VulkanContext()
{
	
	_device.destroyCommandPool(_commandPool);

	_device.destroy();

}


/// Helpers


#include "VulkanUniformSetLayout.h"
shared_ptr<VulkanUniformSetLayout> VulkanContext::makeUniformSetLayout(vector<VulkanUniformLayoutBinding> bindings)
{
	return make_shared<VulkanUniformSetLayout>(this, bindings);
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
shared_ptr<VulkanPipeline> VulkanContext::makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer)
{
	return make_shared<VulkanPipeline>(this, shader, renderer);
}

#include "VulkanSwapchain.h"
VulkanSwapchainRef VulkanContext::makeSwapchain(vk::SurfaceKHR surface)
{
	return make_shared<VulkanSwapchain>(this, surface);
}

#include "VulkanShader.h"
VulkanShaderRef VulkanContext::makeShader(const char * vertPath, const char * fragPath, vector<VulkanVertexLayoutRef> vertexLayouts, vector<VulkanUniformSetLayoutRef> uniformLayouts)
{
	return make_shared<VulkanShader>(this, vertPath, fragPath, vertexLayouts, uniformLayouts);
}
#include "VulkanBuffer.h"
VulkanBufferRef VulkanContext::makeBuffer(vk::BufferUsageFlags usage, uint64 size, void * data)
{
	return make_shared<VulkanBuffer>(this, usage, size, data);
}


#include "VulkanUniformSet.h"
VulkanUniformSetRef VulkanContext::makeUniformSet(VulkanUniformSetLayoutRef layout)
{
	return make_shared<VulkanUniformSet>(this, layout);
}

#include "VulkanTask.h"
VulkanTaskRef VulkanContext::makeTask()
{
	return make_shared<VulkanTask>(this);
}

#include "VulkanImage.h"
VulkanImageRef VulkanContext::makeImage(vk::ImageUsageFlagBits usage, glm::ivec2 size, vk::Format format)
{
	return make_shared<VulkanImage>(this, usage, size, format);
}

VulkanImageRef VulkanContext::makeImage(vk::Image image, glm::ivec2 size, vk::Format format)
{
	return make_shared<VulkanImage>(this, image, size, format);
}
