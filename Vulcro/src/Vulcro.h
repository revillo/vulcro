#pragma once

#include "vulkan-core/VulkanRenderer.h"
#include "vulkan-core/VulkanPipeline.h"
#include "vulkan-core/VulkanTaskGroup.h"
#include "vulkan-core/VulkanTaskPool.h"
#include "vulkan-core/VulkanSwapchain.h"
#include "vulkan-rtx/RTAccelerationStructure.h"
#include "vulkan-rtx/RTPipeline.h"
#include "vulkan-core/VulkanInstance.h"
#include "vulkan-window/VulkanWindowSDL.h"

namespace vulcro
{
    const glm::mat4 glProjFixYZ = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.5f, 1.0f
    );

    const glm::mat4 glProjFixY = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}