#include "VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(VulkanContextPtr ctx, vk::SurfaceKHR surface) :
	mContext(ctx),
	mSurface(surface)
{
	init(surface);
	createSemaphore();
}

VulkanSwapchain::~VulkanSwapchain()
{
	mContext->getDevice().destroySwapchainKHR(mSwapchain);
	mContext->getDevice().destroySemaphore(mSemaphore);
}


bool VulkanSwapchain::init(vk::SurfaceKHR surface) {
	
	auto &physicalDevice = mContext->getPhysicalDevice();
	
	bool supported = physicalDevice.getSurfaceSupportKHR(0, surface);

	auto surfCap = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto surfFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
	mFormat = surfFormats[0].format;
	mExtent = surfCap.currentExtent;

	if (mExtent.width == 0) return false;

	auto presentMode = vk::PresentModeKHR::eImmediate;

	for (auto mode : presentModes) {
		if (mode == vk::PresentModeKHR::eMailbox) {
			presentMode = mode; break;
		}

		if (mode == vk::PresentModeKHR::eFifo) {
			presentMode = mode;
		}
	}

	vk::SwapchainKHR oldSwapchain = mSwapchainInited ? mSwapchain : vk::SwapchainKHR();

	mSwapchain = mContext->getDevice().createSwapchainKHR(
		vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(),
			surface,
			surfCap.minImageCount,
			mFormat,
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

	mImages.clear();

	auto swapImages = mContext->getDevice().getSwapchainImagesKHR(mSwapchain);
	
	for (auto &swapImage : swapImages)
	{
		auto vi = mContext->makeImage2D(swapImage, mFormat, ivec2(surfCap.currentExtent.width, surfCap.currentExtent.height));
		vi->createImageView(vk::ImageAspectFlagBits::eColor);
		mImages.push_back(vi);
	}

	if (mSwapchainInited)
	{
		mContext->getDevice().destroySwapchainKHR(oldSwapchain);
	}

	mSwapchainInited = true;

	return true;
}

void VulkanSwapchain::createSemaphore()
{
	mSemaphore = mContext->getDevice().createSemaphore(
		vk::SemaphoreCreateInfo()
	);
}

bool VulkanSwapchain::present(vector<vk::Semaphore> inSems) {
	try {
		mContext->getQueue().presentKHR(
			vk::PresentInfoKHR(
				static_cast<uint32_t>(inSems.size()),
				inSems.size() > 0 ? &inSems[0] : nullptr,
				1,
				&mSwapchain,
				&mRenderingIndex
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

	if(mExtent.width == 0) return false;

	try {
		auto ret = mContext->getDevice().acquireNextImageKHR(
			mSwapchain,
			UINT64_MAX,
			mSemaphore,
			vk::Fence()
		);

		mRenderingIndex = ret.value;
		mFrameFailed = false;
		return true;
	}
	catch (vk::SystemError error) {
		return false;
	}
}

bool VulkanSwapchain::resize()
{
	return init(mSurface);
}

vk::Rect2D VulkanSwapchain::getRect()
{
	return vk::Rect2D(
		vk::Offset2D(0, 0),
		mExtent
	);
}

