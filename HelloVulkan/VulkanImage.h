#pragma once

#include "vulkan\vulkan.hpp"
#include "General.h"
#include "VulkanContext.h"


class VulkanImage
{
public:
	VulkanImage(VulkanContextRef ctx, glm::ivec2 size, vk::Format format, vk::ImageUsageFlagBits usage);
	~VulkanImage();

	vk::Image deviceImage() {
		return _image;
	}

private:

	VulkanContextRef _ctx;

	vk::Image _image;
	glm::ivec2 size;
};


typedef shared_ptr<VulkanImage> VulkanImageRef;
