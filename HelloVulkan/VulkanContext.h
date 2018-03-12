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

	~VulkanContext();

private:

	//vk::SwapchainKHR _swapchain;

	vk::Instance _instance;
	vector<vk::PhysicalDevice> _physicalDevices;
	vk::Device _device;

};

typedef shared_ptr<VulkanContext> VulkanContextRef;