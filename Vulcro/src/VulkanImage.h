#pragma once

#include "vulkan\vulkan.hpp"
#include "General.h"
#include "VulkanContext.h"


class VulkanImage
{
public:

	//////////////////////////
	//// Constants
	/////////////////////////

	static vk::ImageUsageFlags SAMPLED_STORAGE;
	static vk::ImageUsageFlags SAMPLED_COLOR_ATTACHMENT;

	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format);
	
	VulkanImage(VulkanContextRef ctx, vk::Image image, vk::Format format);

	~VulkanImage();

	//////////////////////////
	//// Functions
	/////////////////////////

	virtual void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) = 0;
	virtual void createImage() = 0;


	void allocateDeviceMemory(vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal);

	void createSampler();
	void setSampler(vk::Sampler sampler) { _sampler = sampler; }

	vk::DescriptorImageInfo getDII();
	vk::DescriptorType getDescriptorType();

	void transitionLayout(vk::CommandBuffer * cmd, vk::ImageLayout layout = vk::ImageLayout::eGeneral);

	void upload(uint64_t size, void* data);

	//////////////////////////
	//// Getters / Setters
	/////////////////////////

	inline bool isMemoryMapped()
	{
		return mMemoryMapping != nullptr;
	}

	inline void* getMemoryMapping()
	{
		return mMemoryMapping;
	}

	inline vk::Format getFormat()
	{
		return _format;
	}

	inline vk::Sampler getSampler()
	{
		return _sampler;
	}

	inline vk::Image getImage()
	{
		return _image;
	}

	inline vk::ImageView getImageView()
	{
		return _imageView;
	}

protected:

	void *mMemoryMapping = nullptr;

	VulkanContextRef _ctx;

	vk::Format _format;
	vk::ImageView _imageView = nullptr;
	vk::DeviceMemory _memory;

	vk::Sampler _sampler = nullptr;

	bool _imageCreated = false;
	bool _memoryAllocated = false;
	bool _viewCreated = false;

	vk::ImageUsageFlags _usage;

	vk::Image _image;
	
	uint64_t _memorySize;
};