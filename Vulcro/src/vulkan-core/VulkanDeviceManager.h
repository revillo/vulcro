#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <stdint.h>
#include <unordered_map>

namespace vke
{
    class WorkingDevice;

    using DeviceIndex = uint32_t;

    class VulkanDeviceManager
    {
        struct PhysicalDeviceProperties
        {
            vk::QueueFlags queueFlags;
            std::unordered_map<std::string, bool> extensionsLookup;
        };

    public:

        /**
            VulkanDeviceManager is used to manage Physical devices and their context
            @param instance - The vulkan instance
        */
        VulkanDeviceManager(vk::Instance instance);

        /**
            Find a list of all devices that would fulfill the passed requirements

            @param requiredDeviceExtensions - A vector listing the required device extensions the physical device needs to support
            @param requiredQueues - Bitflags detailing which submission types at least one of the device queues needs to support
            @return A vector containing the device-index of each capable device
        */
        std::vector<DeviceIndex> findPhysicalDevicesWithCapabilities(const std::vector<const char*> & requiredDeviceExtensions, const vk::QueueFlags requiredQueues);

        /**
            Even though all the devices found using 'findPhysicalDevicesWithCapabilities' should have all the required capabilites,
            it may be fruitful to take a closer look at each physical device and compare specifications such as memory.

            @param deviceIndex - The index of the device to retrieve
            @return A constant reference to the physical device at index
        */
        vk::PhysicalDevice& getPhysicalDevice(DeviceIndex deviceIndex);

        ///**
        //    Once a physical device has been settled on, a working device can be requested for it.
        //    If a working device has already been created for the physical device the existing one will be returned.

        //    @param deviceIndex - The index of the device to request a working device of
        //*/
        //const WorkingDevice& requestWorkingDevice(DeviceIndex deviceIndex);

    private:

        bool checkPhysicalDeviceExtensionSupport(const DeviceIndex deviceIndex, const std::vector<const char*> requiredDeviceExtensions);

        bool checkPhysicalDeviceQueueSupport(const DeviceIndex deviceIndex, const vk::QueueFlags requiredQueues);

        vk::Instance mInstance;
        std::vector<vk::PhysicalDevice> mPhysicalDevices;
        std::unordered_map<DeviceIndex, PhysicalDeviceProperties> mPhysicalDeviceProperties;
    };
}
