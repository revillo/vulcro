#include "VulkanRenderer.h"



VulkanRenderer::VulkanRenderer(VulkanContextRef ctx, ivec2 size) :
	_ctx(ctx),
	_size(size)
{
	createDepthBuffer();
}

void VulkanRenderer::createDepthBuffer() {
	auto imgRef = make_shared<VulkanImage>(_ctx, _size, vk::Format::eD16Unorm, vk::ImageUsageFlagBits::eDepthStencilAttachment);

	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();


	auto req = _ctx->getDevice().getImageMemoryRequirements(imgRef->deviceImage());

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

	_ctx->getDevice().bindImageMemory(imgRef->deviceImage(), memory, 0);
}

VulkanRenderer::~VulkanRenderer()
{
}
