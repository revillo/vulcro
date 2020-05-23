#include "VulkanInstance.h"
#include <unordered_map>
#include <vector>
#include <iostream>


VulkanInstance::VulkanInstance(std::vector<const char*> instanceExtensions)
{
	// Use validation layers if this is a debug build
	std::vector<const char*> layers;

#if defined(_DEBUG)
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	// vk::ApplicationInfo allows the programmer to specifiy some basic information about the
	// program, which can be useful for layers and tools to provide more debug information.
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setPApplicationName("Vulkan C++ Windowed Program Template")
		.setApplicationVersion(1)
		.setPEngineName("LunarG SDK")
		.setEngineVersion(1)
		.setApiVersion(VK_API_VERSION_1_1);

    // Create a lookup table to help us check if the requested extensions are supported
    auto extensionProperties = vk::enumerateInstanceExtensionProperties();
	std::unordered_map<std::string, bool> extensionLookup;
	for (auto & e : extensionProperties)
    {
		extensionLookup[e.extensionName] = true;
	}

    // Collect Supported extensions and provide warnings for unsupported extension
	std::vector<const char*> extensions;
	for (auto* extensionName : instanceExtensions)
    {
        if (extensionLookup[extensionName])
        {
            extensions.push_back(extensionName);
        }
        else
        {
            abort();
        }
	}

    // vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
    // are needed.
	vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
		.setPpEnabledLayerNames(layers.data());

	// Attempt to initialize our vulkan instance
	try
    {
		mVkInstance = vk::createInstance(instanceCreateInfo);
	}
	catch (const std::exception& e) 
    {
        abort();
	}
}

VulkanInstance::~VulkanInstance()
{
	mVkInstance.destroy();
}

vk::Instance VulkanInstance::getInstance()
{
    return mVkInstance;
}


#include "VulkanContext.h"
std::shared_ptr<VulkanContext> VulkanInstance::createContext(vk::PhysicalDevice& pDevice, const std::vector<const char*> & deviceExtensions)
{
	return make_shared<VulkanContext>(mVkInstance, pDevice, deviceExtensions);
}


