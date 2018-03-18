#include "VulkanUniformSet.h"

#include "VulkanUniformLayout.h"

VulkanUniformSet::VulkanUniformSet(VulkanContextRef ctx, VulkanUniformLayoutRef layout) :
	_ctx(ctx),
	_layout(layout)
{
	_descriptorSet = layout->allocateDescriptorSet();
}

void VulkanUniformSet::bindBuffer(uint32 binding, vk::DescriptorBufferInfo dbi)
{

	_writes.push_back(vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		vk::DescriptorType::eUniformBuffer,
		nullptr,
		&dbi,
		nullptr

	));
}

void VulkanUniformSet::update() {

	_ctx->getDevice().updateDescriptorSets(
		_writes.size(),
		&_writes[0],
		0,
		nullptr
	);

	_writes.clear();

}

void VulkanUniformSet::bind(vk::CommandBuffer &cmd, vk::PipelineLayout &pipelineLayout)
{
	cmd.bindDescriptorSets(
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
