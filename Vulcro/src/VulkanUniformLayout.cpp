#include "VulkanUniformLayout.h"


VulkanUniformLayout::VulkanUniformLayout(VulkanContextRef ctx, vector<Binding> bindings) :
	_ctx(ctx),
	_bindings(bindings)
{

	vector<vk::DescriptorSetLayoutBinding> vkbindings;
	vector<vk::DescriptorPoolSize> poolSizes;


	for (auto &binding : bindings) {
		vkbindings.push_back(vk::DescriptorSetLayoutBinding(
			vkbindings.size(),
			binding.type,
			binding.arrayCount,
			vk::ShaderStageFlagBits::eAllGraphics,
			binding.samplers
		));

		poolSizes.push_back(
			vk::DescriptorPoolSize(
				binding.type,
				10
			)
		);
	}

	_descriptorLayout = _ctx->getDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(),
			vkbindings.size(),
			&vkbindings[0]
		)
	);


	_pool = _ctx->getDevice().createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			10, // todo
			poolSizes.size(),
			&poolSizes[0]
		)
	);

}

vk::DescriptorSet VulkanUniformLayout::allocateDescriptorSet() {

	auto set = _ctx->getDevice().allocateDescriptorSets(
		vk::DescriptorSetAllocateInfo(
			_pool,
			1,
			&_descriptorLayout
		)
	)[0];

	return set;
}

void VulkanUniformLayout::freeDescriptorSet(vk::DescriptorSet set)
{
	_ctx->getDevice().freeDescriptorSets(
		_pool,
		{ set }
	);

}

VulkanUniformLayout::~VulkanUniformLayout()
{
	_ctx->getDevice().destroyDescriptorPool(_pool);
}