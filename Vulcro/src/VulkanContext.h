#pragma once

#include <vulkan/vulkan.hpp>
#include "General.h"
#include <unordered_map>

typedef const char * FilePath;

struct VulkanUniformLayoutBinding {

	VulkanUniformLayoutBinding(uint32_t _arrayCount = 1, vk::DescriptorType _type = vk::DescriptorType::eUniformBuffer, vk::Sampler * _samplers = nullptr) :
		type(_type), 
		arrayCount(_arrayCount),
		samplers(_samplers)
	{

	}

	vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;
	uint32_t arrayCount = 1;
	vk::Sampler * samplers = nullptr;
};

struct PipelineConfig {
	vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
	uint32_t patchCount = 3;
	vk::CullModeFlags cullFlags = vk::CullModeFlagBits::eBack;
	vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;
};

struct ColorBlendConfig {

};


typedef VulkanUniformLayoutBinding ULB;

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
class VulkanImage;
class VulkanImage1D;
class VulkanImage2D;
class VulkanImage3D;
class VulkanImageCube;


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
class ssbo;

class ibo;

typedef shared_ptr<VulkanShader> VulkanShaderRef;
typedef shared_ptr<VulkanRenderer> VulkanRendererRef;
typedef shared_ptr<VulkanRenderPipeline> VulkanRenderPipelineRef;
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

typedef shared_ptr<RTShaderBuilder> RTShaderBuilderRef;
typedef shared_ptr<RTPipeline> RTPipelineRef;
typedef shared_ptr<RTScene> RTSceneRef;

class VulkanContext
{
public:
	VulkanContext(vk::Instance instance);

	vk::Instance getInstance() {
		return _instance;
	}

	
	const vk::DispatchLoaderDynamic getDynamicDispatch() {
		return *_dynamicDispatch;
	}

	const vk::Device &getDevice() { return _device; }
	const vk::PhysicalDevice &getPhysicalDevice() { return _physicalDevices[0]; }

	vk::Queue &getQueue() {
		return _queue;
	}

	void resetTasks(uint32_t poolIndex = 0) {

		_device.resetCommandPool(
			_pools[poolIndex],
			vk::CommandPoolResetFlags()
		);

	}

	VulkanVertexLayoutRef makeVertexLayout(vk::ArrayProxy<const vk::Format> fields);
	VulkanRendererRef makeRenderer();

	VulkanRenderPipelineRef makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer, PipelineConfig config = PipelineConfig(),
		vector<ColorBlendConfig> colorBlendConfigs = {}
	);

	VulkanComputePipelineRef makeComputePipeline(VulkanShaderRef shader);
	VulkanComputePipelineRef makeComputePipeline(const char * shaderPath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts );

	VulkanSwapchainRef makeSwapchain(vk::SurfaceKHR surface);

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

	VulkanBufferRef makeBuffer(
		vk::BufferUsageFlags usage,
		uint64_t size,
		vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		, void* data = nullptr
	);

    /*
    * GPU only buffer that is initialized with a staging buffer. 
    */
	VulkanBufferRef makeFastBuffer(
		vk::BufferUsageFlags usage,
		uint64_t size,
		void *data = nullptr
	);


	VulkanBufferRef makeLocalStorageBuffer(
		uint64_t size
	);

	VulkanSetLayoutRef makeSetLayout(vk::ArrayProxy<const VulkanUniformLayoutBinding> bindings);

	VulkanSetRef makeSet(
		VulkanSetLayoutRef layout
	);

	VulkanSetRef makeSet(
		vk::ArrayProxy<const VulkanUniformLayoutBinding> bindings
	);

	VulkanTaskRef makeTask(uint32_t poolIndex = 0, bool autoReset = false);
	VulkanTaskGroupRef makeTaskGroup(uint32_t numTasks, uint32_t poolIndex = 0);

	VulkanImage1DRef makeImage1D(vk::ImageUsageFlags usage, vk::Format format, float size);
	VulkanImage1DRef makeImage1D(vk::Image image, vk::Format format, float size);

	VulkanImage2DRef makeImage2D(vk::ImageUsageFlags usage, vk::Format format, glm::ivec2 size);
	VulkanImage2DRef makeImage2D(vk::Image image, vk::Format format, glm::ivec2 size);

	VulkanImage3DRef makeImage3D(vk::ImageUsageFlags usage, vk::Format format, glm::ivec3 size);
	VulkanImage3DRef makeImage3D(vk::Image image, vk::Format format, glm::ivec3 size);
	
	VulkanImageCubeRef makeImageCube(vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format);

	template <class T>
	shared_ptr<ubo<T>> makeUBO(uint32_t arrayCount, T * data = nullptr) {
		return make_shared<ubo<T>>(this, arrayCount, data);
	}

	template <class T>
	shared_ptr<static_vbo<T>> makeVBO(vk::ArrayProxy<const vk::Format> fieldFormats, uint32_t arrayCount, T * data = nullptr) {
		return make_shared<static_vbo<T>>(this, fieldFormats, arrayCount, data);
	}

	template <class T>
	shared_ptr<static_vbo<T>> makeVBO(shared_ptr<static_vbo<T>> sourceBuffer, uint32_t vertOffset, uint32_t numVerts) {
		return make_shared<static_vbo<T>>(sourceBuffer, vertOffset, numVerts);
	}

	template <class T>
	shared_ptr<dynamic_vbo<T>> makeDynamicVBO(vk::ArrayProxy<const vk::Format> fieldFormats, uint32_t arrayCount, T * data = nullptr) {
		return make_shared<dynamic_vbo<T>>(this, fieldFormats, arrayCount, data);
	}

	template <class T>
	shared_ptr<ssbo<T>> makeSSBO(uint32_t arrayCount) {
		return make_shared<ssbo<T>>(this, arrayCount);
	}

	shared_ptr<ibo> makeIBO(vk::ArrayProxy<const uint32_t> indices);

	shared_ptr<ibo> makeIBO(shared_ptr<ibo> sourceIbo, uint32_t indexOffset, uint32_t numIndices);
	
	vk::Sampler getLinearSampler();
	vk::Sampler getNearestSampler();
	vk::Sampler getShadowSampler();

	shared_ptr<RTShaderBuilder> makeRayTracingShaderBuilder(const char * raygenPath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts);
	shared_ptr<RTPipeline> makeRayTracingPipeline(RTShaderBuilderRef shader);
	RTSceneRef makeRayTracingScene();

	~VulkanContext();
		
	uint32_t getFamilyIndex() {
		return _familyIndex;
	}

private:

	uint32_t _familyIndex;
	vk::DispatchLoaderDynamic * _dynamicDispatch;

	vk::Instance _instance;
	vector<vk::PhysicalDevice> _physicalDevices;
	vk::Device _device;
	
	std::unordered_map<uint32_t, vk::CommandPool> _pools;


	vk::CommandBuffer _cmd;
	vk::Queue _queue;
	vk::Sampler _linearSampler = nullptr, _nearestSampler = nullptr, _shadowSampler = nullptr;


	vk::Sampler createSampler2D(vk::Filter filter);
};

typedef VulkanContext * VulkanContextRef;

