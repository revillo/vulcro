#include "VulkanUniformSetLayout.h"

#define TEMP_SET_MAX 1

VulkanSetLayout::VulkanSetLayout(VulkanContextRef ctx, vector<Binding> bindings) :
	_ctx(ctx),
	_bindings(bindings)
{

	vector<vk::DescriptorSetLayoutBinding> vkbindings;
	vector<vk::DescriptorPoolSize> poolSizes;


	for (auto &binding : bindings) {
		vkbindings.push_back(vk::DescriptorSetLayoutBinding(
			static_cast<uint32>(vkbindings.size()),
			binding.type,
			binding.arrayCount,
			vk::ShaderStageFlagBits::eAll,
			binding.samplers
		));

		poolSizes.push_back(
			vk::DescriptorPoolSize(
				binding.type,
				TEMP_SET_MAX
			)
		);
	}

	_descriptorLayout = _ctx->getDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(),
			static_cast<uint32>(vkbindings.size()),
			&vkbindings[0]
		)
	);


	_pool = _ctx->getDevice().createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			TEMP_SET_MAX, // todo
			static_cast<uint32>(poolSizes.size()),
			&poolSizes[0]
		)
	);

}

vk::DescriptorSet VulkanSetLayout::allocateDescriptorSet() {

	auto set = _ctx->getDevice().allocateDescriptorSets(
		vk::DescriptorSetAllocateInfo(
			_pool,
			1,
			&_descriptorLayout
		)
	)[0];

	return set;
}

void VulkanSetLayout::freeDescriptorSet(vk::DescriptorSet set)
{
	_ctx->getDevice().freeDescriptorSets(
		_pool,
		{ set }
	);

}

VulkanSetLayout::~VulkanSetLayout()
{
	_ctx->getDevice().destroyDescriptorSetLayout(_descriptorLayout);
	_ctx->getDevice().destroyDescriptorPool(_pool);
}