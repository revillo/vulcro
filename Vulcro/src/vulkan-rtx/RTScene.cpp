#include "RTScene.h"

RTScene::RTScene(VulkanContextPtr ctx, RTBlasRepoRef geoRepo)
    :_ctx(ctx)
    , mGeoRepo(geoRepo)
{
    if (!mGeoRepo)
    {
        mGeoRepo = ctx->makeRayTracingBlasRepo();
    }

    _topStruct = std::make_shared<RTAccelerationStructure>(ctx, 1, true /* Allow Updates */);
}

void RTScene::addGeometry(const GeometryId& name, RTGeometryRef geom, bool allowUpdate)
{
    mGeoRepo->addGeometry(name, geom, allowUpdate);
    _geometryMap[name].accelStruct = mGeoRepo->getBLAS(name);
}

void RTScene::flagGeometryRebuild(const GeometryId& id)
{
    mGeoRepo->flagUpdateGeometry(id);
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
        return;
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
        makeScratchBuffer();
    }

    /*
    VkMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    */

    _topStruct->build(cmd, _scratchBuffer, _instanceBuffer);
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

    scratchSize = _topStruct->computeScratchMemorySize();

    if (!_scratchBuffer || _scratchBuffer->getSize() < scratchSize) {
        _scratchBuffer = _ctx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV, scratchSize, VulkanBuffer::CPU_NEVER);
    }
}
