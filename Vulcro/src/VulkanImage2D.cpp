#include "VulkanImage2D.h"


VulkanImage2D::VulkanImage2D(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, glm::ivec2 size) : 
	VulkanImage(ctx, usage, format),
	_size(size)
{

}

VulkanImage2D::VulkanImage2D(VulkanContextRef ctx, vk::Image image, vk::Format format, glm::ivec2 size) :
	VulkanImage(ctx, image, format),
	_size(size)
{

}

void VulkanImage2D::createImage()
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

	_size = size;

	if (_imageCreated) createImage();
	if (_memoryAllocated) allocateDeviceMemory();
	if (_viewCreated) createImageView();

}