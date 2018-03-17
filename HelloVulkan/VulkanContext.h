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

	shared_ptr<VulkanUniformLayout> makeUniformLayout(vector<VulkanUniformLayoutBinding> bindings);

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