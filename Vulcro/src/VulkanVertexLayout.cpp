#include "VulkanVertexLayout.h"

uint32 getSizeOf(vk::Format format) {
	switch (format) {
		case (vk::Format::eR32G32B32A32Sfloat) :
			return 16;
		case (vk::Format::eR32G32B32Sfloat) :
			return 12;
		case (vk::Format::eR32G32Sfloat) :
			return 8;
	}

	cout << "VertexLayout Format unrecognized";
	return 0;
}

VulkanVertexLayout::VulkanVertexLayout(vector<vk::Format> fields) :
	_fields(fields)
{

	for (auto &field : fields) {

		_size += getSizeOf(field);

	}

}

vk::VertexInputBindingDescription VulkanVertexLayout::getVIBD(uint32 binding)
{
	return vk::VertexInputBindingDescription(
		binding,
		_size,
		vk::VertexInputRate::eVertex
	);
}

vector<vk::VertexInputAttributeDescription> VulkanVertexLayout::getVIADS(uint32 binding)
{
	vector<vk::VertexInputAttributeDescription> viads;

	uint32 location = 0;
	uint32 offset = 0;
	for (auto &field : _fields) {
		viads.push_back(
			vk::VertexInputAttributeDescription(
				location++,
				binding,
				field,
				offset
			)
		);

		offset += getSizeOf(field);
	}

	return viads;
}

VulkanVertexLayout::~VulkanVertexLayout()
{
}
