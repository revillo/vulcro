#pragma once

#include "VulkanContext.h"
#include "VulkanUniformSetLayout.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"

class VulkanUniformSet
{
public:
	
	VulkanUniformSet(VulkanContextRef ctx, VulkanUniformSetLayoutRef layout);

	void bindBuffer(uint32 binding, VulkanBufferRef buffer);

	void bindBuffer(uint32 binding, ssboRef buffer) {
		bindBuffer(binding, buffer->getBuffer());
	}

	void bindBuffer(uint32 binding, vk::DescriptorBufferInfo dbi, vk::DescriptorType type = vk::DescriptorType::eUniformBuffer);

	void bindImage(uint32 binding, VulkanImageRef image, vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler);

	void bindStorageImage(uint32 binding, VulkanImageRef image) {
		bindImage(binding, image, vk::DescriptorType::eStorageImage);
	}


	void bindImages(vector<VulkanImageRef> _images, vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler);

	void update();

	vk::DescriptorSet &getDescriptorSet() {
		return _descriptorSet;
	}

	VulkanUniformSetLayoutRef getLayout() {
		return _layout;
	}

	~VulkanUniformSet();

private:

	vector<vk::DescriptorBufferInfo> _dbis;

	vector<vk::DescriptorImageInfo> _diis;

	VulkanContextRef _ctx;

	VulkanUniformSetLayoutRef _layout;

	vector<vk::WriteDescriptorSet> _writes;

	vk::DescriptorSet _descriptorSet;
};