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

void VulkanImage::allocateDeviceMemory()
{
	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();


	auto req = _ctx->getDevice().getImageMemoryRequirements(_image);

	uint32 memTypeIndex = 1000;
	auto reqBits = req.memoryTypeBits;
	auto p = vk::MemoryPropertyFlagBits::eDeviceLocal;


	for (int i = 0; i < memProps.memoryTypeCount; i++) {
		auto type = memProps.memoryTypes[i];
		if ((reqBits >> i) && (type.propertyFlags & p) == p) {

			memTypeIndex = i;
			break;
		}
	}


	auto memory = _ctx->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(req.size, memTypeIndex)
	);

	_ctx->getDevice().bindImageMemory(_image, memory, 0);

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
