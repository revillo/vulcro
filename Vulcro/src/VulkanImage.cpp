#include "VulkanImage.h"


vk::ImageUsageFlags VulkanImage::SAMPLED_STORAGE = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
vk::ImageUsageFlags VulkanImage::SAMPLED_COLOR_ATTACHMENT = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format)
	:_ctx(ctx),
	_format(format),
	_usage(usage)
{

}

VulkanImage::VulkanImage(VulkanContextRef ctx, vk::Image image, vk::Format format)
	:_ctx(ctx),
	_format(format),
	_image(image)
{
	createSampler();
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