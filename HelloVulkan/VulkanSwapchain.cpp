#include "VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface) :
	_ctx(ctx),
	_surface(surface)
{
	init(surface);
	createSemaphore();
	_fence = _ctx->getDevice().createFence(
		vk::FenceCreateInfo()
	);


}

void VulkanSwapchain::init(vk::SurfaceKHR surface) {
	
	auto &physicalDevice = _ctx->getPhysicalDevice();
	
	bool supported = physicalDevice.getSurfaceSupportKHR(0, surface);

	auto surfCap = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto surfFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
	_format = surfFormats[0].format;
	_extent = surfCap.currentExtent;

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
			vk::ImageUsageFlagBits::eColorAttachment,
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
	
	for (auto &swapImage : swapImages) {
		auto vi = make_shared<VulkanImage>(_ctx, swapImage, ivec2(surfCap.currentExtent.width, surfCap.currentExtent.height), _format);
		vi->createImageView(vk::ImageAspectFlagBits::eColor);
		_images.push_back(vi);
	}

	swapchainInited = true;


}

void VulkanSwapchain::createSemaphore()
{
	_semaphore = _ctx->getDevice().createSemaphore(
		vk::SemaphoreCreateInfo()
	);
}

uint32 VulkanSwapchain::getNextIndex()
{

	auto ret = _ctx->getDevice().acquireNextImageKHR(
		_swapchain,
		UINT64_MAX,
		_semaphore,
		_fence
	);

	return ret.value;
}

vk::Rect2D VulkanSwapchain::getRect()
{
	return vk::Rect2D(
		vk::Offset2D(0, 0),
		_extent
	);
}



VulkanSwapchain::~VulkanSwapchain()
{
}
