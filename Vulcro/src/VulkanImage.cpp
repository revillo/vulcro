#include "VulkanImage.h"


vk::ImageUsageFlags VulkanImage::SAMPLED_STORAGE = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
vk::ImageUsageFlags VulkanImage::SAMPLED_COLOR_ATTACHMENT = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, glm::ivec3 size, vk::ImageType imageType)
	:mContext(ctx),
	mFormat(format),
	mUsage(usage),
	mSize(size),
	mImageType(imageType)
{

}

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::Image image, vk::Format format, glm::ivec3 size, vk::ImageType imageType)
	:mContext(ctx),
	mFormat(format),
	mImage(image),
	mSize(size),
	mImageType(imageType)
{
	// TODO create a sampler factor
	mSampler = mContext->getNearestSampler();
}

void VulkanImage::createImage()
{
	mImage = mContext->getDevice().createImage(
		vk::ImageCreateInfo(vk::ImageCreateFlags(),
			mImageType,
			mFormat,
			vk::Extent3D(mSize.x, mSize.y, mSize.z),
			1, //Mip Levels
			1, //Layers
			vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal,
			mUsage,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			vk::ImageLayout::eUndefined
		)
	);

	mImageCreated = true;
}

void VulkanImage::allocateDeviceMemory(vk::MemoryPropertyFlags memFlags)
{
	auto memProps = mContext->getPhysicalDevice().getMemoryProperties();

	auto req = mContext->getDevice().getImageMemoryRequirements(mImage);

	uint32_t memTypeIndex = 1000;
	auto reqBits = req.memoryTypeBits;
	auto p = memFlags;

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		auto type = memProps.memoryTypes[i];
		if ((reqBits & (1 << i)) && ((type.propertyFlags & p) == p)) {

			memTypeIndex = i;
			break;
		}
	}
	
	mMemorySize = req.size;

	mMemory = mContext->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(req.size, memTypeIndex)
	);

	mContext->getDevice().bindImageMemory(mImage, mMemory, 0);

	mMemoryAllocated = true;

	// If the memory is host visible then map the memory, keeping the memory mapped does not come at a performance cost
	if ((memFlags & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible)
	{
		mMemoryMapping = mContext->getDevice().mapMemory(
			mMemory,
			0,
			mMemorySize,
			vk::MemoryMapFlags()
		);
	}
}

void VulkanImage::transitionLayout(vk::CommandBuffer * cmd, vk::ImageLayout layout)
{
	vk::ImageMemoryBarrier imgbarr(
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eUndefined,
		layout,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		getImage(),
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0,
			1,
			0,
			1
		)
	);

	cmd->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlags(0),
		{},
		{},
		{ imgbarr }
	);
}

void VulkanImage::upload(uint64_t size, void* data)
{
	memcpy(mMemoryMapping, data, size);
}

void VulkanImage::createImageView(vk::ImageAspectFlags aspectFlags)
{
	vk::ComponentMapping cmap;
	cmap.r = vk::ComponentSwizzle::eR;
	cmap.g = vk::ComponentSwizzle::eG;
	cmap.b = vk::ComponentSwizzle::eB;
	cmap.a = vk::ComponentSwizzle::eA;

	vk::ImageSubresourceRange irange;
	irange.baseMipLevel = 0;
	irange.levelCount = 1;
	irange.setBaseArrayLayer(0);
	irange.layerCount = 6;
	irange.aspectMask = aspectFlags;

	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::eCube,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;
}


vk::DescriptorImageInfo VulkanImage::getDII()
{

	vk::ImageLayout layout;

	if (mUsage & vk::ImageUsageFlagBits::eColorAttachment)
		layout = vk::ImageLayout::eColorAttachmentOptimal;
	else if (mUsage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	else
		layout = vk::ImageLayout::eGeneral;
	return vk::DescriptorImageInfo(
		mSampler,
		mImageView,
		layout
	);
}

vk::DescriptorType VulkanImage::getDescriptorType()
{
	if (mUsage & vk::ImageUsageFlagBits::eSampled) {
		return vk::DescriptorType::eCombinedImageSampler;
	}
	else if (mUsage & vk::ImageUsageFlagBits::eStorage) {
		return vk::DescriptorType::eStorageImage;
	}
};

VulkanImage::~VulkanImage()
{
	mContext->getDevice().unmapMemory(
		mMemory
	);

	if (mViewCreated) mContext->getDevice().destroyImageView(mImageView);
	if (mImageCreated) mContext->getDevice().destroyImage(mImage);
	if (mMemoryAllocated) mContext->getDevice().freeMemory(mMemory);

	//if (_sampler) _ctx->getDevice().destroySampler(_sampler);

}

/**************************************************
 * 1D
 * ************************************************/

VulkanImage1D::VulkanImage1D(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, float size) : VulkanImage(ctx, usage, format, glm::ivec3(size, 1, 1), vk::ImageType::e1D)
{

}

VulkanImage1D::VulkanImage1D(VulkanContextRef ctx, vk::Image image, vk::Format format, int size) : VulkanImage(ctx, image, format, glm::ivec3(size, 1, 1), vk::ImageType::e1D)
{

}

void VulkanImage1D::createImageView(vk::ImageAspectFlags aspectFlags)
{
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
	irange.aspectMask = aspectFlags;

	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::e1D,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;
}

void VulkanImage1D::resize(float size)
{
	if (mViewCreated) mContext->getDevice().destroyImageView(mImageView);
	if (mImageCreated) mContext->getDevice().destroyImage(mImage);
	if (mMemoryAllocated) mContext->getDevice().freeMemory(mMemory);

	mSize.x = size;

	if (mImageCreated) createImage();
	if (mMemoryAllocated) allocateDeviceMemory();
	if (mViewCreated) createImageView();
}

/**************************************************
 * 2D
 * ************************************************/

VulkanImage2D::VulkanImage2D(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, glm::ivec2 size) : 
	VulkanImage(ctx, usage, format, glm::ivec3(size.x, size.y, 1),  vk::ImageType::e2D)
{

}

VulkanImage2D::VulkanImage2D(VulkanContextRef ctx, vk::Image image, vk::Format format, glm::ivec2 size) :
	VulkanImage(ctx, image, format, glm::ivec3(size.x, size.y, 1), vk::ImageType::e2D)
{

}

void VulkanImage2D::createImageView(vk::ImageAspectFlags aspectFlags) {
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
	irange.aspectMask = aspectFlags;

	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::e2D,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;
}

void VulkanImage2D::resize(ivec2 size)
{
	if (mViewCreated) mContext->getDevice().destroyImageView(mImageView);
	if (mImageCreated) mContext->getDevice().destroyImage(mImage);
	if (mMemoryAllocated) mContext->getDevice().freeMemory(mMemory);

	mSize = ivec3(size.x, size.y, 1);

	if (mImageCreated) createImage();
	if (mMemoryAllocated) allocateDeviceMemory();
	if (mViewCreated) createImageView();
}

/**************************************************
* 3D
* ************************************************/

VulkanImage3D::VulkanImage3D(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, glm::ivec3 size) :
	VulkanImage(ctx, usage, format, glm::ivec3(size.x, size.y, size.z), vk::ImageType::e3D)
{

}

VulkanImage3D::VulkanImage3D(VulkanContextRef ctx, vk::Image image, vk::Format format, glm::ivec3 size) :
	VulkanImage(ctx, image, format, glm::ivec3(size.x, size.y, size.z), vk::ImageType::e3D)
{

}

void VulkanImage3D::createImageView(vk::ImageAspectFlags aspectFlags)
{
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
	irange.aspectMask = aspectFlags;

	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::e3D,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;
}

/**************************************************
 * Cube
 * ************************************************/

VulkanImageCube::VulkanImageCube(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format) :
	VulkanImage(ctx, usage, format, glm::ivec3(size.x, size.y, 1), vk::ImageType::e2D)
{
	
}

void VulkanImageCube::createImageView(vk::ImageAspectFlags aspectFlags)
{
	vk::ComponentMapping cmap;
	cmap.r = vk::ComponentSwizzle::eR;
	cmap.g = vk::ComponentSwizzle::eG;
	cmap.b = vk::ComponentSwizzle::eB;
	cmap.a = vk::ComponentSwizzle::eA;

	vk::ImageSubresourceRange irange;
	irange.baseMipLevel = 0;
	irange.levelCount = 1;
	irange.setBaseArrayLayer(0);
	irange.layerCount = 6;
	irange.aspectMask = aspectFlags;

	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::eCube,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;
}
