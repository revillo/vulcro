#pragma once

#include "VulkanContext.h"

//Catch all class for non-image buffers



class VulkanBuffer
{
public:
	VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint64 size, void* data);
	~VulkanBuffer();
	void bind(vk::CommandBuffer &cmd);

private:

	vk::DeviceMemory _memory;
	VulkanContextRef _ctx;
	vk::Buffer _buffer;
};

typedef shared_ptr<VulkanBuffer> VulkanBufferRef;