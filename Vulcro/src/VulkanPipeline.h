#pragma once

#include "VulkanContext.h"
#include "VulkanRenderer.h"
#include "VulkanSet.h"

class VulkanRenderPipeline
{
public:

	//////////////////////////
	//// Constructors / Descructor
	/////////////////////////

	VulkanRenderPipeline(
		VulkanContextRef ctx, 
		VulkanShaderRef shader, 
		VulkanRendererRef renderer, 
		PipelineConfig config = PipelineConfig(), 
		vector<ColorBlendConfig> colorBlendConfigs = {}
    );

	~VulkanRenderPipeline();

	//////////////////////////
	//// Functions
	/////////////////////////

	void bind(vk::CommandBuffer * cmd);

	void bindSets(vk::CommandBuffer * cmd, vk::ArrayProxy<const VulkanSetRef> sets);

	//////////////////////////
	//// Getters /Setters
	/////////////////////////

	inline vk::Pipeline getPipeline()
	{
		return _pipeline;
	}

	inline vk::PipelineLayout getLayout()
	{
		return _pipelineLayout;
	}

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

	void bindSets(vk::CommandBuffer * cmd, vk::ArrayProxy<const VulkanSetRef> sets);

	~VulkanComputePipeline();

private:
	vk::DescriptorSet _descriptorSets[16];

	VulkanShaderRef  _shader;
	VulkanContextRef _ctx;
	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
};