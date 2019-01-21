#pragma once

#include "VulkanContext.h"
#include "VulkanVertexLayout.h"
//Catch all class for non-image buffers


class VulkanBuffer
{
public:

	typedef uint32_t IndexType;

	static const vk::BufferUsageFlags UNIFORM_BUFFER;
	static const vk::BufferUsageFlags STORAGE_BUFFER;
	static const vk::BufferUsageFlags CLEARABLE_STORAGE_BUFFER;
	static const vk::BufferUsageFlags STORAGE_TEXEL_BUFFER;

	static const vk::MemoryPropertyFlags CPU_ALOT;
	static const vk::MemoryPropertyFlags CPU_NEVER;


	VulkanBuffer(VulkanContextRef ctx, vk::BufferUsageFlags usage, uint64_t size, 
		vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
		void* data = nullptr);

	VULCRO_DONT_COPY(VulkanBuffer)

	~VulkanBuffer();
	
	void bindVertex(vk::CommandBuffer * cmd);
	
	void bindIndex(vk::CommandBuffer * cmd, vk::IndexType type = vk::IndexType::eUint32);

	void upload(uint64_t size, void* data, uint32_t offset = 0);

	/*
	* Set size to -1 for full buffer
	*/
	vk::DescriptorBufferInfo getDBI(uint32_t offset = 0, int64_t size = -1);

	vk::DescriptorType getDescriptorType() {
		if (_usage & vk::BufferUsageFlagBits::eUniformBuffer) {
			return vk::DescriptorType::eUniformBuffer;
		}
		else if (_usage & vk::BufferUsageFlagBits::eStorageBuffer) {
			return vk::DescriptorType::eStorageBuffer;
		}
		else if (_usage & vk::BufferUsageFlagBits::eStorageTexelBuffer) {
			return vk::DescriptorType::eStorageTexelBuffer;
		} else { 
			throw "Unsupported buffer as descriptor";
		}
	};


	VulkanUniformLayoutBinding getBinding() {
		return ULB(1, getDescriptorType());
	}

	void recordClear(vk::CommandBuffer * cmd) {
		cmd->fillBuffer(_buffer, 0, _size, 0);
	}

	vk::Buffer &getBuffer() {
		return _buffer;
	}

	void * getMapped(uint32_t offset = 0, int64_t size = -1);

	void unmap();

	void createView(vk::Format format) {
		
		_view = _ctx->getDevice().createBufferView(vk::BufferViewCreateInfo(
			vk::BufferViewCreateFlags(),
			_buffer,
			format,
			0,
			VK_WHOLE_SIZE));
	}

	vk::BufferView getView() {
		return _view;
	}


private:

	uint64_t _size;

	VulkanContextRef _ctx;
	vk::BufferUsageFlags _usage;

	vk::DeviceMemory _memory;
	vk::Buffer _buffer;
	vk::BufferView _view = nullptr;

};


class iubo {
public:
	virtual vk::DescriptorBufferInfo getDBI() = 0;
	virtual ~iubo() {};

};


typedef shared_ptr<iubo> uboRef;

template <class T>
class ubo : public iubo {
public:

	ubo(VulkanContext * ctx, uint32_t arrayCount = 1, T * data = nullptr) : _arrayCount(arrayCount)
	{
		values = new T[arrayCount];

		_size = sizeof(T) * arrayCount;

		if (data != nullptr) {
			memcpy(values, data, _size);
		}

		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eUniformBuffer, _size, VulkanBuffer::CPU_ALOT, data);
	};

	VULCRO_DONT_COPY(ubo)

	T & at(uint32_t i = 0) {
		return values[i];
	}

	void set(uint32_t i, T value) {
		values[i] = value;
	}

	void set(T value) {
		values[0] = value;
	}

	void set(uint32_t start, uint32_t count, T * values) {
		memcpy(&values[i], values, count * sizeof(T));
	}

	void sync() {
		_vbr->upload(_size, values);
	};

	void sync(uint32_t index, uint32_t count) {
		_vbr->upload(count * sizeof(T), values[index], index * sizeof(T));
	}

	ULB getLayout() {
		return ULB(_arrayCount, vk::DescriptorType::eUniformBuffer);
	}

	vk::DescriptorBufferInfo getDBI() override {
		return _vbr->getDBI(0, -1);
	}
	
	vk::DescriptorBufferInfo getDBI(uint32_t offset, int64_t size) {
		return _vbr->getDBI(offset, size);
	}

	~ubo() {
		delete values;
	}

private:

	T * values;

	vk::BufferUsageFlags _usage;
	uint32_t _arrayCount;
	uint32_t _size;
	VulkanBufferRef _vbr;
};

class ivbo {
	
public:
	virtual ~ivbo() {};

	virtual void bind(vk::CommandBuffer * cmd) = 0;
	virtual VulkanVertexLayoutRef getLayout() = 0;
	virtual uint32_t getCount() = 0;
	virtual vk::Buffer getBuffer() = 0;
};

typedef shared_ptr<ivbo> vboRef;

template <class T>
class static_vbo : public ivbo {
	
public: 

	static_vbo(VulkanContext * ctx, temps<vk::Format> fieldFormats, uint32_t arrayCount, void * data) {
		_arrayCount = arrayCount;
		_layout = ctx->makeVertexLayout(move(fieldFormats));
		_size = sizeof(T) * arrayCount;
		_vbr = ctx->makeFastBuffer(vk::BufferUsageFlagBits::eVertexBuffer, _size, data);
	}

	void bind(vk::CommandBuffer * cmd) override {
		_vbr->bindVertex(cmd);
	}

	VulkanVertexLayoutRef getLayout() override {
		return _layout;
	}

	uint32 getCount() {
		return _arrayCount;
	}

	vk::Buffer getBuffer() override {
		return _vbr->getBuffer();
	}

	VulkanBufferRef _vbr;
	VulkanVertexLayoutRef _layout;
	uint64 _size;
	uint32 _arrayCount;

};

template <class T>
class dynamic_vbo : public ivbo {
public:

	dynamic_vbo(VulkanContext * ctx, temps<vk::Format> fieldFormats, uint32_t arrayCount = 1, void * data = nullptr)
		: _arrayCount(arrayCount)
	{
		values = new T[arrayCount];
		_size = sizeof(T) * arrayCount;

		if (data != nullptr) {
			memcpy(values, data, _size);
		}

		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eVertexBuffer, _size, VulkanBuffer::CPU_ALOT, data);
		_layout = ctx->makeVertexLayout(move(fieldFormats));
	};

	VULCRO_DONT_COPY(dynamic_vbo)

	T & at(uint32_t i = 0) {
		return values[i];
	}

	void set(uint32_t i, T value) {
		values[i] = value;
	}

	void set(T value) {
		values[0] = value;
	}

	void set(uint32_t start, uint32_t count, T * newValues) {
		memcpy(&values[0], newValues, count * sizeof(T));
	}

	void sync() {
		_vbr->upload(_size, values);
	}

	void sync(uint32_t index, uint32_t count) {
		_vbr->upload(count * sizeof(T), values[index], index * sizeof(T));
	}

	void bind(vk::CommandBuffer * cmd) override {
		_vbr->bindVertex(cmd);
	}

	VulkanVertexLayoutRef getLayout() override {
		return _layout;
	}

	uint32_t getCount() {
		return _arrayCount;
	}

	vk::Buffer getBuffer() override {
		return _vbr->getBuffer();
	}

	~dynamic_vbo() {
		delete values;
	}

private:

	T * values;
	VulkanVertexLayoutRef _layout;
	uint32_t _arrayCount;
	uint32_t _size;
	VulkanBufferRef _vbr;
	vector<vk::Format> fieldFormats;
};

class ibo {

public:
	ibo(VulkanContextRef ctx, vk::ArrayProxy<const VulkanBuffer::IndexType> indices) {

		_vbr = ctx->makeFastBuffer(
			vk::BufferUsageFlagBits::eIndexBuffer,
			sizeof(VulkanBuffer::IndexType) * indices.size(),
			(void*)indices.begin()
		);

        _count = static_cast<uint32>(indices.size());
        
	}

	VULCRO_DONT_COPY(ibo)

	void bind(vk::CommandBuffer * cmd) {
		_vbr->bindIndex(cmd);
	}

    uint32_t getCount() {
        return _count;
    }

	vk::Buffer getBuffer() {
		return _vbr->getBuffer();
	}

	vk::IndexType getType() {
		return vk::IndexType::eUint16;
	}

private:

    uint32_t _count = 0;
	VulkanBufferRef _vbr;

};

typedef shared_ptr<ibo> iboRef;

class issbo {
public:

	virtual VulkanBufferRef getBuffer() = 0;
	virtual ~issbo() {};
};

typedef shared_ptr<issbo> ssboRef;

template<class T>
class ssbo : public issbo {
public:

	ssbo(VulkanContextRef ctx, uint32_t arrayCount) :
		_arrayCount(arrayCount) 
	{
		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eStorageBuffer, sizeof(T) * arrayCount, VulkanBuffer::CPU_ALOT, nullptr);
	}

	VULCRO_DONT_COPY(ssbo)


	T* loadMapped() {
		return static_cast<T*>(_vbr->getMapped());
	}
	
	T* loadHeap() {
		if (_heapBuffer == nullptr) {
			_heapBuffer = new T[_arrayCount];
		}

		memcpy(_heapBuffer, loadMapped(), sizeof(T) * _arrayCount);

		unmap();

		return _heapBuffer;
	}

	void unmap() {
		_vbr->unmap();
	}

	VulkanBufferRef getBuffer() override {
		return _vbr;
	};

	~ssbo() {
		if (_heapBuffer != nullptr) {
			delete _heapBuffer;
		}
	}

protected:

	T* _heapBuffer = nullptr;
	uint32_t _arrayCount;
	VulkanBufferRef _vbr;

};
