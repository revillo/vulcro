#pragma once

#include "VulkanContext.h"
#include "VulkanUniformSetLayout.h"
#include "VulkanImage.h"

class VulkanUniformSet
{
public:
	
	VulkanUniformSet(VulkanContextRef ctx, VulkanUniformSetLayoutRef layout);

	void bindBuffer(uint32 binding, vk::DescriptorBufferInfo dbi);

	void bindImage(uint32 binding, VulkanImageRef image);

	void update();

	void bind(vk::CommandBuffer * cmd, vk::PipelineLayout &pipelineLayout);

	~VulkanUniformSet();

private:

	vector<vk::DescriptorBufferInfo> _dbis;

	vector<vk::DescriptorImageInfo> _diis;

	VulkanContextRef _ctx;

	VulkanUniformSetLayoutRef _layout;

	vector<vk::WriteDescriptorSet> _writes;

	vk::DescriptorSet _descriptorSet;
};