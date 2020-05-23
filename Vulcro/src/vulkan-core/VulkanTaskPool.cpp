#include "VulkanTaskPool.h"
#include "VulkanTask.h"
#include "VulkanTaskGroup.h"

VulkanTaskPool::VulkanTaskPool(VulkanContextPtr ctx, vk::CommandPoolCreateFlags createFlags)
    :mCtx(ctx)
{
    auto device = ctx->getDevice();

    mCommandPool = device.createCommandPool(
        vk::CommandPoolCreateInfo(createFlags, ctx->getFamilyIndex())
    );
}

void VulkanTaskPool::reset()
{
    auto device = mCtx->getDevice();
    device.resetCommandPool(mCommandPool, vk::CommandPoolResetFlags());
}


VulkanTaskPool::~VulkanTaskPool()
{
    auto device = mCtx->getDevice();

    device.destroyCommandPool(mCommandPool);
}