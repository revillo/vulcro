#pragma once

#include "VulkanContext.h"

class VulkanUniformLayout;

class VulkanUniformSet
{
public:
	
	VulkanUniformSet(VulkanContextRef ctx, VulkanUniformLayout * layout);

	void bindBuffer(uint32 binding, vk::DescriptorBufferInfo dbi);

	void update();

	void bind(vk::CommandBuffer &cmd, vk::PipelineLayout &pipelineLayout);

	~VulkanUniformSet();

private:

	VulkanContextRef _ctx;

	VulkanUniformLayout * _layout;

	vector<vk::WriteDescriptorSet> _writes;

	vk::DescriptorSet _descriptorSet;
};

typedef shared_ptr<VulkanUniformSet> VulkanUniformSetRef;