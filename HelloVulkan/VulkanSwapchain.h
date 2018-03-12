#pragma once

#include "vulkan\vulkan.hpp"
#include "VulkanContext.h"
#include "General.h"

class VulkanSwapchain
{
public:

	VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface);
	~VulkanSwapchain();

	void init(vk::SurfaceKHR surface);


private:

	VulkanContextRef _ctx;

	vk::SwapchainKHR _swapchain;
	vector<vk::ImageView> _imageViews;

	bool swapchainPresent = false;

};

typedef shared_ptr<VulkanSwapchain> VulkanSwapchainRef;