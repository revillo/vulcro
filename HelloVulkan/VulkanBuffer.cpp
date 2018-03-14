#include "VulkanBuffer.h"





VulkanBuffer::VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint64 size, void * data) :
	_ctx(ctx)
{

	_buffer = _ctx->getDevice().createBuffer(
		vk::BufferCreateInfo(
			vk::BufferCreateFlags(),
			size,
			usage,
			vk::SharingMode::eExclusive,
			0, //queue
			nullptr
		)
	);

	auto memReqs =_ctx->getDevice().getBufferMemoryRequirements(
		_buffer
	);

	uint32 memTypeIndex = 1000;
	auto reqBits = memReqs.memoryTypeBits;
	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();
	
	vk::MemoryPropertyFlags propMask = vk::MemoryPropertyFlagBits::eHostVisible;
	propMask |= vk::MemoryPropertyFlagBits::eHostCoherent;
	
	for (uint32 i = 0; i < memProps.memoryTypeCount; i++) {
		auto type = memProps.memoryTypes[i];
		if ((reqBits & (1 << i)) && (type.propertyFlags & propMask) == propMask) {

			memTypeIndex = i;
			break;
		}
	}

	_memory = _ctx->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(
			memReqs.size,
			memTypeIndex
		)
	);


	uint8 * pData;

	_ctx->getDevice().mapMemory(
		_memory,
		0,
		memReqs.size,
		vk::MemoryMapFlags(),
		(void **)&pData
	);

	memcpy(pData, data, size);

	_ctx->getDevice().unmapMemory(
		_memory
	);

	
	_ctx->getDevice().bindBufferMemory(
		_buffer,
		_memory,
		0
	);

}

VulkanBuffer::~VulkanBuffer()
{
}

void VulkanBuffer::bind(vk::CommandBuffer &cmd)
{
	vk::DeviceSize offsets[1] = { 0 };

	cmd.bindVertexBuffers(
		0,
		1,
		&_buffer,
		offsets
	);
}
