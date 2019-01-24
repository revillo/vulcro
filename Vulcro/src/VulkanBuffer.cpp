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

void VulkanBuffer::bindVertex(vk::CommandBuffer * cmd, vk::DeviceSize offset)
{
	vk::DeviceSize offsets[1] = { offset };

	cmd->bindVertexBuffers(
		0,
		1,
		&_buffer,
		offsets
	);
}

void VulkanBuffer::bindIndex(vk::CommandBuffer * cmd, vk::IndexType type)
{
	cmd->bindIndexBuffer(
		_buffer, 0, type
	);

}

void VulkanBuffer::upload(uint64_t size, void * data, uint32_t offset)
{

	auto * pData = getMapped(offset, size);

	memcpy(pData, data, size);

	unmap();
}

vk::DescriptorBufferInfo VulkanBuffer::getDBI(vk::DeviceSize offset, vk::DeviceSize size)
{
	return vk::DescriptorBufferInfo(
		_buffer,
		offset,
		size == 0 ? _size : static_cast<uint64_t>(size)
	);
}

vk::DescriptorType VulkanBuffer::getDescriptorType()
{
	if (_usage & vk::BufferUsageFlagBits::eUniformBuffer) {
		return vk::DescriptorType::eUniformBuffer;
	}
	else if (_usage & vk::BufferUsageFlagBits::eStorageBuffer) {
		return vk::DescriptorType::eStorageBuffer;
	}
	else if (_usage & vk::BufferUsageFlagBits::eStorageTexelBuffer) {
		return vk::DescriptorType::eStorageTexelBuffer;
	}
	else {
		throw "Unsupported buffer as descriptor";
	}
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

void VulkanBuffer::createView(vk::Format format)
{
	_view = _ctx->getDevice().createBufferView(vk::BufferViewCreateInfo(
		vk::BufferViewCreateFlags(),
		_buffer,
		format,
		0,
		VK_WHOLE_SIZE));
}

VulkanBufferWindow::VulkanBufferWindow(VulkanBufferRef buffer, vk::DeviceSize offset, vk::DeviceSize size) :
	_buffer(buffer), _size(size), _offset(offset)
{

}
