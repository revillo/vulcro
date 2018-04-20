#pragma once

#include "VulkanContext.h"


class VulkanUniformSetLayout
{
	typedef VulkanUniformLayoutBinding Binding;

public:

	VulkanUniformSetLayout(VulkanContextRef ctx, vector<Binding> bindings);

	~VulkanUniformSetLayout();

	vk::DescriptorSetLayout getDescriptorLayout() {
		return _descriptorLayout;
	}

	vk::DescriptorSet allocateDescriptorSet();

	void freeDescriptorSet(vk::DescriptorSet set);

	VulkanContextRef getContext() {
		return _ctx;
	}

private:

	vk::DescriptorSetLayout _descriptorLayout;
	vk::DescriptorPool _pool;

	VulkanContextRef _ctx;
	vector<Binding> _bindings;

};


