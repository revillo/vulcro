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
				binding.arrayCount
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
			vk::DescriptorPoolCreateFlags(),
			10, // todo
			poolSizes.size(),
			&poolSizes[0]
		)
	);

}

vk::DescriptorSet VulkanUniformLayout::allocateSet() {

	return _ctx->getDevice().allocateDescriptorSets(
		vk::DescriptorSetAllocateInfo(
			_pool,
			1,
			&_descriptorLayout
		)
	)[0];

}

VulkanUniformLayout::~VulkanUniformLayout()
{
	_ctx->getDevice().destroyDescriptorPool(_pool);
}
