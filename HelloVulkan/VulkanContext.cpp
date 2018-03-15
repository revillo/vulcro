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



/*

VulkanTaskRef VulkanContext::getTask(std::string taskName)
{
	if (_taskMap.count(taskName) == 0) {
		auto cmdb = _device.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(_commandPool, vk::CommandBufferLevel::ePrimary, 1)
		)[0];

		_taskMap[taskName] = make_shared<VulkanTask>(cmdb);
	}

	return _taskMap[taskName];
}
*/

VulkanContext::~VulkanContext()
{
}
