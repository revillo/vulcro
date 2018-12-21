#include "VulkanSet.h"

#include "VulkanSetLayout.h"

VulkanSet::VulkanSet(VulkanContextRef ctx, VulkanSetLayoutRef layout) :
	_ctx(ctx),
	_layout(layout)
{
	_descriptorSet = layout->allocateDescriptorSet();
}

void VulkanSet::bindBuffer(uint32_t binding, VulkanBufferRef buffer)
{

	auto type = buffer->getDescriptorType();

	vk::WriteDescriptorSet write;

	if ((type == vk::DescriptorType::eStorageTexelBuffer) || (type == vk::DescriptorType::eUniformTexelBuffer)) {

		auto view = buffer->getView();

		write = vk::WriteDescriptorSet(
			_descriptorSet,
			binding,
			0,
			1,
			type,
			nullptr,
			nullptr,
			&view
		);
	
		_ctx->getDevice().updateDescriptorSets(
			1,
			&write,
			0,
			nullptr
		);

	}
	else {
		auto dbi = buffer->getDBI();


		write = vk::WriteDescriptorSet(
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
}

void VulkanSet::bindBuffer(uint32_t binding, vk::DescriptorBufferInfo dbi, vk::DescriptorType type)
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


void VulkanSet::bindBuffer(uint32_t binding, uboRef uboref)
{
	bindBuffer(binding, uboref->getDBI());
}

void VulkanSet::bindImage(uint32_t binding, VulkanImageRef image, vk::DescriptorType type)
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
	uint32_t binding = 0;
	for (auto &img : images) {
		bindImage(binding++, img, type);
	}
}

void VulkanSet::update() {

	if (_writes.size() == 0) return;

	_ctx->getDevice().updateDescriptorSets(
		static_cast<uint32_t>(_writes.size()),
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
