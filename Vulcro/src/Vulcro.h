#pragma once

#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"
#include "VulkanTaskGroup.h"
#include "VulkanSwapchain.h"
#include "rtx/RTAccelerationStructure.h"
#include "rtx/RTPipeline.h"

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

using VulcroResult = uint32_t;

class Vulcro
{
public:
	static const VulcroResult RESULT_ERROR = 0x1;
	static const VulcroResult MEMORY_ALLOCATED = 0x2;

	inline static bool check(VulcroResult result)
	{
		return result & Vulcro::RESULT_ERROR == 0;
	}
};
