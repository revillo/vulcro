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
	void targetSwapcahin(VulkanSwapchainRef swapchain, bool useDepth = true);
	void targetImages(vector<VulkanImageRef> images, bool useDepth = true);

	void record(vk::CommandBuffer * cmd, function<void()> commands);

	void begin(vk::CommandBuffer * cmd);


	void end(vk::CommandBuffer * cmd);

	vk::RenderPass getRenderPass() {
		return _renderPass;
	}
	~VulkanRenderer();

private:
	
	void createSwapchainFramebuffers(VulkanSwapchainRef swapchain);
	void createImagesFramebuffer();

	VulkanContextRef _ctx;
	VulkanImageRef _depthImage;

	VulkanSwapchainRef _swapchain = nullptr;
	vector<VulkanImageRef> _images;

	vector<vk::Framebuffer> _framebuffers;

	vk::RenderPass _renderPass;

	bool _useDepth = false;

	vk::Rect2D _fullRect;

	bool _renderPassCreated = false;
};