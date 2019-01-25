#include "VulkanImageCube.h"


VulkanImageCube::VulkanImageCube(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format) :
	VulkanImage(ctx, usage, format),
	_size(size)
{
	
}

void VulkanImageCube::createImage()
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
