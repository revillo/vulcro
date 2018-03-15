#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanSwapchain.h"

#include "VulkanShader.h"
#include "VulkanBuffer.h"

class VulkanRenderer
{
public:
	VulkanRenderer(VulkanContextRef context, glm::ivec2 size);

	void createDepthBuffer();
	void createSurfaceRenderPass(VulkanSwapchainRef swapchain);
	void createGraphicsPipeline();
	void renderTriangle();

	~VulkanRenderer();

private:
	
	void createSurfaceFramebuffer(VulkanSwapchainRef swapchain);

	vk::Fence _drawFence;

	ivec2 _size;
	VulkanContextRef _ctx;
	VulkanImageRef _depthImage;
	VulkanSwapchainRef _swapchain;
	
	vector<vk::Framebuffer> _framebuffers;
	vk::RenderPass _renderPass;
	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
};

typedef shared_ptr<VulkanRenderer>  VulkanRendererRef;