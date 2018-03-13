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
	auto p = vk::MemoryPropertyFlagBits::eDeviceLocal;
	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();


	for (int i = 0; i < memProps.memoryTypeCount; i++) {
		auto type = memProps.memoryTypes[i];
		if ((reqBits >> i) && (type.propertyFlags & p) == p) {

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


	uint8 * pDevData;

	_ctx->getDevice().mapMemory(
		_memory,
		0,
		memReqs.size,
		vk::MemoryMapFlags(),
		(void **)&pDevData
	);

	memcpy(pDevData, data, size);

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
