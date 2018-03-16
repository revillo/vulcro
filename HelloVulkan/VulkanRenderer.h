#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanSwapchain.h"

#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanTask.h"

class VulkanRenderer
{
public:
	VulkanRenderer(VulkanContextRef context);

	void createDepthBuffer();
	void targetSwapcahin(VulkanSwapchainRef swapchain);
	void createGraphicsPipeline();

	void createTriangle();
	void renderTriangle(VulkanTaskRef task);

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


	bool _renderPassCreated = false;
	bool _pipelineCreated = false;
	bool _layoutCreated = false;

	//Triangle
	VulkanBufferRef _vbuffer, _ibuffer, _ubuffer;
	VulkanUniformLayoutRef _uniformLayout;
};

typedef shared_ptr<VulkanRenderer>  VulkanRendererRef;