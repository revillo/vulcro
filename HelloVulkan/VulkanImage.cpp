#include "VulkanImage.h"



VulkanImage::VulkanImage(VulkanContextRef ctx, glm::ivec2 size, vk::Format format, vk::ImageUsageFlagBits usage)
	:_ctx(ctx)
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


VulkanImage::~VulkanImage()
{
}
