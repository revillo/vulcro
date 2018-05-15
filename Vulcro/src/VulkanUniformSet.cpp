#include "VulkanUniformSet.h"

#include "VulkanUniformSetLayout.h"

VulkanSet::VulkanSet(VulkanContextRef ctx, VulkanSetLayoutRef layout) :
	_ctx(ctx),
	_layout(layout)
{
	_descriptorSet = layout->allocateDescriptorSet();
}

void VulkanSet::bindBuffer(uint32 binding, VulkanBufferRef buffer)
{

	auto dbi = buffer->getDBI();

	auto write = vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		buffer->getDescriptorType(),
		nullptr,
		&dbi,
		nullptr
	);

	_ctx->getDevice().updateDescriptorSets(
		1,
		&write,
		0,
		nullptr
	);
}

void VulkanSet::bindBuffer(uint32 binding, vk::DescriptorBufferInfo dbi, vk::DescriptorType type)
{
	/*
	if (_dbis.size() < binding + 1) {
		_dbis.resize(binding + 1);
	}

	if (_dbis[binding] == dbi) {
		return;
	}
	else {
		//TODO check writes for this dbi
		_dbis[binding] = dbi;
	}

	_writes.push_back(vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		vk::DescriptorType::eUniformBuffer,
		nullptr,
		&_dbis[binding],
		nullptr

	));
	*/

	auto write = vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		type,
		nullptr,
		&dbi,
		nullptr

	);


	_ctx->getDevice().updateDescriptorSets(
		1,
		&write,
		0,
		nullptr
	);
}

void VulkanSet::bindImage(uint32 binding, VulkanImageRef image, vk::DescriptorType type)
{

	auto dii = image->getDII();

	auto write = vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		type,
		&dii,
		nullptr,
		nullptr

	);

	_ctx->getDevice().updateDescriptorSets(
		1,
		&write,
		0,
		nullptr
	);

}


void VulkanSet::bindImages(vector<VulkanImageRef> images, vk::DescriptorType type)
{
	uint32 binding = 0;
	for (auto &img : images) {
		bindImage(binding++, img, type);
	}
}

void VulkanSet::update() {

	if (_writes.size() == 0) return;

	_ctx->getDevice().updateDescriptorSets(
		static_cast<uint32>(_writes.size()),
		&_writes[0],
		0,
		nullptr
	);

	_writes.clear();

}

VulkanSet::~VulkanSet()
{

	_layout->freeDescriptorSet(_descriptorSet);

}
