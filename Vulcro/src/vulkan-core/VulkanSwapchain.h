#pragma once

#include "VulkanContext.h"
#include "VulkanImage.h"

class VulkanSwapchain
{
public:


	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanSwapchain(VulkanContextPtr ctx, vk::SurfaceKHR surface);
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

	inline vk::Format getFormat() {
		return mFormat;
	}

	inline uint32_t numImages() {
		return static_cast<uint32_t>(mImages.size());
	}

	inline vk::SwapchainKHR &getSwapchain() {
		return mSwapchain;
	}

	inline vector<VulkanImage2DRef> getImages() {
		return mImages;
	}
	
	inline vk::Semaphore &getSemaphore() {
		return mSemaphore;
	}

	inline uint32_t getRenderingIndex() {
		return mRenderingIndex;
	}

	vk::Rect2D getRect();

private:

	void createSemaphore();

	uint32_t mRenderingIndex;

	VulkanContextPtr mContext;
	vk::Semaphore mSemaphore;
	vk::SurfaceKHR mSurface;
	vk::SwapchainKHR mSwapchain;
	vk::Format mFormat;

	vector<VulkanImage2DRef> mImages;

	bool mSwapchainInited = false;
	bool mFrameFailed = false;

	vk::Extent2D mExtent;
};
