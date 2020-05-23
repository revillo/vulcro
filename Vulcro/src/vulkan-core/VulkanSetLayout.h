#pragma once

#include "VulkanContext.h"


class VulkanSetLayout
{
	typedef VulkanSetLayoutBinding Binding;

public:

	VulkanSetLayout(VulkanContextPtr ctx, vk::ArrayProxy<const Binding> bindings, uint32_t maxSets = 1);

	~VulkanSetLayout();

	vk::DescriptorSetLayout getDescriptorLayout() {
		return _descriptorLayout;
	}

	vk::DescriptorSet allocateDescriptorSet();

	void freeDescriptorSet(vk::DescriptorSet set);

	VulkanContextPtr getContext() {
		return _ctx;
	}

private:

	vk::DescriptorSetLayout _descriptorLayout;
	vk::DescriptorPool _pool;

	VulkanContextPtr _ctx;
	vector<Binding> _bindings;

};


