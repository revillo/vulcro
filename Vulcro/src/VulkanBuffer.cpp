#include "VulkanBuffer.h"


const vk::MemoryPropertyFlags VulkanBuffer::CPU_ALOT = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
const vk::MemoryPropertyFlags VulkanBuffer::CPU_NEVER = vk::MemoryPropertyFlagBits::eDeviceLocal;

const vk::BufferUsageFlags VulkanBuffer::UNIFORM_BUFFER = vk::BufferUsageFlagBits::eUniformBuffer;
const vk::BufferUsageFlags VulkanBuffer::CLEARABLE_STORAGE_BUFFER = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
const vk::BufferUsageFlags VulkanBuffer::STORAGE_BUFFER = vk::BufferUsageFlagBits::eStorageBuffer;
const vk::BufferUsageFlags VulkanBuffer::STORAGE_TEXEL_BUFFER = vk::BufferUsageFlagBits::eStorageTexelBuffer;


VulkanBuffer::VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint64_t size, vk::MemoryPropertyFlags memFlags, void * data) :
	_ctx(ctx),
	_size(size),
	_usage(usage)
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

	uint32_t memTypeIndex = 1000;
	auto reqBits = memReqs.memoryTypeBits;
	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();
	
	vk::MemoryPropertyFlags propMask = memFlags;
	
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
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

	if (data != nullptr)
		upload(size, data);

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

	if (_view) {
		_ctx->getDevice().destroyBufferView(_view);
	}
}

void VulkanBuffer::bindVertex(vk::CommandBuffer * cmd)
{
	vk::DeviceSize offsets[1] = { 0 };

	cmd->bindVertexBuffers(
		0,
		1,
		&_buffer,
		offsets
	);
}

void VulkanBuffer::bindIndex(vk::CommandBuffer * cmd)
{
	cmd->bindIndexBuffer(
		_buffer, 0, vk::IndexType::eUint16
	);

}

void VulkanBuffer::upload(uint64_t size, void * data, uint32_t offset)
{

	auto * pData = getMapped(offset, size);

	memcpy(pData, data, size);

	unmap();
}

vk::DescriptorBufferInfo VulkanBuffer::getDBI(uint32_t offset, int64_t size)
{
	return vk::DescriptorBufferInfo(
		_buffer,
		offset,
		size == -1 ? _size : static_cast<uint64_t>(size)
	);
}

void * VulkanBuffer::getMapped(uint32_t offset, int64_t size){

	void * pData = _ctx->getDevice().mapMemory(
		_memory,
		offset,
		size < 0 ? _size : size,
		vk::MemoryMapFlags()
	);

	return pData;
}

void VulkanBuffer::unmap() {
	_ctx->getDevice().unmapMemory(
		_memory
	);
}
