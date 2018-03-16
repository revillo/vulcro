#pragma once

#include "VulkanContext.h"

class VulkanUniformLayout
{


public:

	struct Binding {
		vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;
		uint32 arrayCount = 1;
		vk::Sampler * samplers = nullptr;
	};


	VulkanUniformLayout(VulkanContextRef ctx, vector<Binding> bindings);

	~VulkanUniformLayout();

	vk::DescriptorSetLayout getDescriptorLayout() {
		return _descriptorLayout;
	}

	vk::DescriptorSet allocateSet();

private:

	vk::DescriptorSetLayout _descriptorLayout;
	vk::DescriptorPool _pool;


	VulkanContextRef _ctx;
	vector<Binding> _bindings;

};

typedef shared_ptr<VulkanUniformLayout> VulkanUniformLayoutRef;

