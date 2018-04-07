#pragma once

#include "VulkanContext.h"

//Catch all class for non-image buffers

class VulkanBuffer
{
public:

	

	static const vk::MemoryPropertyFlags CPU_ALOT;
	static const vk::MemoryPropertyFlags CPU_NEVER;

	VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint64 size, 
		vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
		void* data = nullptr);

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

	ubo(VulkanContext * ctx, uint32 arrayCount = 1, T * data = nullptr) : _arrayCount(arrayCount)
	{
		values = new T[arrayCount];

		_size = sizeof(T) * arrayCount;

		if (data != nullptr) {
			memcpy(values, data, _size);
		}

		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eUniformBuffer, _size, VulkanBuffer::CPU_ALOT, data);
	};

	T & at(uint32 i = 0) {
		return values[i];
	}

	void set(uint32 i, T value) {
		values[i] = value;
	}

	void set(T value) {
		values[0] = value;
	}

	void set(uint32 start, uint32 count, T * values) {
		memcpy(&values[i], values, count * sizeof(T));
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

	vbo(VulkanContext * ctx, vector<vk::Format> &&fieldFormats, uint32 arrayCount = 1, void * data = nullptr) 
		: _arrayCount(arrayCount)
	{
		values = new T[arrayCount];
		_size = sizeof(T) * arrayCount;

		if (data != nullptr) {
			memcpy(values, data, _size);
		}

		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eVertexBuffer, _size, VulkanBuffer::CPU_ALOT, data);
		_layout = ctx->makeVertexLayout(fieldFormats);
	};

	T & at(uint32 i = 0) {
		return values[i];
	}

	void set(uint32 i, T value) {
		values[i] = value;
	}

	void set(T value) {
		values[0] = value;
	}

	void set(uint32 start, uint32 count, T * newValues) {
		memcpy(&values[0], newValues, count * sizeof(T));
	}

	void sync() {
		_vbr->upload(_size, values);
	}

	void sync(uint32 index, uint32 count) {
		_vbr->upload(count * sizeof(T), values[index], index * sizeof(T));
	}

	void bind(vk::CommandBuffer * cmd) {
		_vbr->bindVertex(cmd);
	}

	VulkanVertexLayoutRef getLayout() {
		return _layout;
	}

	uint32 getCount() {
		return _arrayCount;
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
			VulkanBuffer::CPU_ALOT,
			&indices[0]
		);

        _count = static_cast<uint32>(indices.size());
	}

	void bind(vk::CommandBuffer * cmd) {
		_vbr->bindIndex(cmd);
	}

    uint32 getCount() {
        return _count;
    }

private:

    uint32 _count = 0;
	VulkanBufferRef _vbr;

};

template<class T>
class ssbo {

	ssbo(VulkanContextRef ctx) {
		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eStorageBuffer, sizeof(T), VulkanBuffer::CPU_NEVER, nullptr);
	}


	VulkanBufferRef _vbr;

};