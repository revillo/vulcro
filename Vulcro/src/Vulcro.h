#pragma once

#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"
#include "VulkanTaskGroup.h"
#include "VulkanSwapchain.h"

//TODO Move outside
#include "glm\gtc\matrix_transform.inl"
#include "glm\gtc\matrix_inverse.hpp"


const glm::mat4 VULCRO_glProjFixYZ = mat4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.0f, 0.5f, 1.0f
);

const glm::mat4 VULCRO_glProjFixY = mat4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);
