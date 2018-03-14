#pragma once

#include <vulkan/vulkan.hpp>
#include "General.h"

class VulkanContext
{
public:
	VulkanContext(vk::Instance instance);
	vk::Device &getDevice() { return _device; }
	vk::PhysicalDevice &getPhysicalDevice() { return _physicalDevices[0]; }
	//vector<vk::PhysicalDevice> getPhysicalDevices() { return _instance.enumeratePhysicalDevices(); }

	vk::CommandBuffer &cmd() {
		return _cmd;
	}

	vk::Queue &getQueue() {
		return _queue;
	}



	~VulkanContext();

private:

	//vk::SwapchainKHR _swapchain;

	vk::Instance _instance;
	vector<vk::PhysicalDevice> _physicalDevices;
	vk::Device _device;
	vk::CommandBuffer _cmd;
	vk::Queue _queue;
};

typedef shared_ptr<VulkanContext> VulkanContextRef;