#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"

class VulkanSwapchain
{
public:

	VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface);
	~VulkanSwapchain();

	void init(vk::SurfaceKHR surface);

	vk::Format getFormat() {
		return _format;
	}

	uint32 numImages() {
		return _images.size();
	}


	vector<VulkanImageRef> getImages() {
		return _images;
	}

private:

	VulkanContextRef _ctx;
	vk::SurfaceKHR _surface;
	vk::SwapchainKHR _swapchain;
	vector<VulkanImageRef> _images;
	vk::Format _format;
	bool swapchainInited = false;

};

typedef shared_ptr<VulkanSwapchain> VulkanSwapchainRef;