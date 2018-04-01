#pragma once
#include "VulkanContext.h"
#include "VulkanUniformSetLayout.h"
#include "VulkanVertexLayout.h"

class VulkanShader
{

public:

	VulkanShader(
		VulkanContextRef ctx,
		const char * vertPath,
		const char * fragPath,
		vector<VulkanVertexLayoutRef> vertexLayouts = {},
		vector<VulkanUniformSetLayoutRef> uniformLayouts = {}
	);

	VulkanShader(
		VulkanContextRef ctx,
		const char * vertPath,
		const char * tessControlPath,
		const char * tessEvalPath,
		const char * tessGeomPath,
		const char * fragPath,
		vector<VulkanVertexLayoutRef> vertexLayouts = {},
		vector<VulkanUniformSetLayoutRef> uniformLayouts = {}
	);

	VulkanShader(
		VulkanContextRef ctx,
		const char * computePath,
		vector<VulkanUniformSetLayoutRef> uniformLayouts = {}
	);
		
	~VulkanShader();

	vector<vk::PipelineShaderStageCreateInfo> getStages() {
		return _stages;
	}

	vk::PipelineVertexInputStateCreateInfo getVIS() {
		return _vis;
	}

	vector<VulkanUniformSetLayoutRef> &getUniformLayouts() {
		return _uniformLayouts;
	}

	vector<vk::DescriptorSetLayout> &getDescriptorSetLayouts() {
		return _descriptorSetLayouts;
	}

private:

	vk::ShaderModule loadModule(const char* path);

	vector<VulkanUniformSetLayoutRef> _uniformLayouts;
	vector<VulkanVertexLayoutRef> _vertexLayouts;

	vk::PipelineVertexInputStateCreateInfo _vis;
	vector<vk::VertexInputBindingDescription> _vibds;
	vector<vk::VertexInputAttributeDescription> _viads;

	vector<vk::DescriptorSetLayout> _descriptorSetLayouts;

	VulkanContextRef _ctx;
	vector<vk::PipelineShaderStageCreateInfo> _stages;
	vector<vk::ShaderModule> _modules;

	bool layoutCreated = false;
};

