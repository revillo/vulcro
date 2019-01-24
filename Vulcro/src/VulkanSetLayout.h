#pragma once

#include "VulkanContext.h"


class VulkanSetLayout
{
	typedef VulkanUniformLayoutBinding Binding;

public:

	VulkanSetLayout(VulkanContextRef ctx, vk::ArrayProxy<const Binding> bindings);

	~VulkanSetLayout();

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


