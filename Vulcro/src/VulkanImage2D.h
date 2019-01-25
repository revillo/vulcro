#pragma once

#include "General.h"
#include "VulkanContext.h"
#include "VulkanImage.h"

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
	void createImage() override;

	void resize(ivec2 size);

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