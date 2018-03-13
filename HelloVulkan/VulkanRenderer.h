#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanSwapchain.h"

class VulkanRenderer
{
public:
	VulkanRenderer(VulkanContextRef context, glm::ivec2 size);

	void createDepthBuffer();
	void createSurfaceRenderPass(VulkanSwapchainRef swapchain);


	~VulkanRenderer();

private:
	
	void createSurfaceFramebuffer(VulkanSwapchainRef swapchain);

	ivec2 _size;
	VulkanContextRef _ctx;
	VulkanImageRef _depthImage;
	vk::RenderPass _renderPass;
};

typedef shared_ptr<VulkanRenderer>  VulkanRendererRef;