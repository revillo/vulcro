#pragma once

#include <vulkan/vulkan.hpp>
#include "General.h"

struct VulkanUniformLayoutBinding {

	VulkanUniformLayoutBinding(vk::DescriptorType _type = vk::DescriptorType::eUniformBuffer, uint32 _arrayCount = 1, vk::Sampler * _samplers = nullptr) :
		type(_type), 
		arrayCount(_arrayCount),
		samplers(_samplers)
	{

	}

	vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;
	uint32 arrayCount = 1;
	vk::Sampler * samplers = nullptr;
};

typedef VulkanUniformLayoutBinding VULB;

class VulkanUniformLayout;
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

typedef shared_ptr<VulkanShader> VulkanShaderRef;
typedef shared_ptr<VulkanRenderer> VulkanRendererRef;
typedef shared_ptr<VulkanPipeline> VulkanPipelineRef;
typedef shared_ptr<VulkanUniformLayout> VulkanUniformLayoutRef;
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

	VulkanUniformLayoutRef makeUniformLayout(vector<VulkanUniformLayoutBinding> bindings);
	VulkanVertexLayoutRef makeVertexLayout(vector<vk::Format> fields);
	VulkanRendererRef makeRenderer();
	VulkanPipelineRef makePipeline(VulkanShaderRef shader, VulkanRendererRef renderer);
	VulkanSwapchainRef makeSwapchain(vk::SurfaceKHR surface);

	VulkanShaderRef makeShader(const char * vertPath,
		const char * fragPath,
		vector<VulkanVertexLayoutRef> vertexLayouts,
		vector<VulkanUniformLayoutRef> uniformLayouts = {});

	VulkanBufferRef makeBuffer(
		vk::BufferUsageFlags usage,
		uint64 size,
		void* data
	);

	VulkanUniformSetRef makeUniformSet(
		VulkanUniformLayoutRef layout
	);

	VulkanTaskRef makeTask();

	VulkanImageRef makeImage(vk::ImageUsageFlagBits usage, glm::ivec2 size, vk::Format format);
	VulkanImageRef makeImage(vk::Image image, glm::ivec2 size, vk::Format format);

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