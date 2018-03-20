#pragma once

#include <vulkan/vulkan.hpp>
#include "General.h"


struct VulkanUniformLayoutBinding {

	VulkanUniformLayoutBinding(uint32 _arrayCount = 1, vk::DescriptorType _type = vk::DescriptorType::eUniformBuffer, vk::Sampler * _samplers = nullptr) :
		type(_type), 
		arrayCount(_arrayCount),
		samplers(_samplers)
	{

	}

	vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;
	uint32 arrayCount = 1;
	vk::Sampler * samplers = nullptr;
};

struct PipelineConfig {
	vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
};

struct ColorBlendConfig {

};


typedef VulkanUniformLayoutBinding ULB;

class VulkanUniformSetLayout;
class VulkanUniformSet;
class VulkanVertexLayout;
class VulkanRenderer;
class VulkanShader;
class VulkanPipeline;
class VulkanSwapchain;
class VulkanBuffer;
class VulkanUniformSet;
class VulkanTask;
class VulkanImage;

template <class T>
class ubo;

template <class T>
class vbo;

class ibo;

typedef shared_ptr<VulkanShader> VulkanShaderRef;
typedef shared_ptr<VulkanRenderer> VulkanRendererRef;
typedef shared_ptr<VulkanPipeline> VulkanPipelineRef;
typedef shared_ptr<VulkanUniformSetLayout> VulkanUniformSetLayoutRef;
typedef shared_ptr<VulkanVertexLayout> VulkanVertexLayoutRef;
typedef shared_ptr<VulkanSwapchain> VulkanSwapchainRef;
typedef shared_ptr<VulkanBuffer> VulkanBufferRef;
typedef shared_ptr<VulkanUniformSet> VulkanUniformSetRef;
typedef shared_ptr<VulkanTask> VulkanTaskRef;
typedef shared_ptr<VulkanImage> VulkanImageRef;




class VulkanContext
{
public:
	VulkanContext(vk::Instance instance);
	vk::Device &getDevice() { return _device; }
	vk::PhysicalDevice &getPhysicalDevice() { return _physicalDevices[0]; }

	vk::Queue &getQueue() {
		return _queue;
	}

	vk::CommandPool &getCommandPool() {
		return _commandPool;
	}

	void resetTasks() {

		_device.resetCommandPool(
			_commandPool,
			vk::CommandPoolResetFlags()
		);

	}

	VulkanUniformSetLayoutRef makeUniformSetLayout(vector<VulkanUniformLayoutBinding> bindings);
	VulkanVertexLayoutRef makeVertexLayout(vector<vk::Format> fields);
	VulkanRendererRef makeRenderer();

	VulkanPipelineRef makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer, PipelineConfig config = PipelineConfig(),
		vector<ColorBlendConfig> colorBlendConfigs = {}
	);

	VulkanSwapchainRef makeSwapchain(vk::SurfaceKHR surface);

	VulkanShaderRef makeShader(const char * vertPath,
		const char * fragPath,
		vector<VulkanVertexLayoutRef> vertexLayouts,
		vector<VulkanUniformSetLayoutRef> uniformLayouts = {});

	VulkanBufferRef makeBuffer(
		vk::BufferUsageFlags usage,
		uint64 size,
		void* data
	);

	VulkanUniformSetRef makeUniformSet(
		VulkanUniformSetLayoutRef layout
	);

	VulkanTaskRef makeTask();

	VulkanImageRef makeImage(vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format);
	VulkanImageRef makeImage(vk::Image image, glm::ivec2 size, vk::Format format);
	

	template <class T>
	shared_ptr<ubo<T>> makeUBO(uint32 arrayCount, T * data = nullptr) {
		return make_shared<ubo<T>>(this, arrayCount, data);
	}

	template <class T>
	shared_ptr<vbo<T>> makeVBO(vector<vk::Format> fieldFormats, uint32 arrayCount, T * data = nullptr) {
		return make_shared<vbo<T>>(this, fieldFormats, arrayCount, data);
	}

	shared_ptr<ibo> makeIBO(vector<uint16_t> indices);

	~VulkanContext();

private:

	vk::Instance _instance;
	vector<vk::PhysicalDevice> _physicalDevices;
	vk::Device _device;
	vk::CommandPool _commandPool;
	vk::CommandBuffer _cmd;
	vk::Queue _queue;


};

typedef VulkanContext * VulkanContextRef;

