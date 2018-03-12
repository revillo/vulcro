#include "VulkanSwapchain.h"


void VulkanSwapchain::init(vk::SurfaceKHR surface) {
	
	auto &physicalDevice = _ctx->getPhysicalDevice();
	auto surfCap = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto surfFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	auto presentMode = vk::PresentModeKHR::eImmediate;

	for (auto mode : presentModes) {
		if (mode == vk::PresentModeKHR::eMailbox) {
			presentMode = mode; break;
		}

		if (mode == vk::PresentModeKHR::eFifo) {
			presentMode = mode;
		}
	}


	_swapchain = _ctx->getDevice().createSwapchainKHR(
		vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(),
			surface,
			surfCap.minImageCount,
			surfFormats[0].format,
			surfFormats[0].colorSpace,
			surfCap.currentExtent,
			1, // imageArrayLayers
			vk::ImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment),
			vk::SharingMode::eExclusive,
			0, // Queue Family Index Count
			(const uint32_t*)NULL, //Indices
			surfCap.currentTransform, //vk::SurfaceTransformFlagBitsKHR::eIdentity,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			presentMode,
			VK_TRUE, // Clipping
			swapchainPresent ? _swapchain : NULL //TODO, pass in old swapchain
		)
	);


	auto swapImages = _ctx->getDevice().getSwapchainImagesKHR(_swapchain);
	vk::ComponentMapping cmap;
	cmap.r = vk::ComponentSwizzle::eR;
	cmap.g = vk::ComponentSwizzle::eG;
	cmap.b = vk::ComponentSwizzle::eB;
	cmap.a = vk::ComponentSwizzle::eA;

	vk::ImageSubresourceRange irange;
	irange.baseMipLevel = 0;
	irange.levelCount = 1;
	irange.setBaseArrayLayer(0);
	irange.layerCount = 1;

	for (int i = 0; i < swapImages.size(); i++) {

		_imageViews.push_back(_ctx->getDevice().createImageView(
			vk::ImageViewCreateInfo(
				vk::ImageViewCreateFlags(),
				swapImages[i],
				vk::ImageViewType::e2D,
				surfFormats[0].format,
				cmap,
				irange
			)
		));


	}


	swapchainPresent = true;

}

VulkanSwapchain::VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface) :
	_ctx(ctx)
{
	init(surface);
}

VulkanSwapchain::~VulkanSwapchain()
{
}
