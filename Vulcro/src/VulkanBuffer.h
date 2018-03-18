#pragma once

#include "VulkanContext.h"

//Catch all class for non-image buffers

class VulkanBuffer
{
public:
	VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint64 size, void* data = nullptr);
	~VulkanBuffer();
	
	void bindVertex(vk::CommandBuffer * cmd);
	
	void bindIndex(vk::CommandBuffer * cmd);

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


template <class T>
class ubo {
public:

	ubo(VulkanContext * ctx, uint32 arrayCount = 1) : _arrayCount(arrayCount)
	{
		values = new T[arrayCount];
		_size = sizeof(T) * arrayCount;
		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eUniformBuffer, _size, values);
	};

	T & at(uint32 i = 0) {
		return values[i];
	}

	void sync() {
		_vbr->upload(_size, values);
	};

	void sync(uint32 index, uint32 count) {
		_vbr->upload(count * sizeof(T), values[index], index * sizeof(T));
	}

	ULB getLayout() {
		return ULB(_arrayCount, vk::DescriptorType::eUniformBuffer);
	}

	vk::DescriptorBufferInfo getDBI() {
		return _vbr->getDBI(0, -1);
	}
	
	vk::DescriptorBufferInfo getDBI(uint32 offset, int64 size) {
		return _vbr->getDBI(offset, size);
	}

	~ubo() {
		delete values;
	}

private:

	T * values;

	vk::BufferUsageFlags _usage;
	uint32 _arrayCount;
	uint32 _size;
	VulkanBufferRef _vbr;
};

template <class T>
class vbo {
public:

	vbo(VulkanContext * ctx, vector<vk::Format> fieldFormats, uint32 arrayCount = 1) 
		: _arrayCount(arrayCount)
	{
		values = new T[arrayCount];
		_size = sizeof(T) * arrayCount;
		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eVertexBuffer, _size, values);
		_layout = ctx->makeVertexLayout(fieldFormats);
	};

	T & at(uint32 i = 0) {
		return values[i];
	}

	void sync() {
		_vbr->upload(_size, values);
	};

	void sync(uint32 index, uint32 count) {
		_vbr->upload(count * sizeof(T), values[index], index * sizeof(T));
	}

	void bind(vk::CommandBuffer * cmd) {
		_vbr->bindVertex(cmd);
	}

	VulkanVertexLayoutRef getLayout() {
		return _layout;
	}

	~vbo() {
		delete values;
	}

private:

	T * values;
	VulkanVertexLayoutRef _layout;
	uint32 _arrayCount;
	uint32 _size;
	VulkanBufferRef _vbr;
	vector<vk::Format> fieldFormats;
};

class ibo {

public:
	ibo(VulkanContextRef ctx, vector<uint16_t> indices) {

		_vbr = ctx->makeBuffer(
			vk::BufferUsageFlagBits::eIndexBuffer,
			sizeof(uint16_t) * indices.size(),
			&indices[0]
		);
	}

	void bind(vk::CommandBuffer * cmd) {
		_vbr->bindIndex(cmd);
	}

private:

	VulkanBufferRef _vbr;

};