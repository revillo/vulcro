#pragma once

#include <memory>
using std::shared_ptr;


class VulkanContext;
class VulkanSetLayout;
class VulkanSet;
class VulkanVertexLayout;
class VulkanRenderer;
class VulkanShader;
class VulkanRenderPipeline;
class VulkanComputePipeline;
class VulkanSwapchain;
class VulkanBuffer;
class VulkanSet;
class VulkanTask;
class VulkanTaskGroup;
class VulkanTaskPool;
class VulkanImage;
class VulkanImage1D;
class VulkanImage2D;
class VulkanImage3D;
class VulkanImageCube;

class RTGeometry;
class RTBlasRepo;
class RTScene;
class RTPipeline;
class RTShaderBuilder;
class RTAccelerationStructure;
class RTBottomStructure;
class RTTopStructure;
class RTTopStructureManager;

template <class T>
class ubo;

template <class T>
class vbo;

template <class T>
class static_vbo;

template <class T>
class dynamic_vbo;

template <class T>
class dynamic_ssbo;

template <class T>
class VulkanCoherentArray;

class ibo;
class ivbo;
class iubo;
class issbo;

typedef VulkanContext * VulkanContextPtr;
typedef shared_ptr<VulkanContext> VulkanContextRef;
typedef shared_ptr<VulkanShader> VulkanShaderRef;
typedef shared_ptr<VulkanRenderer> VulkanRendererRef;
typedef shared_ptr<VulkanRenderPipeline> VulkanRenderPipelineRef;
typedef VulkanRenderPipelineRef VulkanPipelineRef;
typedef shared_ptr<VulkanSetLayout> VulkanSetLayoutRef;
typedef shared_ptr<VulkanVertexLayout> VulkanVertexLayoutRef;
typedef shared_ptr<VulkanSwapchain> VulkanSwapchainRef;
typedef shared_ptr<VulkanBuffer> VulkanBufferRef;
typedef shared_ptr<VulkanSet> VulkanSetRef;
typedef shared_ptr<VulkanTask> VulkanTaskRef;
typedef shared_ptr<VulkanImage> VulkanImageRef;
typedef shared_ptr<VulkanImage1D> VulkanImage1DRef;
typedef shared_ptr<VulkanImage2D> VulkanImage2DRef;
typedef shared_ptr<VulkanImage3D> VulkanImage3DRef;
typedef shared_ptr<VulkanImageCube> VulkanImageCubeRef;
typedef shared_ptr<VulkanTaskGroup> VulkanTaskGroupRef;
typedef shared_ptr<VulkanComputePipeline> VulkanComputePipelineRef;
typedef shared_ptr<VulkanTaskPool> VulkanTaskPoolRef;

typedef shared_ptr<RTGeometry> RTGeometryRef;
typedef shared_ptr<RTBlasRepo> RTBlasRepoRef;
typedef shared_ptr<RTShaderBuilder> RTShaderBuilderRef;
typedef shared_ptr<RTPipeline> RTPipelineRef;
typedef shared_ptr<RTAccelerationStructure> RTAccelStructRef;
typedef shared_ptr<RTTopStructure> RTTopStructureRef;
typedef shared_ptr<RTTopStructureManager> RTTopStructureManagerRef;
typedef shared_ptr<RTBottomStructure> RTBottomStructureRef;
typedef shared_ptr<RTScene> RTSceneRef;
typedef shared_ptr<ibo> iboRef;
typedef shared_ptr<ivbo> vboRef;
typedef shared_ptr<iubo> iuboRef;

typedef shared_ptr<issbo> issboRef;

template<typename T>
using uboRef = shared_ptr<ubo<T>>;

template<typename T>
using ssboRef = shared_ptr<dynamic_ssbo<T>>;

template<typename T>
using VulkanCoherentArrayRef = shared_ptr<VulkanCoherentArray<T>>;
