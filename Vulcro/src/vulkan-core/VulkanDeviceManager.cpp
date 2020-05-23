#include "VulkanDeviceManager.h"

#include "VulkanContext.h"

namespace vke
{
    VulkanDeviceManager::VulkanDeviceManager(vk::Instance instance) : mInstance(instance)
    {
        // List all physical devices with vulkan capabilities
        mPhysicalDevices = instance.enumeratePhysicalDevices();

        // Prepare metadata about each device
        for (auto deviceIndex = 0; deviceIndex < mPhysicalDevices.size(); ++deviceIndex)
        {
            auto& physicalDevice = mPhysicalDevices[deviceIndex];
            auto& physicalDeviceProperties = mPhysicalDeviceProperties[deviceIndex];

            // Prepare a lookup of the device's supported extensions
            auto supportedExtensions = physicalDevice.enumerateDeviceExtensionProperties();
            for (auto& extension : supportedExtensions)
            {
                physicalDeviceProperties.extensionsLookup[extension.extensionName] = true;
            }

            // Request information about this physical device's queueFamily
            auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

            // Loop over all of the queues and keep track of the supported submission types
            vk::QueueFlags supportedQueueTypes;
            for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
            {
                supportedQueueTypes = supportedQueueTypes | queueFamilyProperties[i].queueFlags;
            }

            // Set the lookup table with flags detailing its supported queues
            physicalDeviceProperties.queueFlags = supportedQueueTypes;
        }
    }

    std::vector<DeviceIndex> VulkanDeviceManager::findPhysicalDevicesWithCapabilities(const std::vector<const char*> & requiredDeviceExtensions, const vk::QueueFlags requiredQueues)
    {
        std::vector<DeviceIndex> useableDevices;

        // Loop over each device and check if it has hte required apabilities
        for (auto deviceIndex = 0; deviceIndex < mPhysicalDevices.size(); ++deviceIndex)
        {
            // Pass over this device if not all extensions are supported
            if (!checkPhysicalDeviceExtensionSupport(deviceIndex, requiredDeviceExtensions))
            {
                continue;
            }

            // Pass over this device if not all queue capabilities are supported
            if (!checkPhysicalDeviceQueueSupport(deviceIndex, requiredQueues))
            {
                continue;
            }

            // Since the device has passed all of our tests, add it o the our lsit of useable devices
            useableDevices.push_back(deviceIndex);
        }

        return useableDevices;
    }

    vk::PhysicalDevice& VulkanDeviceManager::getPhysicalDevice(DeviceIndex deviceIndex)
    {
        return mPhysicalDevices.at(deviceIndex);
    }

    //const VulkanContext& VulkanDeviceManager::requestWorkingDevice(DeviceIndex deviceIndex)
    //{

    //}


    bool VulkanDeviceManager::checkPhysicalDeviceExtensionSupport(const DeviceIndex deviceIndex, const std::vector<const char*> requiredDeviceExtensions)
    {
        auto& physicalDeviceProperties = mPhysicalDeviceProperties[deviceIndex];

        // Loop over each extension and check if it's supported
        for (auto* extension : requiredDeviceExtensions)
        {
            // Return early with a failure if we find an undupported extension
            if (!physicalDeviceProperties.extensionsLookup[extension])
            {
                return false;
            }
        }

        // All required extensions are supported
        return true;
    }

    bool VulkanDeviceManager::checkPhysicalDeviceQueueSupport(const DeviceIndex deviceIndex, const vk::QueueFlags requiredQueues)
    {
        auto& physicalDeviceProperties = mPhysicalDeviceProperties[deviceIndex];

        // Return 'true' if the physical device supports all the required queue types
        return (physicalDeviceProperties.queueFlags & requiredQueues) == requiredQueues;
    }
}

