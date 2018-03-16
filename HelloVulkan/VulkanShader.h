#pragma once
#include "VulkanContext.h"
#include "VulkanUniformLayout.h"

class VulkanShader
{

public:

	VulkanShader(VulkanContextRef ctx, const char * vertPath, const char * fragPath, vector<VulkanUniformLayoutRef> layouts = {});
		
	~VulkanShader();

	vector<vk::PipelineShaderStageCreateInfo> getStages() {
		return _stages;
	}

	vector<VulkanUniformLayoutRef> &getLayouts() {
		return _layouts;
	}

	vector<vk::DescriptorSetLayout> &getDescriptorSetLayouts() {
		return _descriptorSetLayouts;
	}

private:

	vector<VulkanUniformLayoutRef> _layouts;

	vector<vk::DescriptorSetLayout> _descriptorSetLayouts;

	VulkanContextRef _ctx;
	vector<vk::PipelineShaderStageCreateInfo> _stages;
	vk::ShaderModule _vertModule, _fragModule;

	bool layoutCreated = false;
};

typedef shared_ptr<VulkanShader> VulkanShaderRef;