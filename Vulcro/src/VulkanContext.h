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
class VulkanPipeline;
class VulkanComputePipeline;
class VulkanSwapchain;
class VulkanBuffer;
class VulkanSet;
class VulkanTask;
class VulkanTaskGroup;
class VulkanImage;

template <class T>
class ubo;

template <class T>
class vbo;

template <class T>
class static_vbo;

template <class T>
class ssbo;

class ibo;

typedef shared_ptr<VulkanShader> VulkanShaderRef;
typedef shared_ptr<VulkanRenderer> VulkanRendererRef;
typedef shared_ptr<VulkanPipeline> VulkanPipelineRef;
typedef shared_ptr<VulkanSetLayout> VulkanSetLayoutRef;
typedef shared_ptr<VulkanVertexLayout> VulkanVertexLayoutRef;
typedef shared_ptr<VulkanSwapchain> VulkanSwapchainRef;
typedef shared_ptr<VulkanBuffer> VulkanBufferRef;
typedef shared_ptr<VulkanSet> VulkanSetRef;
typedef shared_ptr<VulkanTask> VulkanTaskRef;
typedef shared_ptr<VulkanImage> VulkanImageRef;
typedef shared_ptr<VulkanTaskGroup> VulkanTaskGroupRef;
typedef shared_ptr<VulkanComputePipeline> VulkanComputePipelineRef;


class VulkanContext
{
public:
	VulkanContext(vk::Instance instance);
	vk::Device &getDevice() { return _device; }
	vk::PhysicalDevice &getPhysicalDevice() { return _physicalDevices[0]; }

	vk::Queue &getQueue() {
		return _queue;
	}

	void resetTasks(uint32_t poolIndex = 0) {

		_device.resetCommandPool(
			_pools[poolIndex],
			vk::CommandPoolResetFlags()
		);

	}

	VulkanVertexLayoutRef makeVertexLayout(temps<vk::Format> fields);
	VulkanRendererRef makeRenderer();

	VulkanPipelineRef makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer, PipelineConfig config = PipelineConfig(),
		vector<ColorBlendConfig> colorBlendConfigs = {}
	);

	VulkanComputePipelineRef makeComputePipeline(VulkanShaderRef shader);
	VulkanComputePipelineRef makeComputePipeline(const char * shaderPath, vector<VulkanSetLayoutRef> && setLayouts );

	VulkanSwapchainRef makeSwapchain(vk::SurfaceKHR surface);

	VulkanShaderRef makeShader(const char * vertPath,
		const char * fragPath,
		vector<VulkanVertexLayoutRef>&& vertexLayouts,
		vector<VulkanSetLayoutRef>&& setLayouts = {});

	VulkanShaderRef makeTessShader(
		const char * vertPath,
		const char * tessControlPath,
		const char * tessEvalPath,
		const char * tessGeomPath,
		const char * fragPath,
		vector<VulkanVertexLayoutRef>&& vertexLayouts = {},
		vector<VulkanSetLayoutRef>&& setLayouts = {}
	);

	VulkanShaderRef makeComputeShader(
		const char * computePath,
		vector<VulkanSetLayoutRef>&& setLayouts = {}
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

	VulkanSetLayoutRef makeSetLayout(temps<VulkanUniformLayoutBinding> bindings);

	VulkanSetLayoutRef makeSetLayout(vector<VulkanUniformLayoutBinding> & bindings);

	VulkanSetRef makeSet(
		VulkanSetLayoutRef layout
	);

	VulkanSetRef makeSet(
		temps<VulkanUniformLayoutBinding> && bindings
	);

	VulkanSetRef makeSet(
		vector<VulkanUniformLayoutBinding> & bindings
	);

	VulkanTaskRef makeTask(uint32_t poolIndex = 0, bool autoReset = false);
	VulkanTaskGroupRef makeTaskGroup(uint32_t numTasks, uint32_t poolIndex = 0);

	VulkanImageRef makeImage(vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format);
	VulkanImageRef makeImage(vk::Image image, glm::ivec2 size, vk::Format format);
	
	template <class T>
	VulkanImageRef makeImageTyped(vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format) {
		auto r = make_shared<T>(this, usage, size, format);
		r->createImage();
		r->setSampler(getNearestSampler());
		return r;
	};

	template <class T>
	shared_ptr<ubo<T>> makeUBO(uint32_t arrayCount, T * data = nullptr) {
		return make_shared<ubo<T>>(this, arrayCount, data);
	}

	template <class T>
	shared_ptr<static_vbo<T>> makeVBO(temps<vk::Format> fieldFormats, uint32_t arrayCount, T * data = nullptr) {
		return make_shared<static_vbo<T>>(this, std::move(fieldFormats), arrayCount, data);
	}

	template <class T>
	shared_ptr<ssbo<T>> makeSSBO(uint32_t arrayCount) {
		return make_shared<ssbo<T>>(this, arrayCount);
	}

	shared_ptr<ibo> makeIBO(vk::ArrayProxy<const uint16_t> indices);
	
	vk::Sampler getLinearSampler();
	vk::Sampler getNearestSampler();
	vk::Sampler getShadowSampler();


	~VulkanContext();

private:

	uint32_t _familyIndex;

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

