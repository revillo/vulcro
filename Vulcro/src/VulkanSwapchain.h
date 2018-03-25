#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"

class VulkanSwapchain
{
public:

	VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface);
	~VulkanSwapchain();

	bool init(vk::SurfaceKHR surface);


	bool nextFrame();

	bool present(vector<vk::Semaphore> inSems = {});

	vk::Format getFormat() {
		return _format;
	}

	uint64 numImages() {
		return _images.size();
	}

	vk::SwapchainKHR &getSwapchain() {
		return _swapchain;
	}

	vector<VulkanImageRef> getImages() {
		return _images;
	}

	bool resize();

	vk::Semaphore &getSemaphore() {
		return _semaphore;
	}

	const uint32 getRenderingIndex() {
		return _renderingIndex;
	}

	vk::Rect2D getRect();

private:

	void createSemaphore();

	uint32 _renderingIndex;

	VulkanContextRef _ctx;
	vk::Semaphore _semaphore;
	vk::SurfaceKHR _surface;
	vk::SwapchainKHR _swapchain;
	vk::Format _format;

	vector<VulkanImageRef> _images;

	bool swapchainInited = false;
	bool _frameFailed = false;

	vk::Extent2D _extent;
};
