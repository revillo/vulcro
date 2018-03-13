#pragma once

#include "vulkan\vulkan.hpp"
#include "General.h"
#include "VulkanContext.h"


class VulkanImage
{
public:
	VulkanImage(VulkanContextRef ctx, glm::ivec2 size, vk::Format format, vk::ImageUsageFlagBits usage);
	VulkanImage(VulkanContextRef ctx, vk::Image image, glm::ivec2 size, vk::Format format);

	void createImageView(vk::ImageAspectFlags aspectFlags);
	void allocateDeviceMemory();

	vk::Format getFormat() {
		return _format;
	}

	~VulkanImage();

	vk::Image deviceImage() {
		return _image;
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
	vk::ImageView _imageView;

	vk::Image _image;
	glm::ivec2 _size;
};


typedef shared_ptr<VulkanImage> VulkanImageRef;
