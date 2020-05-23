#pragma once 
#include "vulkan/vulkan.hpp"

#include "VulkanDeviceManager.h"

class VulkanContext;

class VulkanInstance
{
public:

	VulkanInstance(std::vector<const char *> instanceExtensions);

	~VulkanInstance();

    vk::Instance getInstance();

	std::shared_ptr<VulkanContext> createContext(vk::PhysicalDevice& pDevice, const std::vector<const char *> & deviceExtensions = { "VK_KHR_get_memory_requirements2", "VK_NV_ray_tracing" });
	
private:

	vk::Instance mVkInstance;
};