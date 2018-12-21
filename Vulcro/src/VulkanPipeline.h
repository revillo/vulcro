#pragma once
#include "VulkanContext.h"
#include "VulkanRenderer.h"
#include "VulkanSet.h"

class VulkanPipeline {

public:
	VulkanPipeline(
		VulkanContextRef ctx, 
		VulkanShaderRef shader, 
		VulkanRendererRef renderer, 
		PipelineConfig config = PipelineConfig(), 
		vector<ColorBlendConfig> colorBlendConfigs = {}
    );

	vk::Pipeline getPipeline() {
		return _pipeline;
	}

	vk::PipelineLayout getLayout() {
		return _pipelineLayout;
	}

	void bind(vk::CommandBuffer * cmd);

	void bindUniformSets(vk::CommandBuffer * cmd, temps<VulkanSetRef> sets);
	void bindUniformSets(vk::CommandBuffer * cmd, vector<VulkanSetRef>& sets);
	void bindUniformSets(vk::CommandBuffer * cmd, const VulkanSetRef * sets, uint32_t numSets);

	~VulkanPipeline();

private:

    vector<vk::PipelineColorBlendAttachmentState> configureBlending();
    vk::PipelineDepthStencilStateCreateInfo configureDepthTest();


	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;

	vk::DescriptorSet _descriptorSets[16];

	VulkanShaderRef  _shader;
	VulkanContextRef _ctx;
	VulkanRendererRef _renderer;
};

class VulkanComputePipeline {

public:

	VulkanComputePipeline(VulkanContextRef ctx, VulkanShaderRef shader);

	void bind(vk::CommandBuffer * cmd);

	void bindUniformSets(vk::CommandBuffer * cmd, temps<VulkanSetRef> sets);
	void bindUniformSets(vk::CommandBuffer * cmd, vector<VulkanSetRef>& sets);
	void bindUniformSets(vk::CommandBuffer * cmd, const VulkanSetRef * sets, uint32_t numSets);

	~VulkanComputePipeline();

private:
	vk::DescriptorSet _descriptorSets[16];

	VulkanShaderRef  _shader;
	VulkanContextRef _ctx;
	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
};