#pragma once

#include "General.h"
#include "VulkanContext.h"
#include "VulkanImage.h"

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
	void createImage() override;

	//////////////////////////
	//// Getters / Setters
	/////////////////////////

	inline const ivec2 getSize()
	{
		return _size;
	}

	inline vk::Rect2D getFullRect()
	{
		return vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(_size.x, _size.y));
	}

private:
	glm::ivec2 _size;
};