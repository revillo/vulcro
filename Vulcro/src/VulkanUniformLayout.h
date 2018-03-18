#pragma once

#include "VulkanContext.h"


class VulkanUniformLayout
{
	typedef VulkanUniformLayoutBinding Binding;

public:

	VulkanUniformLayout(VulkanContextRef ctx, vector<Binding> bindings);

	~VulkanUniformLayout();

	vk::DescriptorSetLayout getDescriptorLayout() {
		return _descriptorLayout;
	}

	vk::DescriptorSet allocateDescriptorSet();

	void freeDescriptorSet(vk::DescriptorSet set);

private:

	vk::DescriptorSetLayout _descriptorLayout;
	vk::DescriptorPool _pool;

	VulkanContextRef _ctx;
	vector<Binding> _bindings;

};


