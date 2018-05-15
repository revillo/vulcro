#pragma once

#include "vulkan\vulkan.hpp"
#include "General.h"
#include "VulkanContext.h"


class VulkanImage
{
public:

	static vk::ImageUsageFlags SAMPLED_STORAGE;
	static vk::ImageUsageFlags SAMPLED_COLOR_ATTACHMENT;

	VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format);
	
	VulkanImage(VulkanContextRef ctx, vk::Image image, glm::ivec2 size, vk::Format format);

	virtual void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
	virtual void createSampler();
	void setSampler(vk::Sampler sampler) { _sampler = sampler; }


	vk::DescriptorImageInfo getDII();
	vk::DescriptorType getDescriptorType();

	void allocateDeviceMemory(vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::Format getFormat() {
		return _format;
	}

	~VulkanImage();

	vk::Image deviceImage() {
		return _image;
	}

	vk::Sampler getSampler() {
		return _sampler;
	}
	
	void resize(ivec2 size);

	const ivec2 getSize() {
		return _size;
	}

	vk::Rect2D getFullRect() {
		return vk::Rect2D(
			vk::Offset2D(0, 0),
			vk::Extent2D(_size.x, _size.y)
		);
	}

	vk::ImageView getImageView() {
		return _imageView;
	}

	virtual void createImage();

protected:

	VulkanImage() {};


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
	glm::ivec2 _size;
};

class VulkanCubeImage : public VulkanImage {

public:

	VulkanCubeImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format);

	
	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor) override;
	
	void createImage() override;


};