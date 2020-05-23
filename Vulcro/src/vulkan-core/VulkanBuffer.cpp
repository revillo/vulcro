#include "VulkanBuffer.h"

const vk::MemoryPropertyFlags VulkanBuffer::CPU_ALOT = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
const vk::MemoryPropertyFlags VulkanBuffer::CPU_SOMETIMES = vk::MemoryPropertyFlagBits::eHostVisible;
const vk::MemoryPropertyFlags VulkanBuffer::CPU_NEVER = vk::MemoryPropertyFlagBits::eDeviceLocal;

const vk::BufferUsageFlags VulkanBuffer::UNIFORM_BUFFER = vk::BufferUsageFlagBits::eUniformBuffer;

const vk::BufferUsageFlags VulkanBuffer::TRANSFER_DST_STORAGE_BUFFER = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
const vk::BufferUsageFlags VulkanBuffer::TRANSFER_SRC_STORAGE_BUFFER = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc;

const vk::BufferUsageFlags VulkanBuffer::STORAGE_BUFFER = vk::BufferUsageFlagBits::eStorageBuffer;
const vk::BufferUsageFlags VulkanBuffer::STORAGE_TEXEL_BUFFER = vk::BufferUsageFlagBits::eStorageTexelBuffer;

VulkanBuffer::VulkanBuffer(VulkanContextPtr ctx, vk::BufferUsageFlags usage, uint64_t size, vk::MemoryPropertyFlags memFlags, void * data) :
	_ctx(ctx),
	_size(size),
	_usage(usage),
    mMemoryFlags(memFlags)
{
    // A buffer is a view into memeory, we create the view here,
    // but the memory still needs to be allocated
	mBuffer = _ctx->getDevice().createBuffer(
		vk::BufferCreateInfo(
			vk::BufferCreateFlags(),
			size,
			usage,
			vk::SharingMode::eExclusive,
			0, //queue
			nullptr
		)
	);

    // Ask the buffer what memory flags it requires
	auto memReqs =_ctx->getDevice().getBufferMemoryRequirements(mBuffer);

    auto memTypeRes = _ctx->getBestMemoryIndex(memReqs, memFlags);

    assert(memTypeRes.isValid);

    // Given our requirments, allocate memory
	mMemory = _ctx->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(
			memReqs.size,
			memTypeRes.value
		)
	);

#ifdef VULCRO_PRINT_ALLOCATIONS
    printf("VulkanBuffer: Allocating memory %lli \n", memReqs.size);
#endif
    // If a handle to data was passed, upload it
    if (data != nullptr)
    {
        upload(size, data);
    }

    // Bind the backed buffer with the allocated memoery
	_ctx->getDevice().bindBufferMemory(mBuffer, mMemory, 0);
}

VulkanBuffer::~VulkanBuffer()
{
    if (_mappedData)
    {
        unmap();
    }

	_ctx->getDevice().destroyBuffer(mBuffer);
	_ctx->getDevice().freeMemory(mMemory);

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
		&mBuffer,
		offsets
	);
}

void VulkanBuffer::bindIndex(vk::CommandBuffer * cmd, vk::IndexType type)
{
	cmd->bindIndexBuffer(
		mBuffer, 0, type
	);

}

void VulkanBuffer::upload(uint64_t size, void * data, uint64_t offset)
{

	auto * pData = getMapped(offset, size);

	memcpy(pData, data, size);

    flush(offset, size);

	unmap();
}

#include "VulkanTask.h"

void VulkanBuffer::copyBuffer(VulkanBufferRef srcBuffer, uint64_t size, uint64_t srcOffset, uint64_t dstOffset)
{

    // Create a new task and record our copy command
    auto pool = _ctx->makeTaskPool(vk::CommandPoolCreateFlagBits::eTransient);
    auto task = _ctx->makeTask(pool);

    copyBuffer(task, srcBuffer, size, srcOffset, dstOffset);
}


void VulkanBuffer::copyBuffer(VulkanTaskRef task, VulkanBufferRef srcBuffer, uint64_t size, uint64_t srcOffset, uint64_t dstOffset)
{
    auto BufferCopy = vk::BufferCopy(srcOffset, dstOffset, size);

    task->record([&](vk::CommandBuffer* cmd)
    {
        cmd->copyBuffer(srcBuffer->getBuffer(), getBuffer(), { BufferCopy });
    });

    // Execute the copy task
    task->execute(true);
}

vk::DescriptorBufferInfo VulkanBuffer::getDBI(vk::DeviceSize offset, vk::DeviceSize size)
{
	return vk::DescriptorBufferInfo(
		mBuffer,
		offset,
		size == 0 ? _size : static_cast<uint64_t>(size)
	);
}

vk::DescriptorType VulkanBuffer::getDescriptorType()
{
	if (_usage & vk::BufferUsageFlagBits::eUniformBuffer)
    {
		return vk::DescriptorType::eUniformBuffer;
	}
	else if (_usage & vk::BufferUsageFlagBits::eStorageBuffer)
    {
		return vk::DescriptorType::eStorageBuffer;
	}
	else if (_usage & vk::BufferUsageFlagBits::eStorageTexelBuffer)
    {
		return vk::DescriptorType::eStorageTexelBuffer;
	}
	else
    {
		throw "Unsupported buffer as descriptor";
	}
}

static uint64_t roundUp(uint64_t number, uint64_t multiplier)
{
    uint64_t remainder = number % multiplier;
    if (remainder == 0)
        return number;

    return number + multiplier - remainder;
}

void * VulkanBuffer::getMapped(uint64_t offset, uint64_t size){

    assert(mMemoryFlags & vk::MemoryPropertyFlagBits::eHostVisible);
    
    if ((_mappedData != nullptr) && offset == 0) {
        return _mappedData;
    }

    if (size == _size || size == VK_WHOLE_SIZE)
    {
        size = VK_WHOLE_SIZE;
    }
    else
    {
        auto multiplier = _ctx->getPhysicalDeviceProperties().limits.nonCoherentAtomSize;
        size = roundUp(size, multiplier);
    }

	void * pData = _ctx->getDevice().mapMemory(
		mMemory,
		offset,
		size,
		vk::MemoryMapFlags()
	);

    if (offset == 0) {
        _mappedData = pData;
    }

	return pData;
}


void VulkanBuffer::flush(uint64_t offset, uint64_t size)
{
    if (!(mMemoryFlags & vk::MemoryPropertyFlagBits::eHostVisible))
    {
        return;
    }

    //No need to flush host coherent memory
    if (mMemoryFlags & vk::MemoryPropertyFlagBits::eHostCoherent)
    {
        return;
    }


    if (size == _size || size == VK_WHOLE_SIZE)
    {
        size = VK_WHOLE_SIZE;
    }
    else
    {
        auto multiplier = _ctx->getPhysicalDeviceProperties().limits.nonCoherentAtomSize;
        size = roundUp(size, multiplier);
    }
    

    _ctx->getDevice().flushMappedMemoryRanges(vk::MappedMemoryRange{
        mMemory, offset, size
    });
}

void VulkanBuffer::unmap()
{
    if (!(mMemoryFlags & vk::MemoryPropertyFlagBits::eHostVisible))
    {
        return;
    }

	_ctx->getDevice().unmapMemory(
		mMemory
	);

    _mappedData = nullptr;
}

void VulkanBuffer::createView(vk::Format format)
{
	_view = _ctx->getDevice().createBufferView(vk::BufferViewCreateInfo(
		vk::BufferViewCreateFlags(),
		mBuffer,
		format,
		0,
		VK_WHOLE_SIZE));
}

VulkanBufferWindow::VulkanBufferWindow(VulkanBufferRef buffer, vk::DeviceSize offset, vk::DeviceSize size) :
	_buffer(buffer), _size(size), _offset(offset)
{

}
