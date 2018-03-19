#include "VulkanImage.h"

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format)
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

	_imageCreated = true;

}

void VulkanImage::allocateDeviceMemory()
{
	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();


	auto req = _ctx->getDevice().getImageMemoryRequirements(_image);

	uint32 memTypeIndex = 1000;
	auto reqBits = req.memoryTypeBits;
	auto p = vk::MemoryPropertyFlagBits::eDeviceLocal;

	for (uint32 i = 0; i < memProps.memoryTypeCount; i++) {
		auto type = memProps.memoryTypes[i];
		if ((reqBits && (1 << i)) && (type.propertyFlags & p) == p) {

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

	_viewCreated = true;

}

void VulkanImage::createSampler()
{
	_sampler = _ctx->getDevice().createSampler(
		vk::SamplerCreateInfo(
			vk::SamplerCreateFlags(),
			vk::Filter::eLinear, //Mag Filter
			vk::Filter::eLinear, //Min Filter
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat, //U
			vk::SamplerAddressMode::eRepeat,  //V
			vk::SamplerAddressMode::eRepeat, //W
			0.0, //mip lod bias
			0, //Anisotropy Enable
			1.0f,
			0, //Compare Enable
			vk::CompareOp::eAlways,
			0.0f, //min lod
			0.0f, //max lod
			vk::BorderColor::eFloatOpaqueBlack,
			0 //Unnormalized Coordinates

		)
	);
}

vk::DescriptorImageInfo VulkanImage::getDII()
{
	return vk::DescriptorImageInfo(
		_sampler,
		_imageView,
		vk::ImageLayout::eShaderReadOnlyOptimal
	);
}

VulkanImage::~VulkanImage()
{

	if (_viewCreated) _ctx->getDevice().destroyImageView(_imageView);
	if (_imageCreated) _ctx->getDevice().destroyImage(_image);
	if (_memoryAllocated) _ctx->getDevice().freeMemory(_memory);

	if (_sampler) _ctx->getDevice().destroySampler(_sampler);

}
