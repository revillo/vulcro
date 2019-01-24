#pragma once

#include "VulkanContext.h"
#include "VulkanBuffer.h"


class RTGeometry {
public:
	RTGeometry(iboRef ibo, vboRef vbo);
	vk::GeometryNV getGeometry() const {
		return _geometry;
	}
protected:

	vk::GeometryNV _geometry;

};

class RTAccelerationStructure {


public:
	RTAccelerationStructure(VulkanContextRef ctx, uint32_t numInstances);
	RTAccelerationStructure(VulkanContextRef ctx, RTGeometry geometries);
	
	void allocateDeviceMemory();

	vk::AccelerationStructureNV & getAccelerationStruct()  {
		return _accelStruct;
	}

	void setNumInstances(uint32_t numInstances) {
		_info.instanceCount = numInstances;
	}

	vk::AccelerationStructureInfoNV getInfo() {
		return _info;
	}

	uint64_t getHandle() {
		return _handle;
	}

	~RTAccelerationStructure();

protected:

	std::vector<vk::GeometryNV> _geometries;

	vk::AccelerationStructureNV _accelStruct;
	vk::AccelerationStructureInfoNV _info;
	VulkanContextRef _ctx;

	vk::DeviceMemory _memory;
	uint64_t _memorySize, _handle;

	bool _isTop;

};
typedef shared_ptr<RTAccelerationStructure> RTAccelStructRef;

class RTScene {

	struct VkGeometryInstance {
		float transform[12] = { 1.0, 0.0, 0.0, 0.0,
							   0.0, 1.0, 0.0, 0.0,
							   0.0, 0.0, 1.0, 0.0 };

		uint32_t instanceId : 24;
		uint32_t mask : 8;
		uint32_t instanceOffset : 24;
		uint32_t flags : 8;
		uint64_t accelerationStructureHandle;
	};

public:

	RTScene(VulkanContextRef ctx);

	void addGeometry(RTGeometry geom);
	void build(vk::CommandBuffer * cmd);
	vk::WriteDescriptorSetAccelerationStructureNV getWriteDescriptor();

protected:

	void makeScratchBuffer();

	VulkanContextRef _ctx;
	RTAccelStructRef _topStruct;
	std::vector<RTAccelStructRef> _bottomStructs;
	VulkanBufferRef _scratchBuffer;
	VulkanBufferRef _instanceBuffer;
	vector< VkGeometryInstance> _instanceData;
	bool _built = false;
};