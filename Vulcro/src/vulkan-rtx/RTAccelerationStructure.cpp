#include "RTAccelerationStructure.h"

RTAccelerationStructure::RTAccelerationStructure(VulkanContextPtr ctx, uint32_t numInstances, bool allowUpdate)
	:_ctx(ctx),
	_isTop(true),
	_needsRebuild(true),
    _allowUpdate(allowUpdate)
{

    vk::BuildAccelerationStructureFlagBitsNV flags;

    if (allowUpdate)
    {
        flags = vk::BuildAccelerationStructureFlagBitsNV::eAllowUpdate;
    }
    else
    {
        flags = vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace;
    }

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


RTAccelerationStructure::RTAccelerationStructure(VulkanContextPtr ctx, RTGeometryRef geometry, bool allowUpdate)
	:_ctx(ctx),
	_isTop(false),
	_needsRebuild(true),
    _allowUpdate(allowUpdate),
    mGeoRef(geometry)
{

    vk::BuildAccelerationStructureFlagBitsNV flags;


    if (allowUpdate)
    {
        flags = vk::BuildAccelerationStructureFlagBitsNV::eAllowUpdate;
    }
    else
    {
        flags = vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace;
    }


	/*
	for (auto &g : geometries) {
		geos.push_back(g.getGeometry());
	}*/
	_geometries.push_back(geometry->getGeometry());

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

void RTAccelerationStructure::build(vk::CommandBuffer * cmd, VulkanBufferRef scratchBuffer, VkMemoryBarrier &memoryBarrier, VulkanBufferRef instanceBuffer)
{
	if (!_needsRebuild) {
		return;
	}

	if (_isTop) {

		cmd->buildAccelerationStructureNV(
			getInfo(),
			instanceBuffer ? instanceBuffer->getBuffer() : nullptr,
			vk::DeviceSize(0),
			_built,
			getAccelerationStruct(),
			_built ? getAccelerationStruct() : nullptr,
			scratchBuffer->getBuffer(),
			vk::DeviceSize(0),
			_ctx->getDynamicDispatch()
		);

        cmd->pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, vk::DependencyFlags(0), { memoryBarrier }, nullptr, nullptr);
	}
	else {

		cmd->buildAccelerationStructureNV(
			getInfo(),
			nullptr,
			vk::DeviceSize(0),
			_built,
			getAccelerationStruct(),
			_built ? getAccelerationStruct() : nullptr,
			scratchBuffer->getBuffer(),
			vk::DeviceSize(0),
			_ctx->getDynamicDispatch()
		);
        
        cmd->pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, vk::DependencyFlags(0), { memoryBarrier }, nullptr, nullptr);
	}

    _built = true;
	_needsRebuild = false;

    //if (_allowUpdate)

    //No need to hold onto geometry after building
    //mGeoRef = nullptr;
}

void RTAccelerationStructure::allocateDeviceMemory()
{

    if (_memorySize > 0)
    {
        _ctx->getDevice().freeMemory(_memory, nullptr, _ctx->getDynamicDispatch());
    }

    auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();

    auto memReqs = _ctx->getDevice().getAccelerationStructureMemoryRequirementsNV(
        vk::AccelerationStructureMemoryRequirementsInfoNV(
            vk::AccelerationStructureMemoryRequirementsTypeNV::eObject,
            _accelStruct
        ), _ctx->getDynamicDispatch()
    );

    auto req = memReqs.memoryRequirements;

    auto memRes = _ctx->getBestMemoryIndex(memReqs.memoryRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);

    /*
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
    {
        auto type = memProps.memoryTypes[i];
        if ((reqBits & (1 << i)) && ((type.propertyFlags & p) == p))
        {

            memTypeIndex = i;
            break;
        }
    }*/



    _memorySize = req.size;



    _memory = _ctx->getDevice().allocateMemory(
        vk::MemoryAllocateInfo(req.size, memRes.value)
    );

    _ctx->getDevice().bindAccelerationStructureMemoryNV({

        vk::BindAccelerationStructureMemoryInfoNV(
        _accelStruct,
        _memory,
        0) }

    , _ctx->getDynamicDispatch());

    _ctx->getDevice().getAccelerationStructureHandleNV(
        _accelStruct,
        sizeof(uint64_t),
        &_handle,
        _ctx->getDynamicDispatch()
    );
}

RTGeometry::RTGeometry(uint64_t aabbCount, uint64_t aabbOffset, VulkanBufferRef aabbBuffer)
    :_geometry(),
    _indexBuffer(nullptr),
    _vertexBuffer(nullptr),
    mFaceVertexBuffer(nullptr),
    mAABBBuffer(aabbBuffer)
{
    vk::GeometryAABBNV aabb;
    aabb.setNumAABBs(aabbCount);
    aabb.setStride(sizeof(float) * 6);
    aabb.setOffset(aabbOffset);
    aabb.setPNext(nullptr);
    aabb.setAabbData(aabbBuffer->getBuffer());
    vk::GeometryDataNV geometry;
    geometry.setAabbs(aabb);
    _geometry.setGeometryType(vk::GeometryTypeNV::eAabbs);
    _geometry.setGeometry(geometry);
    _geometry.setFlags(vk::GeometryFlagBitsNV::eOpaque);
    //_geometry.setFlags(vk::GeometryFlagsNV());
}

RTGeometry::RTGeometry(VulkanBufferRef vertexBuffer, uint64_t vertexCount,  uint32_t vertexStride, uint64_t positionOffset, vk::Format positionFormat)
    :_geometry(),
    _indexBuffer(nullptr),
    _vertexBuffer(nullptr),
    mFaceVertexBuffer(vertexBuffer)
{
    vk::GeometryTrianglesNV triangles;

    triangles.setIndexType(vk::IndexType::eNoneNV);

    triangles.setVertexCount(vertexCount);
    triangles.setVertexData(vertexBuffer->getBuffer());

    triangles.setVertexFormat(positionFormat);
    triangles.setVertexOffset(positionOffset);
    triangles.setVertexStride(vertexStride);

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

RTScene::RTScene(VulkanContextPtr ctx, RTGeometryRepoRef geoRepo)
	:_ctx(ctx)
    ,mGeoRepo(geoRepo)
{
    if (!mGeoRepo)
    {
        mGeoRepo = ctx->makeRayTracingGeometryRepo();
    }

	_topStruct = std::make_shared<RTAccelerationStructure>(ctx, 1, true /* Allow Updates */);
	_topStruct->setNeedsRebuild(true);
}

void RTScene::addGeometry(const GeometryId& name, RTGeometryRef geom, bool allowUpdate)
{
    mGeoRepo->addGeometry(name, geom, allowUpdate);
	_geometryMap[name].accelStruct = mGeoRepo->getBLAS(name);
}

void RTScene::flagGeometryRebuild(const GeometryId& id)
{
    mGeoRepo->flagUpdateGeometry(id);
    _topStruct->setNeedsRebuild(true);
}

void RTScene::setInstanceCount(const GeometryId& geometryName, uint32_t numInstances)
{
	assert(_geometryMap.count(geometryName) > 0);
	_geometryMap[geometryName].instances.resize(numInstances);
}

uint32_t RTScene::getInstanceCount(const GeometryId& geometryName)
{
	assert(_geometryMap.count(geometryName) > 0);
	return static_cast<uint32_t>(_geometryMap[geometryName].instances.size());
}

std::array<float, 12> RTScene::getInstanceTransform(const GeometryId & geometryName, uint32_t instanceIndex)
{
	assert(_geometryMap.count(geometryName) > 0);
	assert(_geometryMap[geometryName].instances.size() > instanceIndex);

	return _geometryMap[geometryName].instances[instanceIndex].transform;
}

void RTScene::setInstanceTransform(const GeometryId& geometryName, uint32_t instanceIndex, glm::mat4 const & transformTranspose)
{
	assert(_geometryMap.count(geometryName) > 0);
	assert(_geometryMap[geometryName].instances.size() > instanceIndex);

    glm::mat4 transform = glm::transpose(transformTranspose);

    _geometryMap[geometryName].instances[instanceIndex].transform = std::array<float, 12>{transform[0][0], transform[0][1], transform[0][2], transform[0][3], transform[1][0], transform[1][1], transform[1][2], transform[1][3], transform[2][0], transform[2][1], transform[2][2], transform[2][3]};
	_topStruct->setNeedsRebuild(true);
}

void RTScene::setInstanceData(const GeometryId& id, uint32_t instanceIndex, const InstanceData& data)
{
    auto & instance = _geometryMap[id].instances[instanceIndex];
    instance.flags = (uint32_t)data.flags;
    instance.instanceCustomIndex = data.customIndexU24;
    instance.instanceShaderBindingTableRecordOffset = data.sbtOffset;
    instance.mask = data.mask;
}

void RTScene::addInstance(const GeometryId& geometryName, glm::mat4 const & transformTranspose, InstanceData const & data)
{
    assert(_geometryMap.count(geometryName) > 0);

    auto & instances = _geometryMap[geometryName].instances;

    VkGeometryInstance instance;
    instance.mask = data.mask;
    instance.instanceCustomIndex = data.customIndexU24;
    instance.instanceShaderBindingTableRecordOffset = data.sbtOffset;
    instances.push_back(instance);
    
    setInstanceTransform(geometryName, instances.size() - 1, transformTranspose);
}

uint32_t RTScene::getGeometryIndex(const GeometryId& geometryName)
{
	assert(_geometryMap.count(geometryName) > 0);

	return _geometryMap[geometryName].bindingIndex;
}

uint32_t RTScene::getInstanceGlobalIndex(const GeometryId& geometryName, uint32_t instanceIndex)
{
	assert(_geometryMap.count(geometryName) > 0);
	assert(_geometryMap[geometryName].instances.size() > instanceIndex);
	return _geometryMap[geometryName].instanceOffset + instanceIndex;
}

#include "../vulkan-core/VulkanTask.h"
void RTScene::build(VulkanTaskRef task)
{
    mGeoRepo->rebuildDirtyGeometries();

    task->begin();
    build(&task->getCommandBuffer());
    task->end();

    task->execute(true);
}

void RTScene::build(vk::CommandBuffer * cmd)
{
	//TODO only make buffer when instances change size?

    int instanceGlobalIndex = 0;
	int bindingIndex = 0;

	_instanceData.clear();

	for (auto &iter : _geometryMap) {

		iter.second.bindingIndex = bindingIndex;
		iter.second.instanceOffset = instanceGlobalIndex;

		for (auto &instance : iter.second.instances) {
			VkGeometryInstance instanceData;
            instanceData.instanceCustomIndex = instance.instanceCustomIndex;
			instanceData.instanceShaderBindingTableRecordOffset = instance.instanceShaderBindingTableRecordOffset;
			instanceData.mask = instance.mask;
			instanceData.transform = instance.transform;
            instanceData.flags = instance.flags;
          
			instanceData.accelerationStructureHandle = iter.second.accelStruct->getHandle();
			_instanceData.push_back(instanceData);

			++instanceGlobalIndex;
		}

		++bindingIndex;
	}

	auto newSize = sizeof(VkGeometryInstance) * _instanceData.size();

    if (newSize == 0)
    {
        _instanceBuffer = nullptr;
    }
    else if (!_instanceBuffer || newSize > _instanceBuffer->getSize())
    {
		_instanceBuffer = _ctx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV, newSize * 10, VulkanBuffer::CPU_ALOT);
	}
	
	auto * p = _instanceBuffer->getMapped();
	memcpy(p, _instanceData.data(), newSize);
	_instanceBuffer->unmap();
	
    if (_instanceData.size() != _topStruct->getInfo().instanceCount) {
        _topStruct = std::make_shared<RTAccelerationStructure>(_ctx, _instanceData.size(), true /* Allow Updates */);
        _topStruct->setNeedsRebuild(true);
        makeScratchBuffer();
    }

    VkMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    
	_topStruct->build(cmd, _scratchBuffer, memoryBarrier, _instanceBuffer);
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

	scratchSize = glm::max<uint64_t>(memReqs.memoryRequirements.size, scratchSize);

    memReqs = _ctx->getDevice().getAccelerationStructureMemoryRequirementsNV(
        vk::AccelerationStructureMemoryRequirementsInfoNV(
            vk::AccelerationStructureMemoryRequirementsTypeNV::eUpdateScratch,
            _topStruct->getAccelerationStruct()
        ), _ctx->getDynamicDispatch()
    );


    scratchSize = glm::max<uint64_t>(memReqs.memoryRequirements.size, scratchSize);

	if (!_scratchBuffer || _scratchBuffer->getSize() < scratchSize) {
		_scratchBuffer = _ctx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV, scratchSize, VulkanBuffer::CPU_NEVER);
	}
}


RTGeometryRepo::RTGeometryRepo(VulkanContextPtr ctx)
    :mCtx(ctx)
{
    mRebuildTask = ctx->makeTask();
}


void RTGeometryRepo::flagUpdateGeometry(const GeometryId & id)
{
    assert(mBlas.count(id) != 0);
    mDirtyBlas.push_back(id);
}

//Can return nullptr
RTAccelStructRef RTGeometryRepo::getBLAS(const GeometryId & id)
{
    if (mBlas.count(id) == 0) return nullptr;
    return mBlas[id];
}

void RTGeometryRepo::addGeometry(const GeometryId & id, RTGeometryRef geom, bool allowUpdate)
{
    auto as = RTAccelStructRef(new RTAccelerationStructure(mCtx, geom, allowUpdate));
    mBlas[id] = as;
    as->setNeedsRebuild(true);

    //updateScratchBuffer

    uint64_t scratchSize = mScratch ? mScratch->getSize() : 0;

    auto memReqs = mCtx->getDevice().getAccelerationStructureMemoryRequirementsNV(
        vk::AccelerationStructureMemoryRequirementsInfoNV(
            vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch,
            as->getAccelerationStruct()
        ), mCtx->getDynamicDispatch()
    );

    scratchSize = glm::max<uint64_t>(scratchSize, memReqs.memoryRequirements.size);

    memReqs = mCtx->getDevice().getAccelerationStructureMemoryRequirementsNV(
        vk::AccelerationStructureMemoryRequirementsInfoNV(
            vk::AccelerationStructureMemoryRequirementsTypeNV::eUpdateScratch,
            as->getAccelerationStruct()
        ), mCtx->getDynamicDispatch()
    );

    scratchSize = glm::max<uint64_t>(memReqs.memoryRequirements.size, scratchSize);
    mScratch = mCtx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV, scratchSize, VulkanBuffer::CPU_NEVER);
    flagUpdateGeometry(id);
}

void RTGeometryRepo::rebuildDirtyGeometries()

{
    if (mDirtyBlas.size() == 0) return;

    mRebuildTask->begin();

    VkMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

    for (auto id : mDirtyBlas)
    {
        mBlas[id]->setNeedsRebuild(true);
        mBlas[id]->build(&mRebuildTask->getCommandBuffer(), mScratch, memoryBarrier);
    }


    mRebuildTask->end();

    mDirtyBlas.clear();

    mRebuildTask->execute(true);
}