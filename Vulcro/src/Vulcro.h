#pragma once

#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"

namespace vulcro {
	const glm::mat4 glProjFix = mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.5f,
		0.0f, 0.0f, 0.0f, 0.0f
	);
}