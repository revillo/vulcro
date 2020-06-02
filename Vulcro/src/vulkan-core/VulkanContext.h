#pragma once

#include <vulkan/vulkan.hpp>
#include "General.h"
#include <unordered_map>

typedef const char * FilePath;

struct VulkanSetLayoutBinding {

	VulkanSetLayoutBinding(uint32_t _arrayCount, vk::DescriptorType _type, vk::Sampler const * _samplers = nullptr, vk::ShaderStageFlags _stageFlags = vk::ShaderStageFlagBits::eAll) :
		type(_type), 
		arrayCount(_arrayCount),
		samplers(_samplers),
        stageFlags(_stageFlags)
	{}

    VulkanSetLayoutBinding()
    {}

	vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;
	uint32_t arrayCount = 1;
	vk::Sampler const * samplers = nullptr;
    vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll;
};

struct PipelineConfig {
	vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
	uint32_t patchCount = 3;
	vk::CullModeFlags cullFlags = vk::CullModeFlagBits::eBack;
	vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;
};

enum VulkanColorBlend {
    VULCRO_BLEND_OPAQUE,
    VULCRO_BLEND_ALPHA
};

struct ColorBlendConfig {
    VulkanColorBlend blend = VULCRO_BLEND_ALPHA;
};


typedef VulkanSetLayoutBinding SLB;

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
class RTGeometryRepo;
class RTScene;
class RTPipeline;
class RTShaderBuilder;

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

class ibo;
class ivbo;

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
typedef shared_ptr<RTGeometryRepo> RTGeometryRepoRef;
typedef shared_ptr<RTShaderBuilder> RTShaderBuilderRef;
typedef shared_ptr<RTPipeline> RTPipelineRef;
typedef shared_ptr<RTScene> RTSceneRef;
typedef shared_ptr<ibo> iboRef;
typedef shared_ptr<ivbo> vboRef;
//typedef shared_ptr<issbo> ssboRef;

template<typename T>
using uboRef = shared_ptr<ubo<T>>;

class VulkanContext
{



    template<typename T, typename ErrorCode>
    struct res {

        res()
            :isValid(false)
        {

        }

        res(ErrorCode error, std::string errorMessage = "")
            :isValid(false)
            , message(errorMessage)
            , error(error)
        {}

        res(T &t)
            :value(t)
            , isValid(true)
            , message("")
        {}

        template<typename Child>
        res(res<Child, ErrorCode> childRes)
        {
            error = childRes.error;
            message = childRes.message;
            isValid = childRes.isValid;

            if (isValid)
            {
                value = childRes.value;
            }
        }

        T value;
        bool isValid;
        std::string message;
        ErrorCode error;
    };


public:
    

    enum VulcroError {
       NOT_AN_ERROR = 0,
       MEMORY_TYPE_NOT_SUPPORTED
    };

    /****************************
        Constructors / Destructor
    ****************************/

	VulkanContext(vk::Instance instance, vk::PhysicalDevice& pDevice, const std::vector<const char *> & deviceExtensions = { "VK_KHR_swapchain", "VK_KHR_get_memory_requirements2", "VK_NV_ray_tracing" });

    ~VulkanContext();


	vk::Instance getInstance() {
		return _instance;
	}

	
	const vk::DispatchLoaderDynamic getDynamicDispatch()
    {
		return *_dynamicDispatch;
	}

	const vk::Device &getDevice() { return _device; }
	const vk::PhysicalDevice &getPhysicalDevice() { return _physicalDevice; }

	vk::Queue &getQueue(uint32_t queueIndex = 0) {
		
        return _queues[0];
        //return getDevice().getQueue(_familyIndex, queueIndex);

	}

	VulkanVertexLayoutRef makeVertexLayout(vk::ArrayProxy<const vk::Format> fields);
	VulkanRendererRef makeRenderer();

	VulkanRenderPipelineRef makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer, PipelineConfig config = PipelineConfig(), 
		vector<ColorBlendConfig> colorBlendConfigs = {}, uint32_t pushConstantSize = 0
	);

	VulkanComputePipelineRef makeComputePipeline(VulkanShaderRef shader, uint32_t pushConstantSize = 0);
	VulkanComputePipelineRef makeComputePipeline(const char * shaderPath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts, uint32_t pushConstantSize = 0);

	VulkanSwapchainRef makeSwapchain(vk::SurfaceKHR surface);

    
    res<uint32_t, VulcroError> getBestMemoryIndex(vk::MemoryRequirements memReqs, vk::MemoryPropertyFlags memFlags)
    {
        auto reqBits = memReqs.memoryTypeBits;
        
        auto memProps = getPhysicalDevice().getMemoryProperties();


        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            auto memoryType = memProps.memoryTypes[i];
            if ((reqBits & (1 << i)) && (memoryType.propertyFlags & memFlags) == memFlags)
            {
                return i;
            }
        }

        return res<uint32_t, VulcroError>(MEMORY_TYPE_NOT_SUPPORTED, "Memory not supported.");
    }

    /****************************
        Shaders
    ****************************/

	VulkanShaderRef makeShader(const char * vertPath,
		const char * fragPath,
		vk::ArrayProxy<const VulkanVertexLayoutRef> vertexLayouts,
		vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts = {});

	VulkanShaderRef makeTessShader(
		const char * vertPath,
		const char * tessControlPath,
		const char * tessEvalPath,
		const char * tessGeomPath,
		const char * fragPath,
		vk::ArrayProxy<const VulkanVertexLayoutRef> vertexLayouts = {},
		vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts = {}
	);

	VulkanShaderRef makeComputeShader(
		const char * computePath,
		vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts = {}
	);

    /****************************
        Sets / Set Layouts
    ****************************/

	VulkanSetLayoutRef makeSetLayout(vk::ArrayProxy<const VulkanSetLayoutBinding> bindings, uint32_t maxSets = 1);

	VulkanSetRef makeSet(VulkanSetLayoutRef layout);

	VulkanSetRef makeSet(vk::ArrayProxy<const VulkanSetLayoutBinding> bindings);

    /*********************
        Tasks / Command Buffers
    **********************/

    VulkanTaskPoolRef makeTaskPool(vk::CommandPoolCreateFlags createFlags = vk::CommandPoolCreateFlags());

    VulkanTaskRef makeTask(VulkanTaskPoolRef taskPool);

	VulkanTaskRef makeTask();
	
    VulkanTaskGroupRef makeTaskGroup(uint32_t numTasks);
    VulkanTaskGroupRef makeTaskGroup(uint32_t numTasks, VulkanTaskPoolRef taskPool);
   

    /****************************
        Samplers
    ****************************/

    vk::Sampler const& getLinearSampler();
    vk::Sampler const& getNearestSampler();
    vk::Sampler const& getShadowSampler();

    /****************************
        Images
    ****************************/

	VulkanImage1DRef makeImage1D(vk::ImageUsageFlags usage, vk::Format format, float size);
	VulkanImage1DRef makeImage1D(vk::Image image, vk::Format format, float size);

	VulkanImage2DRef makeImage2D(vk::ImageUsageFlags usage, vk::Format format, glm::uvec2 size, vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal);
	VulkanImage2DRef makeImage2D(vk::ImageUsageFlags usage, vk::Format format, glm::uvec2 size, uint16_t mipLevels, vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal);
	VulkanImage2DRef makeImage2D(vk::Image image, vk::Format format, glm::uvec2 size, uint16_t mipLevels = 1);


	VulkanImage3DRef makeImage3D(vk::ImageUsageFlags usage, vk::Format format, glm::uvec3 size);
	VulkanImage3DRef makeImage3D(vk::Image image, vk::Format format, glm::uvec3 size);
	
	VulkanImageCubeRef makeImageCube(vk::ImageUsageFlags usage, glm::uvec2 size, vk::Format format);

    VulkanImage2DRef makeTexture2D_RGBA(glm::uvec2 size, uint16_t mipLevels = 1, void * pixelData = nullptr);

    /****************************
        Buffers
    ****************************/

    VulkanBufferRef makeBuffer(vk::BufferUsageFlags usage, uint64_t size, vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eHostVisible, void* data = nullptr);

    /**
      GPU only buffer that is initialized with a staging buffer.

      @param usage - How is this buffer going to be used (i.e. ubo ibo, vbo, ...)
      @param sizeBytes - How large of a buffer to allocate in bytes
      @param data - (optional) The initial data to stage and copy over.
    */
    VulkanBufferRef makeDeviceBuffer(vk::BufferUsageFlags usage, uint64_t sizeBytes, void* data = nullptr);

    VulkanBufferRef makeStagingBuffer(vk::BufferUsageFlags usage, uint64_t sizeBytes, void* data = nullptr);
    VulkanBufferRef makeStagingStorageBuffer(uint64_t sizeBytes, void * data = nullptr);

    VulkanBufferRef makeDeviceStorageBuffer(uint64_t size, void * data = nullptr);

	template <class T>
	shared_ptr<ubo<T>> makeUBO(uint32_t arrayCount, T * data = nullptr)
    {
		return make_shared<ubo<T>>(this, arrayCount, data);
	}

	template <class T>
	shared_ptr<static_vbo<T>> makeVBO(vk::ArrayProxy<const vk::Format> fieldFormats, uint32_t arrayCount, T * data = nullptr)
    {
		return make_shared<static_vbo<T>>(this, fieldFormats, arrayCount, data);
	}

	template <class T>
	shared_ptr<static_vbo<T>> makeVBO(shared_ptr<static_vbo<T>> sourceBuffer, uint32_t vertOffset, uint32_t numVerts)
    {
		return make_shared<static_vbo<T>>(sourceBuffer, vertOffset, numVerts);
	}

	template <class T>
	shared_ptr<dynamic_vbo<T>> makeDynamicVBO(vk::ArrayProxy<const vk::Format> fieldFormats, uint32_t arrayCount, T * data = nullptr)
    {
		return make_shared<dynamic_vbo<T>>(this, fieldFormats, arrayCount, data);
	}

	template <class T>
	shared_ptr<dynamic_ssbo<T>> makeSSBO(uint32_t arrayCount)
    {
        return make_shared<dynamic_ssbo<T>>(this, arrayCount);
	}

	shared_ptr<ibo> makeIBO(vk::ArrayProxy<const uint32_t> indices);

	shared_ptr<ibo> makeIBO(shared_ptr<ibo> sourceIbo, uint32_t indexOffset, uint32_t numIndices);

    /****************************
        RTX
    ****************************/

	RTGeometryRef makeRayTracingGeometry(iboRef indexBuffer, vboRef vertexBuffer);
    RTGeometryRef makeRayTracingGeometry(uint64_t aabbCount, uint64_t aabbOffset, VulkanBufferRef aabbBuffer);
    RTGeometryRef makeRayTracingGeometry(VulkanBufferRef vertexBuffer, uint64_t vertexCount, uint32_t vertexStride, uint32_t positionOffset = 0, vk::Format positionFormat = vk::Format::eR32G32B32A32Sfloat);
    RTGeometryRepoRef makeRayTracingGeometryRepo();

	RTShaderBuilderRef makeRayTracingShaderBuilder(const char * raygenPath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts);
	RTPipelineRef makeRayTracingPipeline(RTShaderBuilderRef shader);
	RTSceneRef makeRayTracingScene();
		
	uint32_t getFamilyIndex() {
		return _familyIndex;
	}

    uint32_t getQueueCount()
    {
        return _queueCount;
    }

    const vk::PhysicalDeviceProperties &getPhysicalDeviceProperties()
    {
        return mPhysicalDeviceProperties;
    }

private:

	uint32_t _familyIndex;
	vk::DispatchLoaderDynamic * _dynamicDispatch;

	vk::Instance _instance;
    vk::PhysicalDevice _physicalDevice;
	vk::Device _device;

    vk::PhysicalDeviceProperties mPhysicalDeviceProperties;
	
    VulkanTaskPoolRef mOneTimePool = nullptr;

    uint32_t _queueCount = 0;

	vk::CommandBuffer _cmd;
	vector<vk::Queue> _queues;
	vk::Sampler _linearSampler = nullptr, _nearestSampler = nullptr, _shadowSampler = nullptr;


	vk::Sampler createSampler2D(vk::Filter filter);
};

typedef VulkanContext * VulkanContextPtr;
typedef std::shared_ptr<VulkanContext> VulkanContextRef;
