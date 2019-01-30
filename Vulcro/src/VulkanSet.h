#pragma once

#include "VulkanContext.h"
#include "VulkanSetLayout.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"

class VulkanSet
{
public:

	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////
	
	VulkanSet(VulkanContextRef ctx, VulkanSetLayoutRef layout);

	~VulkanSet();

	//////////////////////////
	//// Functions
	/////////////////////////

	//////////////////////////
	//// Getters / Setters
	/////////////////////////

	void bindBuffer(uint32_t binding, VulkanBufferRef buffer);

	void bindBuffer(uint32_t binding, ssboRef buffer) {
		bindBuffer(binding, buffer->getBuffer());
	}

	void bindBuffer(uint32_t binding, vk::DescriptorBufferInfo dbi, vk::DescriptorType type = vk::DescriptorType::eUniformBuffer);

	void bindBuffer(uint32_t binding, uboRef uboref);

	void bindImage(uint32_t binding, VulkanImageRef image, vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler);

	void bindStorageImage(uint32_t binding, VulkanImageRef image) {
		bindImage(binding, image, vk::DescriptorType::eStorageImage);
	}

	void bindRTScene(uint32_t binding, RTSceneRef rtscene);

	void update();

	vk::DescriptorSet getDescriptorSet() const {
		return _descriptorSet;
	}

	VulkanSetLayoutRef getLayout() const {
		return _layout;
	}

	

private:

	vector<vk::DescriptorBufferInfo> _dbis;

	vector<vk::DescriptorImageInfo> _diis;

	VulkanContextRef _ctx;

	VulkanSetLayoutRef _layout;

	vector<vk::WriteDescriptorSet> _writes;

	vk::DescriptorSet _descriptorSet;
};