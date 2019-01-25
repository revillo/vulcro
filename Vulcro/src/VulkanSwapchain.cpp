#include "VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(VulkanContextRef ctx, vk::SurfaceKHR surface) :
	_ctx(ctx),
	_surface(surface)
{
	init(surface);
	createSemaphore();
}

bool VulkanSwapchain::init(vk::SurfaceKHR surface) {
	
	auto &physicalDevice = _ctx->getPhysicalDevice();
	
	bool supported = physicalDevice.getSurfaceSupportKHR(0, surface);

	auto surfCap = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto surfFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
	_format = surfFormats[0].format;
	_extent = surfCap.currentExtent;

	if (_extent.width == 0) return false;

	auto presentMode = vk::PresentModeKHR::eImmediate;

	for (auto mode : presentModes) {
		if (mode == vk::PresentModeKHR::eMailbox) {
			presentMode = mode; break;
		}

		if (mode == vk::PresentModeKHR::eFifo) {
			presentMode = mode;
		}
	}

	vk::SwapchainKHR oldSwapchain = swapchainInited ? _swapchain : vk::SwapchainKHR();

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
			oldSwapchain
		)
	);

	_images.clear();

	auto swapImages = _ctx->getDevice().getSwapchainImagesKHR(_swapchain);
	
	for (auto &swapImage : swapImages)
	{
		auto vi = _ctx->makeImage2D(swapImage, _format, ivec2(surfCap.currentExtent.width, surfCap.currentExtent.height));
		vi->createImageView(vk::ImageAspectFlagBits::eColor);
		_images.push_back(vi);
	}

	if (swapchainInited)
	{
		_ctx->getDevice().destroySwapchainKHR(oldSwapchain);
	}

	swapchainInited = true;

	return true;
}

void VulkanSwapchain::createSemaphore()
{
	_semaphore = _ctx->getDevice().createSemaphore(
		vk::SemaphoreCreateInfo()
	);
}

bool VulkanSwapchain::present(vector<vk::Semaphore> inSems) {
	try {
		_ctx->getQueue().presentKHR(
			vk::PresentInfoKHR(
				static_cast<uint32_t>(inSems.size()),
				inSems.size() > 0 ? &inSems[0] : nullptr,
				1,
				&_swapchain,
				&_renderingIndex
			)
		);

		return true;
		
	}
	catch (vk::SystemError error) {
		return false;
	}
}

bool VulkanSwapchain::nextFrame()
{

	if (_extent.width == 0) return false;

	try {


		auto ret = _ctx->getDevice().acquireNextImageKHR(
			_swapchain,
			UINT64_MAX,
			_semaphore,
			vk::Fence()
		);

		_renderingIndex = ret.value;
		_frameFailed = false;
		return true;
	}
	catch (vk::SystemError error) {
		return false;
	}
	}

bool VulkanSwapchain::resize()
{
	return init(_surface);
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
	_ctx->getDevice().destroySwapchainKHR(_swapchain);
	_ctx->getDevice().destroySemaphore(_semaphore);
}
