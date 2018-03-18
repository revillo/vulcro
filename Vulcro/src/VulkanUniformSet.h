#pragma once

#include "VulkanContext.h"
#include "VulkanUniformLayout.h"

class VulkanUniformSet
{
public:
	
	VulkanUniformSet(VulkanContextRef ctx, VulkanUniformLayoutRef layout);

	void bindBuffer(uint32 binding, vk::DescriptorBufferInfo dbi);

	void update();

	void bind(vk::CommandBuffer &cmd, vk::PipelineLayout &pipelineLayout);

	~VulkanUniformSet();

private:

	VulkanContextRef _ctx;

	VulkanUniformLayoutRef _layout;

	vector<vk::WriteDescriptorSet> _writes;

	vk::DescriptorSet _descriptorSet;
};

typedef shared_ptr<VulkanUniformSet> VulkanUniformSetRef;