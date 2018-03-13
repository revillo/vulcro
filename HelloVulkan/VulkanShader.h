#pragma once
#include "VulkanContext.h"

class VulkanShader
{
public:
	VulkanShader(VulkanContextRef ctx, const char * name, const char * vertPath, const char * fragPath);
	~VulkanShader();

	vector<vk::PipelineShaderStageCreateInfo> getStages() {
		return _stages;
	}


private:
	VulkanContextRef _ctx;
	vector<vk::PipelineShaderStageCreateInfo> _stages;
	vk::ShaderModule _vertModule, _fragModule;
};

typedef shared_ptr<VulkanShader> VulkanShaderRef;