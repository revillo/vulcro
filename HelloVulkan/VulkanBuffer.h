#pragma once

#include "VulkanContext.h"

//Catch all class for non-image buffers



class VulkanBuffer
{
public:
	VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint32 size, void* data);
	~VulkanBuffer();
	void bindVertex(vk::CommandBuffer &cmd);
	void bindIndex(vk::CommandBuffer &cmd);
	vk::Buffer &getBuffer() {
		return _buffer;
	}

	void *getData();
private:

	VulkanContextRef _ctx;

	vk::DeviceMemory _memory;
	vk::Buffer _buffer;
};

typedef shared_ptr<VulkanBuffer> VulkanBufferRef;