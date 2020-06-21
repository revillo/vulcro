#pragma once

#include "../vulkan-core/VulkanContext.h"
#include "../vulkan-core/VulkanBuffer.h"
#include "../vulkan-core/VulkanTask.h"

class RTGeometry {

public:
	RTGeometry(iboRef indexBuffer, vboRef vertexBuffer);

    RTGeometry(VulkanBufferRef vertexBuffer, uint64_t vertexCount, uint32_t vertexStride, uint64_t vertexOffset = 0, vk::Format positionFormat = vk::Format::eR32G32B32A32Sfloat);

    RTGeometry(uint64_t aabbCount, uint64_t aabbOffset, VulkanBufferRef aabbBuffer);

	vk::GeometryNV getGeometry() const {
		return _geometry;
	}

	iboRef getIndexBuffer() {
		return _indexBuffer;
	}

	vboRef getVertexBuffer() {
		return _vertexBuffer;
	}

protected:
	iboRef _indexBuffer;
	vboRef _vertexBuffer;

    VulkanBufferRef mFaceVertexBuffer = nullptr;
    VulkanBufferRef mAABBBuffer = nullptr;
   
	vk::GeometryNV _geometry;
};

class RTAccelerationStructure {


public:

    VULCRO_DONT_COPY(RTAccelerationStructure)

	RTAccelerationStructure(VulkanContextPtr ctx, uint32_t numInstances, bool allowUpdate = false);
	RTAccelerationStructure(VulkanContextPtr ctx, std::vector<RTGeometryRef> && geometries, bool allowUpdate = false);
	
	void allocateDeviceMemory();

	vk::AccelerationStructureNV & getAccelerationStruct()  {
		return _accelStruct;
	}

	vk::AccelerationStructureInfoNV getInfo() {
		return _info;
	}
    
    //Whether this acceleration structure can be modified / animated
    bool supportsUpdate()
    {
        return _allowUpdate;
    }

	void build(vk::CommandBuffer *cmd, VulkanBufferRef scratchBuffer, VulkanBufferRef instanceBuffer = nullptr);

    void dropGeometryRefs()
    {
        mGeoRefs.clear();
    }

    uint64_t computeScratchMemorySize();

	uint64_t getHandle() {
		return _handle;
	}

	~RTAccelerationStructure();

protected:

    bool _allowUpdate;
	std::vector<vk::GeometryNV> _geometries;
    std::vector<RTGeometryRef> mGeoRefs;
	vk::AccelerationStructureNV _accelStruct;
	vk::AccelerationStructureInfoNV _info;
	VulkanContextPtr _ctx;
	vk::DeviceMemory _memory;
	uint64_t _memorySize, _handle;
	bool _built = false;
	bool _isTop;

};

class RTBottomStructure : public RTAccelerationStructure
{
public:

    VULCRO_DONT_COPY(RTBottomStructure)

    RTBottomStructure(VulkanContextPtr ctx, std::vector<RTGeometryRef> && geometries, bool allowUpdate = false)
        :RTAccelerationStructure(ctx, std::move(geometries), allowUpdate)
    {}
};


class RTTopStructure : public RTAccelerationStructure
{
public:

    VULCRO_DONT_COPY(RTTopStructure)

    RTTopStructure(VulkanContextPtr ctx, uint32_t numInstances, bool allowUpdate = true)
        :RTAccelerationStructure(ctx, numInstances, allowUpdate)
    {}

    vk::WriteDescriptorSetAccelerationStructureNV getWriteDescriptor();
};

class RTTopStructureManager
{
public:
    
    RTTopStructureManager(VulkanContextPtr ctx, uint32_t numInstances);

    RTTopStructureRef getTopStructure()
    {
        return mTopStructure;
    }

    uint32_t getNumInstances()
    {
        return mNumInstances;
    }

    void resize(uint32_t numInstances);

    VkGeometryInstance & getInstance(uint32_t index);

    void rebuild(VulkanTaskRef task = nullptr);

private:
    
    VulkanContextPtr mCtx;
    VulkanTaskRef mTask;
    uint32_t mNumInstances;
    RTTopStructureRef mTopStructure;
    VulkanBufferRef mScratchBuffer;
    VulkanBufferRef mInstanceBuffer;
    VkGeometryInstance * mInstancePtr;
};


class RTBlasRepo
{
public:
    using GeometryId = uint64_t;

    RTBlasRepo(VulkanContextPtr ctx);

    void flagUpdateGeometry(GeometryId id);

    void addGeometry(GeometryId id, RTGeometryRef geom, bool allowUpdate = false);

    void removeGeometry(GeometryId id);

    //Can return nullptr
    RTBottomStructureRef getBLAS(GeometryId id);

    void rebuildDirtyGeometries(VulkanTaskRef optionalTask = nullptr);
    
protected:

    VulkanContextPtr mCtx;
    std::unordered_map<GeometryId, RTBottomStructureRef> mBlas;
    std::vector<GeometryId> mDirtyBlas;
    VulkanBufferRef mScratch = nullptr;
    VulkanTaskRef mRebuildTask;
};
typedef shared_ptr<RTBlasRepo> RTBlasRepoRef;
