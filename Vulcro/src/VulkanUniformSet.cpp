#include "VulkanUniformSet.h"

#include "VulkanUniformSetLayout.h"

VulkanUniformSet::VulkanUniformSet(VulkanContextRef ctx, VulkanUniformSetLayoutRef layout) :
	_ctx(ctx),
	_layout(layout)
{
	_descriptorSet = layout->allocateDescriptorSet();
}

void VulkanUniformSet::bindBuffer(uint32 binding, vk::DescriptorBufferInfo dbi)
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
		vk::DescriptorType::eUniformBuffer,
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

void VulkanUniformSet::bindImage(uint32 binding, VulkanImageRef image)
{

	auto dii = image->getDII();

	auto write = vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		vk::DescriptorType::eCombinedImageSampler,
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

void VulkanUniformSet::update() {

	if (_writes.size() == 0) return;

	_ctx->getDevice().updateDescriptorSets(
		_writes.size(),
		&_writes[0],
		0,
		nullptr
	);

	_writes.clear();

}

void VulkanUniformSet::bind(vk::CommandBuffer * cmd, vk::PipelineLayout &pipelineLayout)
{
	cmd->bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout,
		0,
		1,
		&_descriptorSet,
		0,
		nullptr
	);
}


VulkanUniformSet::~VulkanUniformSet()
{

	_layout->freeDescriptorSet(_descriptorSet);

}
