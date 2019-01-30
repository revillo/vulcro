#include "RTAccelerationStructure.h"

RTAccelerationStructure::RTAccelerationStructure(VulkanContextRef ctx, uint32_t numInstances)
	:_ctx(ctx),
	_isTop(true)
{

	vk::BuildAccelerationStructureFlagsNV flags = vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace;// | vk::BuildAccelerationStructureFlagBitsNV::eAllowUpdate;

	_info = vk::AccelerationStructureInfoNV(
		vk::AccelerationStructureTypeNV::eTopLevel,
		flags, //flags
		numInstances,  //InstanceCount -- does this even matter? Just overwritten later...
		0,  //GeometryCount
		nullptr //Geometry
	);

	auto ci = vk::AccelerationStructureCreateInfoNV(
		0, //Compact Size
		_info
	); 
	
	_accelStruct = ctx->getDevice().createAccelerationStructureNV(ci, nullptr, _ctx->getDynamicDispatch());
	allocateDeviceMemory();
}


RTAccelerationStructure::RTAccelerationStructure(VulkanContextRef ctx, RTGeometry geometry)
	:_ctx(ctx),
	_isTop(false)
{

	vk::BuildAccelerationStructureFlagsNV flags = vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace;// | vk::BuildAccelerationStructureFlagBitsNV::eAllowUpdate;

	/*
	for (auto &g : geometries) {
		geos.push_back(g.getGeometry());
	}*/
	_geometries.push_back(geometry.getGeometry());

	_info = vk::AccelerationStructureInfoNV(
		vk::AccelerationStructureTypeNV::eBottomLevel,
		flags, //flags
		0,  //InstanceCount
		static_cast<uint32_t>(_geometries.size()),  //GeometryCount
		_geometries.data() //Geometry
	);

	_accelStruct = ctx->getDevice().createAccelerationStructureNV(
		vk::AccelerationStructureCreateInfoNV(
			0, //Compact Size
			_info
		), nullptr,
		ctx->getDynamicDispatch()
	);

	allocateDeviceMemory();
}

RTAccelerationStructure::~RTAccelerationStructure()
{
	_ctx->getDevice().destroyAccelerationStructureNV(_accelStruct, nullptr, _ctx->getDynamicDispatch());
	_ctx->getDevice().freeMemory(_memory, nullptr, _ctx->getDynamicDispatch());
}

void RTAccelerationStructure::allocateDeviceMemory()
{

	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();

	auto memReqs = _ctx->getDevice().getAccelerationStructureMemoryRequirementsNV(
		vk::AccelerationStructureMemoryRequirementsInfoNV(
			vk::AccelerationStructureMemoryRequirementsTypeNV::eObject,
			_accelStruct
		), _ctx->getDynamicDispatch()
	);
	
	auto req = memReqs.memoryRequirements;

	uint32_t memTypeIndex = 1000;
	auto reqBits = req.memoryTypeBits;

	auto p = vk::MemoryPropertyFlagBits::eDeviceLocal;

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		auto type = memProps.memoryTypes[i];
		if ((reqBits & (1 << i)) && ((type.propertyFlags & p) == p)) {

			memTypeIndex = i;
			break;
		}
	}

	_memorySize = req.size;

	_memory = _ctx->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(req.size, memTypeIndex)
	);

	_ctx->getDevice().bindAccelerationStructureMemoryNV({

		vk::BindAccelerationStructureMemoryInfoNV(
		_accelStruct,
		_memory,
		0)}

		, _ctx->getDynamicDispatch());

	_ctx->getDevice().getAccelerationStructureHandleNV(
		_accelStruct,
		sizeof(uint64_t),
		&_handle,
		_ctx->getDynamicDispatch()
	);


}

RTGeometry::RTGeometry(iboRef ibuf, vboRef vbuf)
	:_geometry(),
	_indexBuffer(ibuf),
	_vertexBuffer(vbuf)
{
	
	vk::GeometryTrianglesNV triangles;
	triangles.setIndexCount(ibuf->getCount());
	triangles.setIndexData(ibuf->getBuffer());
	triangles.setIndexOffset(ibuf->getOffset()); //todo
	triangles.setIndexType(vk::IndexType::eUint32);
	
	triangles.setVertexCount(vbuf->getCount());
	triangles.setVertexData(vbuf->getBuffer());
	triangles.setVertexFormat(vbuf->getLayout()->getFields()[0]);
	triangles.setVertexOffset(vbuf->getOffset());
	triangles.setVertexStride(vbuf->getLayout()->getSize());
	
	triangles.setTransformData(nullptr);
	triangles.setTransformOffset(0); 
	

	vk::GeometryDataNV geometry;
	
	geometry.setTriangles(triangles);

	vk::GeometryAABBNV aabb;
	geometry.setAabbs(aabb);

	_geometry.setGeometryType(vk::GeometryTypeNV::eTriangles);
	
	_geometry.setGeometry(geometry);
	_geometry.setFlags(vk::GeometryFlagBitsNV::eOpaque);

}

RTScene::RTScene(VulkanContextRef ctx)
	:_ctx(ctx)
{
	_topStruct = std::make_shared<RTAccelerationStructure>(ctx, 1);
}

void RTScene::addGeometry(RTGeometry geom) {

	auto as = std::make_shared<RTAccelerationStructure>(_ctx, geom);
	_bottomStructs.push_back(
		as
	);
}




void RTScene::build(vk::CommandBuffer * cmd)
{

	if (!_built) {
		makeScratchBuffer();
	}

	VkMemoryBarrier memoryBarrier;
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.pNext = nullptr;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

	
	for (auto &bs : _bottomStructs) {

		cmd->buildAccelerationStructureNV(
			bs->getInfo(),
			nullptr,
			vk::DeviceSize(0),
			_built,
			bs->getAccelerationStruct(),
			_built ? bs->getAccelerationStruct() : nullptr,
			_scratchBuffer->getBuffer(), 
			vk::DeviceSize(0),
			_ctx->getDynamicDispatch()
		);
		vkCmdPipelineBarrier(*cmd, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

	}

	int id = 0;
	for (auto &bs : _bottomStructs) {
		VkGeometryInstance instanceData;
		instanceData.instanceId = id++;
		instanceData.instanceOffset = 0;
		instanceData.mask = 0xff;
		instanceData.flags = (uint32_t)vk::GeometryInstanceFlagBitsNV::eTriangleFrontCounterclockwise;
		instanceData.accelerationStructureHandle = bs->getHandle();
		_instanceData.push_back(instanceData);
	}
	_instanceBuffer = _ctx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV, sizeof(VkGeometryInstance) * _instanceData.size(), VulkanBuffer::CPU_ALOT, _instanceData.data());
	
	_topStruct->setNumInstances(static_cast<uint32_t>(_instanceData.size()));

	cmd->buildAccelerationStructureNV(
		_topStruct->getInfo(),
		_instanceBuffer->getBuffer(),
		vk::DeviceSize(0),
		_built,
		_topStruct->getAccelerationStruct(),
		_built ? _topStruct->getAccelerationStruct() : nullptr,
		_scratchBuffer->getBuffer(),
		vk::DeviceSize(0),
		_ctx->getDynamicDispatch()
	);
	
	vkCmdPipelineBarrier(*cmd, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
}

vk::WriteDescriptorSetAccelerationStructureNV RTScene::getWriteDescriptor()
{

	auto as = _topStruct->getAccelerationStruct();

	return vk::WriteDescriptorSetAccelerationStructureNV(
		1, &_topStruct->getAccelerationStruct()
	);
}

void RTScene::makeScratchBuffer()
{
	uint64_t scratchSize = 0;

	auto memReqs = _ctx->getDevice().getAccelerationStructureMemoryRequirementsNV(
		vk::AccelerationStructureMemoryRequirementsInfoNV(
			vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch,
			_topStruct->getAccelerationStruct()
		), _ctx->getDynamicDispatch()
	);

	scratchSize = memReqs.memoryRequirements.size;

	for (auto &bs : _bottomStructs) {

		auto memReqs = _ctx->getDevice().getAccelerationStructureMemoryRequirementsNV(
			vk::AccelerationStructureMemoryRequirementsInfoNV(
				vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch,
				bs->getAccelerationStruct()
			), _ctx->getDynamicDispatch()
		);


		scratchSize = glm::max<uint64_t>(scratchSize, memReqs.memoryRequirements.size);
	}


	_scratchBuffer = _ctx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV, scratchSize, VulkanBuffer::CPU_NEVER);

}
