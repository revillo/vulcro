#pragma once

#include "General.h"
#include "vulkan\vulkan.hpp"

class VulkanVertexLayout
{
public:
	VulkanVertexLayout(vk::ArrayProxy<const vk::Format> fields);

	vk::VertexInputBindingDescription getVIBD(uint32_t binding);
	vector<vk::VertexInputAttributeDescription> getVIADS(uint32_t binding);

	const vector<vk::Format> getFields() {
		return _fields;
	}

	const uint32_t getSize() {
		return _size;
	}

	~VulkanVertexLayout();

private:

	uint32_t _size = 0;
	//vk::VertexInputBindingDescription _vibd;
	//vector<vk::VertexInputAttributeDescription> _viad;

	vector<vk::Format> _fields;
};

