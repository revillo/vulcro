#pragma once
#include "VulkanContext.h"
#include "VulkanSetLayout.h"
#include "VulkanVertexLayout.h"

class VulkanShader
{

public:

	VulkanShader(
		VulkanContextPtr ctx,
		const char * vertPath,
		const char * fragPath,
		vk::ArrayProxy<const VulkanVertexLayoutRef> vertexLayouts = {},
		vk::ArrayProxy<const VulkanSetLayoutRef> uniformLayouts = {}
	);

	VulkanShader(
		VulkanContextPtr ctx,
		const char * vertPath,
		const char * tessControlPath,
		const char * tessEvalPath,
		const char * tessGeomPath,
		const char * fragPath,
		vk::ArrayProxy<const VulkanVertexLayoutRef> vertexLayouts = {},
		vk::ArrayProxy<const VulkanSetLayoutRef> uniformLayouts = {}
	);

	VulkanShader(
		VulkanContextPtr ctx,
		const char * computePath,
		vk::ArrayProxy<const VulkanSetLayoutRef> uniformLayouts = {}
	);

	static vk::ShaderModule createModule(VulkanContextPtr ctx, const char * path);
		
	~VulkanShader();

	const vector<vk::PipelineShaderStageCreateInfo> & getStages() {
		return _stages;
	}

	vk::PipelineVertexInputStateCreateInfo getVIS() {
		return _vis;
	}

	const vector<VulkanSetLayoutRef> &getUniformLayouts() {
		return _uniformLayouts;
	}

	const vector<vk::DescriptorSetLayout> &getDescriptorSetLayouts() {
		return _descriptorSetLayouts;
	}

private:

	vk::ShaderModule loadModule(const char* path);

	vector<VulkanSetLayoutRef> _uniformLayouts;
	vector<VulkanVertexLayoutRef> _vertexLayouts;

	vk::PipelineVertexInputStateCreateInfo _vis;
	vector<vk::VertexInputBindingDescription> _vibds;
	vector<vk::VertexInputAttributeDescription> _viads;

	vector<vk::DescriptorSetLayout> _descriptorSetLayouts;

	VulkanContextPtr _ctx;
	vector<vk::PipelineShaderStageCreateInfo> _stages;
	vector<vk::ShaderModule> _modules;

	bool layoutCreated = false;
};

