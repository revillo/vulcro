#include "VulkanSwapchain.h"


void VulkanSwapchain::init(vk::SurfaceKHR surface) {
	
	auto &physicalDevice = _ctx->getPhysicalDevice();
	auto surfCap = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto surfFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
	_format = surfFormats[0].format;

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
			_format,
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
			swapchainInited ? _swapchain : vk::SwapchainKHR()
		)
	);


	auto swapImages = _ctx->getDevice().getSwapchainImagesKHR(_swapchain);
	
	for (auto swapImage : swapImages) {
		auto vi = make_shared<VulkanImage>(_ctx, swapImage, ivec2(surfCap.currentExtent.width, surfCap.currentExtent.height), _format);
		vi->createImageView(vk::ImageAspectFlagBits::eColor);
		_images.push_back(vi);
	}

	swapchainInited = true;
}

VulkanSwapchain::VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface) :
	_ctx(ctx),
	_surface(surface)
{
	init(surface);
}

VulkanSwapchain::~VulkanSwapchain()
{
}
