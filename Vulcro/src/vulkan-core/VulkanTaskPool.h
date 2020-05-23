#pragma once

#include "General.h"
#include "vulkan/vulkan.hpp"
#include "VulkanContext.h"

class VulkanTaskPool
{
public:
    VulkanTaskPool(VulkanContextPtr ctx, vk::CommandPoolCreateFlags poolFlags = vk::CommandPoolCreateFlags());
    ~VulkanTaskPool();

    vk::CommandPool getPool()
    {
        return mCommandPool;
    }

    void reset();
private:

    VulkanContextPtr mCtx;
    vk::CommandPool mCommandPool;
};