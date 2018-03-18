#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanSwapchain.h"

#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanTask.h"
#include "VulkanUniformSet.h"

class VulkanRenderer
{
public:
	VulkanRenderer(VulkanContextRef context);

	void createDepthBuffer();
	void targetSwapcahin(VulkanSwapchainRef swapchain);

	void begin(VulkanTaskRef task);


	void end(VulkanTaskRef task);

	vk::RenderPass getRenderPass() {
		return _renderPass;
	}
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

	bool _renderPassCreated = false;

	//Triangle
	VulkanBufferRef _vbuffer, _ibuffer, _ubuffer;
	VulkanUniformLayoutRef _uniformLayout;
	VulkanUniformSetRef _uniformSet;
};

typedef shared_ptr<VulkanRenderer>  VulkanRendererRef;