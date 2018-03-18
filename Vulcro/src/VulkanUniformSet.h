#pragma once

#include "VulkanContext.h"
#include "VulkanUniformSetLayout.h"

class VulkanUniformSet
{
public:
	
	VulkanUniformSet(VulkanContextRef ctx, VulkanUniformSetLayoutRef layout);

	void bindBuffer(uint32 binding, vk::DescriptorBufferInfo dbi);

	void update();

	void bind(vk::CommandBuffer * cmd, vk::PipelineLayout &pipelineLayout);

	~VulkanUniformSet();

private:

	vector<vk::DescriptorBufferInfo> _dbis;

	VulkanContextRef _ctx;

	VulkanUniformSetLayoutRef _layout;

	vector<vk::WriteDescriptorSet> _writes;

	vk::DescriptorSet _descriptorSet;
};