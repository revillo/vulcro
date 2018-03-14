#include "VulkanContext.h"

using namespace vk;

VulkanContext::VulkanContext(vk::Instance instance)
	:_instance(instance)
{
	_physicalDevices = _instance.enumeratePhysicalDevices();

	auto qfps = _physicalDevices[0].getQueueFamilyProperties();
	
	int familyIndex = -1;

	for (int i = 0; i < qfps.size(); i++) {
		if (qfps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			
			familyIndex = i;
			break;
		}
	}


	std::vector<const char*> extensions;

	extensions.push_back("VK_KHR_swapchain");
	
	float qpriors[1] = { 0.0f };

	auto devQ = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), familyIndex, 1, qpriors);

	_device = _physicalDevices[0].createDevice(
		vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1,
			&devQ, 0, nullptr, 1, &extensions[0])
	);


	_queue = _device.getQueue(familyIndex, 0);

	auto commandPool = _device.createCommandPool(
		vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(), familyIndex)
	);
	
	_cmd = _device.allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1)
	)[0];
}





VulkanContext::~VulkanContext()
{
}
