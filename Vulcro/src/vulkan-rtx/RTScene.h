#include "../vulkan-core/VulkanContext.h"
#include "../vulkan-core/VulkanBuffer.h"
#include "../vulkan-core/VulkanTask.h"
#include "RTAccelerationStructure.h"

class RTScene
{
    using GeometryId = uint64_t;

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

    RTScene(VulkanContextPtr ctx, RTBlasRepoRef geoRepo = nullptr);
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

    VulkanContextPtr _ctx = nullptr;
    RTAccelStructRef _topStruct = nullptr;
    VulkanBufferRef _scratchBuffer = nullptr;
    VulkanBufferRef _instanceBuffer = nullptr;
    vector<VkGeometryInstance> _instanceData;
    RTBlasRepoRef mGeoRepo = nullptr;
};