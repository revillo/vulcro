#include "VulkanSetLayout.h"

VulkanSetLayout::VulkanSetLayout(VulkanContextPtr ctx, vk::ArrayProxy<const Binding> bindings, uint32_t maxSets) :
	_ctx(ctx),
	_bindings((SLB*)bindings.begin(), (SLB*)bindings.end())
{

	vector<vk::DescriptorSetLayoutBinding> vkbindings;
	vector<vk::DescriptorPoolSize> poolSizes;


	for (auto &binding : bindings) {
		vkbindings.push_back(vk::DescriptorSetLayoutBinding(
			static_cast<uint32_t>(vkbindings.size()),
			binding.type,
			binding.arrayCount,
			binding.stageFlags,
			binding.samplers
		));

		poolSizes.push_back(
			vk::DescriptorPoolSize(
				binding.type,
				binding.arrayCount
			)
		);
	}

	_descriptorLayout = _ctx->getDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(),
			static_cast<uint32_t>(vkbindings.size()),
			&vkbindings[0]
		), nullptr,
		_ctx->getDynamicDispatch()
	);

	_pool = _ctx->getDevice().createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			maxSets,
			static_cast<uint32_t>(poolSizes.size()),
			&poolSizes[0]
		), nullptr,
		_ctx->getDynamicDispatch()
	);

}

vk::DescriptorSet VulkanSetLayout::allocateDescriptorSet() {

	auto set = _ctx->getDevice().allocateDescriptorSets(
		vk::DescriptorSetAllocateInfo(
			_pool,
			1,
			&_descriptorLayout
		),
		_ctx->getDynamicDispatch()
	)[0];

	return set;
}

void VulkanSetLayout::freeDescriptorSet(vk::DescriptorSet set)
{
	_ctx->getDevice().freeDescriptorSets(
		_pool,
		{ set },
		_ctx->getDynamicDispatch()
	);

}

VulkanSetLayout::~VulkanSetLayout()
{
	_ctx->getDevice().destroyDescriptorSetLayout(_descriptorLayout, nullptr, _ctx->getDynamicDispatch());
	_ctx->getDevice().destroyDescriptorPool(_pool, nullptr, _ctx->getDynamicDispatch());
}