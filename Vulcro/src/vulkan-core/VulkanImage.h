#pragma once

#include "vulkan/vulkan.hpp"
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

	VulkanImage(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, glm::uvec3 size, vk::ImageType imageType);
	
	VulkanImage(VulkanContextPtr ctx, vk::Image image, vk::Format format, glm::uvec3 size, vk::ImageType imageType);

	~VulkanImage();

	//////////////////////////
	//// Functions
	/////////////////////////

	virtual void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) = 0;
	virtual void createImage();

	void setSampler(vk::Sampler sampler) { mSampler = sampler; }

	vk::DescriptorImageInfo getDII(uint16_t mipLevel = 0);
	vk::DescriptorType getDescriptorType();

	void transitionLayout(vk::CommandBuffer * cmd, vk::ImageLayout layout = vk::ImageLayout::eGeneral);

    //Warning: Only use for host visible images. Use VulkanImage2D::loadFromMemory instead. 
	void upload(uint64_t size, void* data);

	//////////////////////////
	//// Getters / Setters
	/////////////////////////

	inline bool isMemoryMapped() {
		return mMemoryMapping != nullptr;
	}

	inline void* getMemoryMapping() {
		return mMemoryMapping;
	}

	inline vk::Format getFormat() {
		return mFormat;
	}

	inline vk::Sampler &getSampler() {
		return mSampler;
	}

	inline vk::ImageType getImageType() {
		return mImageType;
	}

	inline vk::ImageView getImageView() {
		return mImageView;
	}

	inline vk::Image getImage() {
		return mImage;
	}

	inline uvec3 getSize() {
		return mSize;
	}

	inline uint64_t getMemorySize() {
		return mMemorySize;
	}

protected:
	void allocateDeviceMemory(vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal);

	VulkanContextPtr mContext;

	void *mMemoryMapping = nullptr;
	vk::Format mFormat;
	vk::Sampler mSampler = nullptr;
	vk::ImageType mImageType;
	vk::ImageView mImageView = nullptr;
    std::vector<vk::ImageView> mMipViews;

    vk::ImageUsageFlags mUsage;
	vk::Image mImage = nullptr;
	glm::uvec3 mSize;

	vk::DeviceMemory mMemory = nullptr;
	uint64_t mMemorySize;
	uint16_t mMipLevels = 1;
	
	bool mImageCreated = false;
	bool mMemoryAllocated = false;
    bool mViewCreated = false;
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

	VulkanImage1D(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, float size);

	VulkanImage1D(VulkanContextPtr ctx, vk::Image image, vk::Format format, int size);

	//////////////////////////
	//// Functions
	/////////////////////////

	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) override;

	void resize(uint64_t size);
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

	VulkanImage2D(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, glm::uvec2 size, vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal);

	VulkanImage2D(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, glm::uvec2 size, uint16_t mipLevels, vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal);

	VulkanImage2D(VulkanContextPtr ctx, vk::Image image, vk::Format format, glm::uvec2 size);

	//////////////////////////
	//// Functions
	/////////////////////////
	
    //Generate mipmaps now or optionally provide command buffer to submit later.
	void generateMipmaps(vk::CommandBuffer * cmd = nullptr);

    void loadFromBuffer(VulkanBufferRef stagingBuffer, vk::CommandBuffer * cmd = nullptr);

    void loadFromMemory(void * pixelData, uint64_t sizeBytes, VulkanBufferRef stagingBuffer = nullptr, vk::CommandBuffer * cmd = nullptr);
         
	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) override;

	void resize(uvec2 size);

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

	VulkanImage3D(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, glm::uvec3 size);

	VulkanImage3D(VulkanContextPtr ctx, vk::Image image, vk::Format format, glm::uvec3 size);

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

	VulkanImageCube(VulkanContextPtr ctx, vk::ImageUsageFlags usage, glm::uvec2 size, vk::Format format);

	//////////////////////////
	//// Functions
	/////////////////////////

	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) override;

	void createImage() override;
};