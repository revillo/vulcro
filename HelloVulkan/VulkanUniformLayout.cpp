#include "VulkanUniformLayout.h"





VulkanUniformLayout::VulkanUniformLayout(VulkanContextRef ctx, vector<Binding> bindings) :
	_ctx(ctx),
	_bindings(bindings)
{

	vector<vk::DescriptorSetLayoutBinding> vkbindings;

	for (auto &binding : bindings) {
		vkbindings.push_back(vk::DescriptorSetLayoutBinding(
			vkbindings.size(),
			binding.type,
			binding.arrayCount,
			vk::ShaderStageFlagBits::eAllGraphics,
			binding.samplers
		));
	}


	_descriptorLayout = _ctx->getDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(),
			vkbindings.size(),
			&vkbindings[0]
		)
	);
}

VulkanUniformLayout::~VulkanUniformLayout()
{
}
