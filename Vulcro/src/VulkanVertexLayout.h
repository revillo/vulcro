#pragma once

#include "General.h"
#include "vulkan\vulkan.hpp"

class VulkanVertexLayout
{
public:
	VulkanVertexLayout(temps<vk::Format> fields);

	vk::VertexInputBindingDescription getVIBD(uint32 binding);
	vector<vk::VertexInputAttributeDescription> getVIADS(uint32 binding);

	~VulkanVertexLayout();

private:

	uint32 _size = 0;
	//vk::VertexInputBindingDescription _vibd;
	//vector<vk::VertexInputAttributeDescription> _viad;

	vector<vk::Format> _fields;
};

