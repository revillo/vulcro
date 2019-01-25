#include "VulkanImage.h"


vk::ImageUsageFlags VulkanImage::SAMPLED_STORAGE = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
vk::ImageUsageFlags VulkanImage::SAMPLED_COLOR_ATTACHMENT = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, glm::ivec3 size, vk::ImageType imageType)
	:_ctx(ctx),
	_format(format),
	_usage(usage),
	mSize(size),
	mImageType(imageType)
{

}

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::Image image, vk::Format format, glm::ivec3 size, vk::ImageType imageType)
	:_ctx(ctx),
	_format(format),
	_image(image),
	mSize(size),
	mImageType(imageType)
{
	createSampler();
}

void VulkanImage::createImage()
{
	_image = _ctx->getDevice().createImage(
		vk::ImageCreateInfo(vk::ImageCreateFlags(),
			mImageType,
			_format,
			vk::Extent3D(mSize.x, mSize.y, mSize.z),
			1, //Mip Levels
			1, //Layers
			vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal,
			_usage,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			vk::ImageLayout::eUndefined
		)
	);

	_imageCreated = true;
}

void VulkanImage::allocateDeviceMemory(vk::MemoryPropertyFlags memFlags)
{
	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();

	auto req = _ctx->getDevice().getImageMemoryRequirements(_image);

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
	
	_memorySize = req.size;

	_memory = _ctx->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(req.size, memTypeIndex)
	);

	_ctx->getDevice().bindImageMemory(_image, _memory, 0);

	_memoryAllocated = true;

	// If the memory is host visible then map the memory, keeping the memory mapped does not come at a performance cost
	if ((memFlags & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible)
	{
		mMemoryMapping = _ctx->getDevice().mapMemory(
			_memory,
			0,
			_memorySize,
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

void VulkanImage::createSampler()
{
	_sampler = _ctx->getNearestSampler();
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

	_imageView = _ctx->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			_image,
			vk::ImageViewType::eCube,
			_format,
			cmap,
			irange
		)
	);

	_viewCreated = true;
}


vk::DescriptorImageInfo VulkanImage::getDII()
{

	vk::ImageLayout layout;

	if (_usage & vk::ImageUsageFlagBits::eColorAttachment)
		layout = vk::ImageLayout::eColorAttachmentOptimal;
	else if (_usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	else
		layout = vk::ImageLayout::eGeneral;
	return vk::DescriptorImageInfo(
		_sampler,
		_imageView,
		layout
	);
}

vk::DescriptorType VulkanImage::getDescriptorType()
{
	if (_usage & vk::ImageUsageFlagBits::eSampled) {
		return vk::DescriptorType::eCombinedImageSampler;
	}
	else if (_usage & vk::ImageUsageFlagBits::eStorage) {
		return vk::DescriptorType::eStorageImage;
	}
};

VulkanImage::~VulkanImage()
{
	_ctx->getDevice().unmapMemory(
		_memory
	);

	if (_viewCreated) _ctx->getDevice().destroyImageView(_imageView);
	if (_imageCreated) _ctx->getDevice().destroyImage(_image);
	if (_memoryAllocated) _ctx->getDevice().freeMemory(_memory);

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

void VulkanImage1D::createImageView(vk::ImageAspectFlags aspectFlags) {
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

	_imageView = _ctx->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			_image,
			vk::ImageViewType::e1D,
			_format,
			cmap,
			irange
		)
	);

	_viewCreated = true;
}

void VulkanImage1D::resize(float size)
{
	if (_viewCreated) _ctx->getDevice().destroyImageView(_imageView);
	if (_imageCreated) _ctx->getDevice().destroyImage(_image);
	if (_memoryAllocated) _ctx->getDevice().freeMemory(_memory);

	mSize.x = size;

	if (_imageCreated) createImage();
	if (_memoryAllocated) allocateDeviceMemory();
	if (_viewCreated) createImageView();
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

	_imageView = _ctx->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			_image,
			vk::ImageViewType::e2D,
			_format,
			cmap,
			irange
		)
	);

	_viewCreated = true;
}

void VulkanImage2D::resize(ivec2 size)
{
	if (_viewCreated) _ctx->getDevice().destroyImageView(_imageView);
	if (_imageCreated) _ctx->getDevice().destroyImage(_image);
	if (_memoryAllocated) _ctx->getDevice().freeMemory(_memory);

	mSize = ivec3(size.x, size.y, 1);

	if (_imageCreated) createImage();
	if (_memoryAllocated) allocateDeviceMemory();
	if (_viewCreated) createImageView();
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

void VulkanImage3D::createImageView(vk::ImageAspectFlags aspectFlags) {
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

	_imageView = _ctx->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			_image,
			vk::ImageViewType::e3D,
			_format,
			cmap,
			irange
		)
	);

	_viewCreated = true;
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

	_imageView = _ctx->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			_image,
			vk::ImageViewType::eCube,
			_format,
			cmap,
			irange
		)
	);

	_viewCreated = true;
}
