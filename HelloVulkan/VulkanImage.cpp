#include "VulkanImage.h"

VulkanImage::VulkanImage(VulkanContextRef ctx, glm::ivec2 size, vk::Format format, vk::ImageUsageFlagBits usage)
	:_ctx(ctx),
	_format(format),
	_size(size)
{
	_image = _ctx->getDevice().createImage(
		vk::ImageCreateInfo(vk::ImageCreateFlags(),
			vk::ImageType::e2D,
			format,
			vk::Extent3D(size.x, size.y, 1),
			1, //Mip Levels
			1, //Layers
			vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
			usage,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			vk::ImageLayout::eUndefined)
	);

}

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::Image image, glm::ivec2 size, vk::Format format)
	:_ctx(ctx),
	_format(format),
	_size(size),
	_image(image)
{
	
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

}

VulkanImage::~VulkanImage()
{
}
