#pragma once

#include <vulkan/vulkan.hpp>
#include "General.h"

class VulkanContext
{
public:
	VulkanContext(vk::Instance instance);
	vk::Device &getDevice() { return _device; }
	vk::PhysicalDevice &getPhysicalDevice() { return _physicalDevices[0]; }

	vk::Queue &getQueue() {
		return _queue;
	}

	vk::CommandPool &getCommandPool() {
		return _commandPool;
	}

	void resetTasks() {

		_device.resetCommandPool(
			_commandPool,
			vk::CommandPoolResetFlags()
		);

	}
	~VulkanContext();

private:

	vk::Instance _instance;
	vector<vk::PhysicalDevice> _physicalDevices;
	vk::Device _device;
	vk::CommandPool _commandPool;
	vk::CommandBuffer _cmd;
	vk::Queue _queue;
};

typedef shared_ptr<VulkanContext> VulkanContextRef;