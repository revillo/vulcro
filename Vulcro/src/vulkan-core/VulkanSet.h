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
	
	VulkanSet(VulkanContextPtr ctx, VulkanSetLayoutRef layout);

	~VulkanSet();

	//////////////////////////
	//// Functions
	/////////////////////////


	void bindBuffer(uint32_t binding, VulkanBufferRef buffer);

	void bindBuffer(uint32_t binding, issboRef buffer) {
        if (!buffer) return;

		bindBuffer(binding, buffer->getBuffer());
	}

	void bindBuffer(uint32_t binding, vk::DescriptorBufferInfo dbi, vk::DescriptorType type = vk::DescriptorType::eUniformBuffer);

	void bindBufferArray(uint32_t binding, vk::ArrayProxy<VulkanBufferRef> buffers);

	void bindBuffer(uint32_t binding, iuboRef uboref);

	void bindImage(uint32_t binding, VulkanImageRef image, vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler, uint16_t mipLevel = 0);

	void bindImageArray(uint32_t binding, vk::ArrayProxy<VulkanImageRef> images, vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler);

	void bindStorageImage(uint32_t binding, VulkanImageRef image, uint16_t mipLevel = 0) {
		bindImage(binding, image, vk::DescriptorType::eStorageImage, mipLevel);
	}

	void bindRTScene(uint32_t binding, RTSceneRef rtscene);

    void bindTopStructure(uint32_t binding, RTTopStructureRef topStructure);

	void update();


    //////////////////////////
    //// Getters / Setters
    /////////////////////////

	vk::DescriptorSet getDescriptorSet() const {
		return _descriptorSet;
	}

	VulkanSetLayoutRef getLayout() const {
		return _layout;
	}


private:

	vector<vk::DescriptorBufferInfo> _dbis;

	vector<vk::DescriptorImageInfo> _diis;

	VulkanContextPtr _ctx;

	VulkanSetLayoutRef _layout;

	vector<vk::WriteDescriptorSet> _writes;

	vk::DescriptorSet _descriptorSet;
};