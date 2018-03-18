#pragma once

#include "VulkanContext.h"

//Catch all class for non-image buffers



class VulkanBuffer
{
public:
	VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint64 size, void* data = nullptr);
	~VulkanBuffer();
	
	void bindVertex(vk::CommandBuffer &cmd);
	
	void bindIndex(vk::CommandBuffer &cmd);

	void upload(uint64 size, void* data, uint32 offset = 0);

	/*
	* Set size to -1 for full buffer
	*/
	vk::DescriptorBufferInfo getDBI(uint32 offset = 0, int64 size = -1);


	vk::Buffer &getBuffer() {
		return _buffer;
	}

	void *getData();
private:

	uint64 _size;

	VulkanContextRef _ctx;

	vk::DeviceMemory _memory;
	vk::Buffer _buffer;
};

typedef shared_ptr<VulkanBuffer> VulkanBufferRef;