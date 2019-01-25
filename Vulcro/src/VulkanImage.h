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

	VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, glm::ivec3 size, vk::ImageType imageType);
	
	VulkanImage(VulkanContextRef ctx, vk::Image image, vk::Format format, glm::ivec3 size, vk::ImageType imageType);

	~VulkanImage();

	//////////////////////////
	//// Functions
	/////////////////////////

	virtual void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) = 0;
	void createImage();

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

	inline ivec3 getSize()
	{
		return mSize;
	}


protected:

	void *mMemoryMapping = nullptr;
	vk::ImageType mImageType;
	glm::ivec3 mSize;

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


/**************************************************
 * 1D
 * ************************************************/

class VulkanImage1D : public VulkanImage
{
public:

	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanImage1D(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, float size);

	VulkanImage1D(VulkanContextRef ctx, vk::Image image, vk::Format format, int size);

	//////////////////////////
	//// Functions
	/////////////////////////

	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) override;

	void resize(float size);
};


/**************************************************
 * 2D
 * ************************************************/

class VulkanImage2D : public VulkanImage
{
public:

	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanImage2D(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, glm::ivec2 size);

	VulkanImage2D(VulkanContextRef ctx, vk::Image image, vk::Format format, glm::ivec2 size);

	//////////////////////////
	//// Functions
	/////////////////////////

	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) override;

	void resize(ivec2 size);

	//////////////////////////
	//// Getters / Setters
	/////////////////////////

	inline vk::Rect2D getFullRect()
	{
		return vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(mSize.x, mSize.y));
	}
};

/**************************************************
 * 3D
 * ************************************************/

class VulkanImage3D : public VulkanImage
{
public:

	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanImage3D(VulkanContextRef ctx, vk::ImageUsageFlags usage, vk::Format format, glm::ivec3 size);

	VulkanImage3D(VulkanContextRef ctx, vk::Image image, vk::Format format, glm::ivec3 size);

	//////////////////////////
	//// Functions
	/////////////////////////

	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) override;

};

/**************************************************
 * Cube
 * ************************************************/

class VulkanImageCube : public VulkanImage
{
public:

	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanImageCube(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format);

	//////////////////////////
	//// Functions
	/////////////////////////

	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) override;
};