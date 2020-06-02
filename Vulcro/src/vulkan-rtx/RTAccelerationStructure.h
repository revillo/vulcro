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
	RTAccelerationStructure(VulkanContextPtr ctx, uint32_t numInstances, bool allowUpdate = false);
	RTAccelerationStructure(VulkanContextPtr ctx, RTGeometryRef geometry, bool allowUpdate = false);
	
	void allocateDeviceMemory();

	vk::AccelerationStructureNV & getAccelerationStruct()  {
		return _accelStruct;
	}

	vk::AccelerationStructureInfoNV getInfo() {
		return _info;
	}

	void setNeedsRebuild(bool toggle) {
		_needsRebuild = toggle;
	}
    
    //Whether this acceleration structure can be modified / animated
    bool supportsUpdate()
    {
        return _allowUpdate;
    }

	void build(vk::CommandBuffer *cmd, VulkanBufferRef scratchBuffer, VkMemoryBarrier &memoryBarrier, VulkanBufferRef instanceBuffer = nullptr);

	uint64_t getHandle() {
		return _handle;
	}

	~RTAccelerationStructure();

protected:

    bool _allowUpdate;
	std::vector<vk::GeometryNV> _geometries;
    RTGeometryRef mGeoRef;
	vk::AccelerationStructureNV _accelStruct;
	vk::AccelerationStructureInfoNV _info;
	VulkanContextPtr _ctx;
	bool _needsRebuild;
	vk::DeviceMemory _memory;
	uint64_t _memorySize, _handle;
	bool _built = false;
	bool _isTop;

};
typedef shared_ptr<RTAccelerationStructure> RTAccelStructRef;


class RTGeometryRepo
{
public:
    using GeometryId = uint64_t;

    RTGeometryRepo(VulkanContextPtr ctx);

    void flagUpdateGeometry(const GeometryId & id);

    void addGeometry(const GeometryId & id, RTGeometryRef geom, bool allowUpdate = false);

    //Can return nullptr
    RTAccelStructRef getBLAS(const GeometryId & id);
    

    void rebuildDirtyGeometries();
    

    VulkanContextPtr mCtx;
    std::unordered_map<GeometryId, RTAccelStructRef> mBlas;
    std::vector<GeometryId> mDirtyBlas;
    VulkanBufferRef mScratch = nullptr;
    VulkanTaskRef mRebuildTask;
};
typedef shared_ptr<RTGeometryRepo> RTGeometryRepoRef;

class RTScene
{
    using GeometryId = uint64_t;

	struct VkGeometryInstance {
        //Row major affine transform

        VkGeometryInstance()
        {
            transform = { { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 } };
            mask = 0xff;
            instanceShaderBindingTableRecordOffset = 0;
            flags = 0;
            accelerationStructureHandle = 0;
        }

		std::array<float, 12> transform = {{ 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 }};
		
        //Custom instance index mapped to glsl gl_InstanceCustomIndexNV
        uint32_t instanceCustomIndex : 24;

        //Visibilty mask for TraceNV
		uint32_t mask : 8;

        //Index of hit shader group to use when ray hits this instance
		uint32_t instanceShaderBindingTableRecordOffset : 24;

        //see VkGeometryInstanceFlagBitsKHR / VkGeometryInstanceFlagBitsNV
		uint32_t flags : 8;
		
        //The handle to the corresponding geometry BLAS
        uint64_t accelerationStructureHandle;
	};

	struct GeometryData {
        //An array of instances for this geometry
		vector<VkGeometryInstance> instances;

        //The bottom level acceleration structure (BLAS) for this geometry
		RTAccelStructRef accelStruct;

        //The index this BLAS is bound to in TLAS 
		uint32_t bindingIndex;
		
        //The offset among all instances in TLAS of first instance of this geometry
        uint32_t instanceOffset;
	};

public:
    typedef std::string const & StringProxy;

    struct InstanceData
    {
        uint32_t sbtOffset = 0; //hit group index
        uint8_t mask = 0xFF; //Visibility Mask
        vk::GeometryInstanceFlagsNV flags = vk::GeometryInstanceFlagsNV();
        uint32_t customIndexU24 = 0; // User Custom Data (only uses bottom 24 bits)
    };

	RTScene(VulkanContextPtr ctx, RTGeometryRepoRef geoRepo = nullptr);
    //Pass true for allowUpdate to support skeletal meshes / vertex updates
	void addGeometry(const GeometryId& geometryName, RTGeometryRef geometry, bool allowUpdate = false);
    void flagGeometryRebuild(const GeometryId& geometryName);
	void setInstanceCount(const GeometryId& geometryName, uint32_t numInstances);
	uint32_t getGeometryIndex(const GeometryId& geometryName);
	uint32_t getInstanceGlobalIndex(const GeometryId& geometryName, uint32_t instanceIndex);
	uint32_t getInstanceCount(const GeometryId& geometryName);
	void setInstanceTransform(const GeometryId& geometryName, uint32_t instanceIndex, glm::mat4 const & transform);
    
    void setInstanceData(const GeometryId& id, uint32_t instanceIndex, const InstanceData& data);
    
    void addInstance(const GeometryId& geometryName, glm::mat4 const & transform, InstanceData const & data = InstanceData());
	
    std::array<float, 12> getInstanceTransform(const GeometryId & geometryName, uint32_t instanceIndex);

	void build(vk::CommandBuffer * cmd);
    
    void build(VulkanTaskRef task);

	vk::WriteDescriptorSetAccelerationStructureNV getWriteDescriptor();

protected:

	void makeScratchBuffer();

	std::unordered_map<GeometryId, GeometryData> _geometryMap;

	//std::unordered_map<std::string, RTAccelStructRef> _bottomStructMap;
	//std::unordered_map<std::string, vector<VkGeometryInstance>> _instanceMap;

	VulkanContextPtr _ctx;
	RTAccelStructRef _topStruct;
	VulkanBufferRef _scratchBuffer;
	VulkanBufferRef _instanceBuffer = nullptr;
	vector<VkGeometryInstance> _instanceData;
    RTGeometryRepoRef mGeoRepo = nullptr;
};