#include "VulkanBuffer.h"





VulkanBuffer::VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint64 size, void * data) :
	_ctx(ctx),
	_size(size)
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


	void * pData;

	pData = _ctx->getDevice().mapMemory(
		_memory,
		0,
		size
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
	_ctx->getDevice().destroyBuffer(_buffer);
	_ctx->getDevice().freeMemory(_memory);
}

void VulkanBuffer::bindVertex(vk::CommandBuffer &cmd)
{
	vk::DeviceSize offsets[1] = { 0 };

	cmd.bindVertexBuffers(
		0,
		1,
		&_buffer,
		offsets
	);
}

void VulkanBuffer::bindIndex(vk::CommandBuffer & cmd)
{
	cmd.bindIndexBuffer(
		_buffer, 0, vk::IndexType::eUint16
	);

}

vk::DescriptorBufferInfo VulkanBuffer::getDBI(uint32 offset, int64 size)
{
	return vk::DescriptorBufferInfo(
		_buffer,
		offset,
		size == -1 ? _size : static_cast<uint64>(size)
	);
}

void * VulkanBuffer::getData()
{

	void * pData = _ctx->getDevice().mapMemory(
		_memory,
		0,
		96,
		vk::MemoryMapFlags()
	);

	return pData;
}