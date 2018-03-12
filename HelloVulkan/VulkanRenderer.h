#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanSwapchain.h"

class VulkanRenderer
{
public:
	VulkanRenderer(VulkanContextRef context, glm::ivec2 size);

	void createDepthBuffer();

	~VulkanRenderer();

private:
	
	ivec2 _size;
	VulkanContextRef _ctx;

};

typedef shared_ptr<VulkanRenderer>  VulkanRendererRef;