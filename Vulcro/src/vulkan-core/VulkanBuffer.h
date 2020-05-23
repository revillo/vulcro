#pragma once

#include "VulkanContext.h"
#include "VulkanVertexLayout.h"
//Catch all class for non-image buffers


class VulkanBuffer
{
public:

	typedef uint32_t IndexType;

	//////////////////////////
	//// Constants
	/////////////////////////

	static const vk::BufferUsageFlags UNIFORM_BUFFER;
	static const vk::BufferUsageFlags STORAGE_BUFFER;
	static const vk::BufferUsageFlags TRANSFER_DST_STORAGE_BUFFER;
	static const vk::BufferUsageFlags TRANSFER_SRC_STORAGE_BUFFER;
	static const vk::BufferUsageFlags STORAGE_TEXEL_BUFFER;

	static const vk::MemoryPropertyFlags CPU_ALOT;
	static const vk::MemoryPropertyFlags CPU_SOMETIMES;
	static const vk::MemoryPropertyFlags CPU_NEVER;

	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanBuffer(VulkanContextPtr ctx, vk::BufferUsageFlags usage, uint64_t size, 
		vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eHostVisible, 
		void* data = nullptr);

	VULCRO_DONT_COPY(VulkanBuffer)

	~VulkanBuffer();

	//////////////////////////
	//// Functions
	/////////////////////////
	
	void bindVertex(vk::CommandBuffer * cmd, vk::DeviceSize offset = 0);
	
	void bindIndex(vk::CommandBuffer * cmd, vk::IndexType type = vk::IndexType::eUint32);

	void upload(uint64_t size, void* data, uint64_t offset = 0);

    void copyBuffer(VulkanBufferRef srcBuffer, uint64_t size, uint64_t srcOffset, uint64_t dstOffset);
    void copyBuffer(VulkanTaskRef task, VulkanBufferRef srcBuffer, uint64_t size, uint64_t srcOffset, uint64_t dstOffset);
    

	//////////////////////////
	//// Getters / Setters
	/////////////////////////
	/*
	* Set size to -1 for full buffer
	*/
	vk::DescriptorBufferInfo getDBI(vk::DeviceSize offset = 0, vk::DeviceSize size = 0);

	vk::DescriptorType getDescriptorType();

    /*
	inline VulkanSetLayoutBinding getBinding()
	{
		return SLB(1, getDescriptorType());
	}*/

	inline void recordClear(vk::CommandBuffer * cmd)
	{
		cmd->fillBuffer(mBuffer, 0, _size, 0);
	}

	inline vk::Buffer &getBuffer()
	{
		return mBuffer;
	}

	void * getMapped(uint64_t offset = 0, uint64_t size = VK_WHOLE_SIZE);

    void flush(uint64_t offset = 0, uint64_t size = VK_WHOLE_SIZE);

	void unmap();

	void createView(vk::Format format);

	vk::BufferView getView() {
		return _view;
	}

	vk::DeviceSize getSize() {
		return _size;
	}

private:
	vk::DeviceSize _size;

	VulkanContextPtr _ctx;
	vk::BufferUsageFlags _usage;
    vk::MemoryPropertyFlags mMemoryFlags;
	vk::DeviceMemory mMemory;
	vk::Buffer mBuffer;
	vk::BufferView _view = nullptr;

    void * _mappedData = nullptr;
};


class VulkanBufferWindow {

	VulkanBufferWindow(VulkanBufferRef buffer, vk::DeviceSize offset, vk::DeviceSize size);

	VulkanBufferRef getBuffer() {
		return _buffer;
	}

	const vk::DeviceSize getOffset() {
		return _offset;
	}
	
	const vk::DeviceSize getSize() {
		return _size;
	}


protected:

	VulkanBufferRef _buffer;
	vk::DeviceSize _offset, _size;
};

class iubo {
public:
	virtual vk::DescriptorBufferInfo getDBI() = 0;
	virtual ~iubo() {};
};


typedef shared_ptr<iubo> iuboRef;



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

		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eUniformBuffer, _size, VulkanBuffer::CPU_SOMETIMES, data);
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

	void set(uint32_t start, uint32_t count, T * newValues) {
		memcpy(values, newValues, count * sizeof(T));
	}

	void sync() {
		_vbr->upload(_size, values);
	};

	void sync(uint32_t index, uint32_t count) {
		_vbr->upload(count * sizeof(T), values[index], index * sizeof(T));
	}

	SLB getLayout() {
		return SLB(_arrayCount, vk::DescriptorType::eUniformBuffer);
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
	virtual vk::DeviceSize getOffset() = 0;
	virtual VulkanBufferRef getSharedBuffer() = 0;
};

typedef shared_ptr<ivbo> vboRef;

template <class T>
class static_vbo : public ivbo {
	
public: 

	static_vbo(VulkanContext * ctx, vk::ArrayProxy<const vk::Format> fieldFormats, uint32_t numVerts, void * data) {
		_typeCount = numVerts;
		_layout = ctx->makeVertexLayout(fieldFormats);
		_size = sizeof(T) * numVerts;
		_vbr = ctx->makeDeviceBuffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, _size, data);
		_offset = 0;
	}

	static_vbo(vboRef vBuf, uint32_t vertOffset, uint32_t numVerts) {
		_offset = vertOffset * sizeof(T) + vBuf->getOffset();
		_size = numVerts * sizeof(T);
		_typeCount = numVerts;
		_vbr = vBuf->getSharedBuffer();
		_layout = vBuf->getLayout();
	}

	void bind(vk::CommandBuffer * cmd) override {
		_vbr->bindVertex(cmd, _offset);
	}

	VulkanVertexLayoutRef getLayout() override {
		return _layout;
	}

	uint32_t getCount() override {
		return _typeCount;
	}

	vk::DeviceSize getOffset() override {
		return _offset;
	}

	vk::Buffer getBuffer() override {
		return _vbr->getBuffer();
	}

	VulkanBufferRef getSharedBuffer() override {
		return _vbr;
	}

	VulkanBufferRef _vbr;
	VulkanVertexLayoutRef _layout;
	vk::DeviceSize _size, _offset;
	uint32_t _typeCount;

};

template <class T>
class dynamic_vbo : public ivbo {
public:

	dynamic_vbo(VulkanContext * ctx, vk::ArrayProxy<const vk::Format> fieldFormats, uint32_t arrayCount = 1, void * data = nullptr)
		: _arrayCount(arrayCount)
	{
		//values = new T[arrayCount];
		_size = sizeof(T) * arrayCount;

		
		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eVertexBuffer, _size, VulkanBuffer::CPU_ALOT, data);
        values = (T*)_vbr->getMapped();
		_layout = ctx->makeVertexLayout(fieldFormats);
	};

	VULCRO_DONT_COPY(dynamic_vbo)

	T & at(uint32_t i = 0) {
		return values[i];
	}

	void set(uint32_t i, T const & value) {
		values[i] = value;
	}

	void set(T const & value) {
		values[0] = value;
	}

	void set(uint32_t start, uint32_t count, T * newValues) {
		memcpy(&values[0], newValues, count * sizeof(T));
	}

	void bind(vk::CommandBuffer * cmd) override {
		_vbr->bindVertex(cmd);
	}

	VulkanVertexLayoutRef getLayout() override {
		return _layout;
	}

	uint32_t getCount() override {
		return _arrayCount;
	}

	vk::Buffer getBuffer() override {
		return _vbr->getBuffer();
	}

	vk::DeviceSize getOffset() {
		return 0;
	}

	VulkanBufferRef getSharedBuffer() {
		return _vbr;
	}

	~dynamic_vbo() {
		//delete values;
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
	ibo(VulkanContextPtr ctx, vk::ArrayProxy<const VulkanBuffer::IndexType> indices) {

		_vbr = ctx->makeDeviceBuffer(
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
			sizeof(VulkanBuffer::IndexType) * indices.size(),
			(void*)indices.begin()
		);

        _count = static_cast<uint32_t>(indices.size());
		_offset = 0;
        
	}

	ibo(shared_ptr<ibo> iBuf, uint32_t indexOffset, uint32_t numIndices) {
		_count = numIndices;
		_vbr = iBuf->getSharedBuffer();
		_offset = indexOffset * sizeof(VulkanBuffer::IndexType) + iBuf->getOffset();
	}

	VULCRO_DONT_COPY(ibo)

	void bind(vk::CommandBuffer * cmd) {
		_vbr->bindIndex(cmd);
	}

    uint32_t getCount() {
        return _count;
    }

	vk::DeviceSize getOffset() {
		return _offset;
	}

	vk::Buffer getBuffer() {
		return _vbr->getBuffer();
	}

	VulkanBufferRef getSharedBuffer() {
		return _vbr;
	}

	vk::IndexType getType() {
		return vk::IndexType::eUint16;
	}

private:

    uint32_t _count = 0;
	VulkanBufferRef _vbr;
	vk::DeviceSize _offset;

};

typedef shared_ptr<ibo> iboRef;

class issbo {
public:

	virtual VulkanBufferRef getBuffer() = 0;
	virtual ~issbo() {};
};

typedef shared_ptr<issbo> ssboRef;

template<class T>
class dynamic_ssbo : public issbo {
public:

    dynamic_ssbo(VulkanContextPtr ctx, uint32_t arrayCount) :
		_arrayCount(arrayCount) 
	{
		_vbr = ctx->makeBuffer(vk::BufferUsageFlagBits::eStorageBuffer, sizeof(T) * arrayCount, VulkanBuffer::CPU_ALOT, nullptr);
        _values = _vbr->getMapped();
	}

	VULCRO_DONT_COPY(dynamic_ssbo)


    /*
	T* getMapped() {
		return static_cast<T*>(_vbr->getMapped());
	}
	
	void unmap() {
		_vbr->unmap();
	}*/

    void set(uint32_t i, T const & value)
    {
        values[i] = value;
    }

    T & at(uint32_t)
    {
        return values[i];
    }

	VulkanBufferRef getBuffer() override {
		return _vbr;
	}

	~dynamic_ssbo() {
		if (_heapBuffer != nullptr) {
			delete _heapBuffer;
		}
	}

protected:

	T* _values = nullptr;
	uint32_t _arrayCount;
	VulkanBufferRef _vbr;
};
