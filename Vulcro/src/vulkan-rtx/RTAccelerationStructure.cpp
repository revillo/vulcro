#include "RTAccelerationStructure.h"

RTAccelerationStructure::RTAccelerationStructure(VulkanContextPtr ctx, uint32_t numInstances, bool allowUpdate)
	:_ctx(ctx),
	_isTop(true),
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
		numInstances,
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


RTAccelerationStructure::RTAccelerationStructure(VulkanContextPtr ctx, std::vector<RTGeometryRef> && geometries, bool allowUpdate)
	:_ctx(ctx),
	_isTop(false),
    _allowUpdate(allowUpdate),
    mGeoRefs(geometries)
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

	
	for (auto &g : geometries) {
        _geometries.push_back(g->getGeometry());
	}

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

uint64_t RTAccelerationStructure::computeScratchMemorySize()
{
    uint64_t scratchSize = 0;


    auto memReqs = _ctx->getDevice().getAccelerationStructureMemoryRequirementsNV(
        vk::AccelerationStructureMemoryRequirementsInfoNV(
            vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch,
            getAccelerationStruct()
        ), _ctx->getDynamicDispatch()
    );

    scratchSize = glm::max<uint64_t>(scratchSize, memReqs.memoryRequirements.size);

    if (_allowUpdate)
    {

        memReqs = _ctx->getDevice().getAccelerationStructureMemoryRequirementsNV(
            vk::AccelerationStructureMemoryRequirementsInfoNV(
                vk::AccelerationStructureMemoryRequirementsTypeNV::eUpdateScratch,
                getAccelerationStruct()
            ), _ctx->getDynamicDispatch()
        );

        scratchSize = glm::max<uint64_t>(memReqs.memoryRequirements.size, scratchSize);
    }

    return scratchSize;

}

static const vk::MemoryBarrier MemoryBarrier { vk::AccessFlags(vk::AccessFlagBits::eAccelerationStructureWriteNV | vk::AccessFlagBits::eAccelerationStructureReadNV),
    vk::AccessFlags(vk::AccessFlagBits::eAccelerationStructureWriteNV | vk::AccessFlagBits::eAccelerationStructureReadNV)
};
   

void RTAccelerationStructure::build(vk::CommandBuffer * cmd, VulkanBufferRef scratchBuffer, VulkanBufferRef instanceBuffer)
{
   
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

        cmd->pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, vk::DependencyFlags(0), { MemoryBarrier }, nullptr, nullptr);
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
        
        cmd->pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, vk::DependencyFlags(0), { MemoryBarrier }, nullptr, nullptr);
	}

    _built = true;
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

vk::WriteDescriptorSetAccelerationStructureNV RTTopStructure::getWriteDescriptor()
{
    return vk::WriteDescriptorSetAccelerationStructureNV(
        1, &getAccelerationStruct()
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


RTBlasRepo::RTBlasRepo(VulkanContextPtr ctx)
    :mCtx(ctx)
{
    mRebuildTask = ctx->makeTask();
}


void RTBlasRepo::flagUpdateGeometry(GeometryId id)
{
    assert(mBlas.count(id) != 0);
    mDirtyBlas.push_back(id);
}

//Can return nullptr
RTBottomStructureRef RTBlasRepo::getBLAS(GeometryId id)
{
    if (mBlas.count(id) == 0) return nullptr;
    return mBlas[id];
}

void RTBlasRepo::addGeometry(GeometryId id, RTGeometryRef geom, bool allowUpdate)
{
    auto as = RTBottomStructureRef(new RTBottomStructure(mCtx, {geom}, allowUpdate));
    mBlas[id] = as;

    auto newSize = as->computeScratchMemorySize();

    if (!mScratch || (mScratch->getSize() < newSize))
    {
        mScratch = mCtx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV, newSize, VulkanBuffer::CPU_NEVER);
    }

    flagUpdateGeometry(id);
}

void RTBlasRepo::removeGeometry(GeometryId id)
{
    mBlas.erase(id);
}

void RTBlasRepo::rebuildDirtyGeometries(VulkanTaskRef task)
{
    task = task ? task : mRebuildTask;

    if (mDirtyBlas.size() == 0) return;

    task->begin();


    for (auto id : mDirtyBlas)
    {
        mBlas[id]->build(&task->getCommandBuffer(), mScratch);
    }

    task->end();
    task->execute(true);

    for (auto id : mDirtyBlas)
    {
        //Discard geometry if no future updates to release buffers
        if (!mBlas[id]->supportsUpdate())
        {
            mBlas[id]->dropGeometryRefs();
        }
    }

    mDirtyBlas.clear();
}

RTTopStructureManager::RTTopStructureManager(VulkanContextPtr context, uint32_t numInstances)
    :mNumInstances(0)
    ,mCtx(context)
{
    resize(numInstances);
}

void RTTopStructureManager::resize(uint32_t newSize)
{
    if (mNumInstances < newSize)
    {
        mInstanceBuffer = mCtx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV, newSize * sizeof(VkGeometryInstance), VulkanBuffer::CPU_ALOT);
        mInstancePtr = (VkGeometryInstance*)mInstanceBuffer->getMapped();
        mTopStructure = RTTopStructureRef(new RTTopStructure(mCtx, newSize, true));

        auto scratchSize = mTopStructure->computeScratchMemorySize();

        if (!mScratchBuffer || mScratchBuffer->getSize() < scratchSize)
        {
            mScratchBuffer = mCtx->makeDeviceBuffer(vk::BufferUsageFlagBits::eRayTracingNV, scratchSize);
        }

    }
    else if (mNumInstances > newSize)
    {
        mTopStructure = RTTopStructureRef(new RTTopStructure(mCtx, newSize, true));
    }


    mNumInstances = newSize;
}

VkGeometryInstance& RTTopStructureManager::getInstance(uint32_t index)
{
    return mInstancePtr[index];
}

void RTTopStructureManager::rebuild(VulkanTaskRef task)
{
    if (!task)
    {
        task = mTask ? mTask : mCtx->makeTask();
    }

    task->begin();

    mTopStructure->build(&task->getCommandBuffer(), mScratchBuffer, mInstanceBuffer);

    task->end();

    task->execute(true);
}