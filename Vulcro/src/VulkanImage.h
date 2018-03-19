#pragma once

#include "vulkan\vulkan.hpp"
#include "General.h"
#include "VulkanContext.h"


class VulkanImage
{
public:
	VulkanImage(VulkanContextRef ctx, vk::ImageUsageFlags usage, glm::ivec2 size, vk::Format format);
	VulkanImage(VulkanContextRef ctx, vk::Image image, glm::ivec2 size, vk::Format format);

	void createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
	void createSampler();

	vk::DescriptorImageInfo getDII();

	void allocateDeviceMemory();

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


	const ivec2 getSize() {
		return _size;
	}

	vk::ImageView getImageView() {
		return _imageView;
	}

private:

	VulkanContextRef _ctx;

	vk::Format _format;
	vk::ImageView _imageView = nullptr;
	vk::DeviceMemory _memory;

	vk::Sampler _sampler = nullptr;

	bool _imageCreated = false;
	bool _memoryAllocated = false;
	bool _viewCreated = false;

	vk::Image _image;
	glm::ivec2 _size;
};