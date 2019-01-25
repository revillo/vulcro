#pragma once

#include "VulkanContext.h"
#include "VulkanImage2D.h"

class VulkanSwapchain
{
public:


	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface);
	~VulkanSwapchain();

	//////////////////////////
	//// Functions
	/////////////////////////

	bool init(vk::SurfaceKHR surface);

	bool nextFrame();

	bool present(vector<vk::Semaphore> inSems = {});

	bool resize();

	//////////////////////////
	//// Getters / Setters
	/////////////////////////

	vk::Format getFormat() {
		return _format;
	}


	uint32_t numImages() {
		return static_cast<uint32_t>(_images.size());
	}

	vk::SwapchainKHR &getSwapchain() {
		return _swapchain;
	}

	vector<VulkanImage2DRef> getImages() {
		return _images;
	}

	
	vk::Semaphore &getSemaphore() {
		return _semaphore;
	}

	const uint32_t getRenderingIndex() {
		return _renderingIndex;
	}

	vk::Rect2D getRect();

private:

	void createSemaphore();

	uint32_t _renderingIndex;

	VulkanContextRef _ctx;
	vk::Semaphore _semaphore;
	vk::SurfaceKHR _surface;
	vk::SwapchainKHR _swapchain;
	vk::Format _format;

	vector<VulkanImage2DRef> _images;

	bool swapchainInited = false;
	bool _frameFailed = false;

	vk::Extent2D _extent;
};
