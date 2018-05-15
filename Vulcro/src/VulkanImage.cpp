#include "VulkanImage.h"


vk::ImageUsageFlags VulkanImage::SAMPLED_STORAGE = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
vk::ImageUsageFlags VulkanImage::SAMPLED_COLOR_ATTACHMENT = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format)
	:_ctx(ctx),
	_format(format),
	_size(size),
	_usage(usage)
{


}

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::Image image, glm::ivec2 size, vk::Format format)
	:_ctx(ctx),
	_format(format),
	_size(size),
	_image(image)
{
	createSampler();

}

void VulkanImage::allocateDeviceMemory(vk::MemoryPropertyFlags memFlags)
{
	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();


	auto req = _ctx->getDevice().getImageMemoryRequirements(_image);

	uint32 memTypeIndex = 1000;
	auto reqBits = req.memoryTypeBits;
	auto p = memFlags;

	for (uint32 i = 0; i < memProps.memoryTypeCount; i++) {
		auto type = memProps.memoryTypes[i];
		if ((reqBits & (1 << i)) && ((type.propertyFlags & p) == p)) {

			memTypeIndex = i;
			break;
		}
	}


	_memory = _ctx->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(req.size, memTypeIndex)
	);

	_ctx->getDevice().bindImageMemory(_image, _memory, 0);


	_memoryAllocated = true;
}



void VulkanImage::createImage()
{
	_image = _ctx->getDevice().createImage(
		vk::ImageCreateInfo(vk::ImageCreateFlags(),
			vk::ImageType::e2D,
			_format,
			vk::Extent3D(_size.x, _size.y, 1),
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

void VulkanImage::createImageView(vk::ImageAspectFlags aspectFlags) {
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

void VulkanImage::createSampler()
{
	_sampler = _ctx->getNearestSampler();
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

	if (_viewCreated) _ctx->getDevice().destroyImageView(_imageView);
	if (_imageCreated) _ctx->getDevice().destroyImage(_image);
	if (_memoryAllocated) _ctx->getDevice().freeMemory(_memory);

	//if (_sampler) _ctx->getDevice().destroySampler(_sampler);

}

void VulkanImage::resize(ivec2 size)
{
	if (_viewCreated) _ctx->getDevice().destroyImageView(_imageView);
	if (_imageCreated) _ctx->getDevice().destroyImage(_image);
	if (_memoryAllocated) _ctx->getDevice().freeMemory(_memory);
	
	_size = size;

	if (_imageCreated) createImage();
	if (_memoryAllocated) allocateDeviceMemory();
	if (_viewCreated) createImageView();

}


void VulkanCubeImage::createImage()
{
	_image = _ctx->getDevice().createImage(
		vk::ImageCreateInfo(vk::ImageCreateFlagBits::eCubeCompatible,
			vk::ImageType::e2D,
			_format,
			vk::Extent3D(_size.x, _size.y, 1),
			1, //Mip Levels
			6, //Layers
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

VulkanCubeImage::VulkanCubeImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format) :
	VulkanImage(ctx, usage, size, format)
{
	//_sampler = _ctx->getLinearSampler();
}

void VulkanCubeImage::createImageView(vk::ImageAspectFlags aspectFlags)
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
