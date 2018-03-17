#pragma once

#include "VulkanContext.h"
#include "VulkanUniformSet.h"



class VulkanUniformLayout
{
	typedef VulkanUniformLayoutBinding Binding;

public:

	VulkanUniformLayout(VulkanContextRef ctx, vector<Binding> bindings);

	~VulkanUniformLayout();

	vk::DescriptorSetLayout getDescriptorLayout() {
		return _descriptorLayout;
	}

	VulkanUniformSetRef createSet();


	vk::DescriptorSet allocateDescriptorSet();

	void freeDescriptorSet(vk::DescriptorSet set);

private:

	vk::DescriptorSetLayout _descriptorLayout;
	vk::DescriptorPool _pool;

	VulkanContextRef _ctx;
	vector<Binding> _bindings;

};

typedef shared_ptr<VulkanUniformLayout> VulkanUniformLayoutRef;

