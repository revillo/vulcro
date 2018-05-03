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

	void setClearColors(vector<std::array<float, 4>> colors) {
		_clearColors = colors;
	};

	void record(vk::CommandBuffer * cmd, function<void()> commands, int32 whichFramebuffer = -1);

	void begin(vk::CommandBuffer * cmd, int32 whichFramebuffer = -1);

    bool hasDepth() {
        return _useDepth;
    }

	vk::Rect2D getFullRect() {
		return _fullRect;
	}

	vk::Viewport getFullViewport() {
		
		
		return vk::Viewport(
			0,
			0,
			(float)_fullRect.extent.width,
			(float)_fullRect.extent.height,
			0.0,
			1.0
		);
	}

    uint32 getNumTargets() {
        if (_swapchain != nullptr) {
            return 1;
        }
        else {
            return static_cast<uint32>(_images.size());
        }
    }

	void resize();

	void end(vk::CommandBuffer * cmd);

	vk::RenderPass getRenderPass() {
		return _renderPass;
	}

	VulkanImageRef getDepthBuffer() {
		return _depthImage;
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
	vector<std::array<float, 4>> _clearColors;
	vk::Rect2D _fullRect;

	bool _renderPassCreated = false;
};