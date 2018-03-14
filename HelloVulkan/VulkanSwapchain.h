#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"

class VulkanSwapchain
{
public:

	VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface);
	~VulkanSwapchain();

	void init(vk::SurfaceKHR surface);

	void createSemaphore();

	uint32 getNextIndex();

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

	vk::Semaphore &getSempahore() {
		return _semaphore;
	}

	vk::Rect2D getRect();

	vk::Fence &getFence() {
		return _fence;
	}

private:

	VulkanContextRef _ctx;
	vk::Fence _fence;
	vk::Semaphore _semaphore;
	vk::SurfaceKHR _surface;
	vk::SwapchainKHR _swapchain;
	vk::Format _format;

	vector<VulkanImageRef> _images;

	bool swapchainInited = false;

	vk::Extent2D _extent;
};

typedef shared_ptr<VulkanSwapchain> VulkanSwapchainRef;