#include "VulkanVertexLayout.h"

uint32_t getSizeOf(vk::Format format) {
	switch (format) {
		case (vk::Format::eR32G32B32A32Sfloat) :
			return 16;
		case (vk::Format::eR32G32B32Sfloat) :
			return 12;
		case (vk::Format::eR32G32Sfloat) :
			return 8;
	}

	std::cout << "VertexLayout Format unrecognized";
	return 0;
}

VulkanVertexLayout::VulkanVertexLayout(temps<vk::Format> fields) :
	_fields(fields)
{

	for (auto &field : fields) {

		_size += getSizeOf(field);

	}

}

vk::VertexInputBindingDescription VulkanVertexLayout::getVIBD(uint32_t binding)
{
	return vk::VertexInputBindingDescription(
		binding,
		_size,
		vk::VertexInputRate::eVertex
	);
}

vector<vk::VertexInputAttributeDescription> VulkanVertexLayout::getVIADS(uint32_t binding)
{
	vector<vk::VertexInputAttributeDescription> viads;

	uint32_t location = 0;
	uint32_t offset = 0;
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
