#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanSwapchain.h"

#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanTask.h"
#include "VulkanSet.h"

class VulkanRenderer
{
public:
	VulkanRenderer(VulkanContextPtr context);

	void createDepthBuffer();
	void targetSwapcahin(VulkanSwapchainRef swapchain, bool useDepth = true);
	void targetImages(vector<VulkanImage2DRef> images, bool useDepth = true);
	void targetDepth(glm::ivec2 size);

	void setClearColors(vector<std::array<float, 4>> colors) {
		_clearColors = colors;
	};

	void record(vk::CommandBuffer * cmd, function<void()> commands, int32_t whichFramebuffer = -1);

	void begin(vk::CommandBuffer * cmd, int32_t whichFramebuffer = -1);

    bool hasDepth() {
        return _useDepth;
    }

	vk::Rect2D &getFullRect() {
		return _fullRect;
	}

	vk::Viewport &getFullViewport() {
		
		_fullViewport = vk::Viewport(
			0,
			0,
			(float)_fullRect.extent.width,
			(float)_fullRect.extent.height,
			0.0,
			1.0
		);

		return _fullViewport;
	}

    uint32_t getNumTargets() {
        if (_swapchain != nullptr) {
            return 1;
        }
        else {
            return static_cast<uint32_t>(_images.size());
        }
    }

	void resize();

	void end(vk::CommandBuffer * cmd);

	vk::RenderPass getRenderPass() {
		return _renderPass;
	}

	VulkanImage2DRef getDepthBuffer() {
		return _depthImage;
	}
	
	~VulkanRenderer();

private:
	
	void createSwapchainFramebuffers(VulkanSwapchainRef swapchain);
	void createImagesFramebuffer();

	VulkanContextPtr _ctx;
	VulkanImage2DRef _depthImage;

	VulkanSwapchainRef _swapchain = nullptr;
	vector<VulkanImage2DRef> _images;

	vector<vk::Framebuffer> _framebuffers;

	vk::RenderPass _renderPass;

	bool _useDepth = false;
	vector<std::array<float, 4>> _clearColors;
	vk::Rect2D _fullRect;
	vk::Viewport _fullViewport;

	bool _renderPassCreated = false;
};