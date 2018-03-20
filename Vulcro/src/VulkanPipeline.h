#pragma once
#include "VulkanContext.h"
#include "VulkanRenderer.h"


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

	~VulkanPipeline();

private:

    vector<vk::PipelineColorBlendAttachmentState> configureBlending();
    vk::PipelineDepthStencilStateCreateInfo configureDepthTest();


	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;


	VulkanShaderRef  _shader;
	VulkanContextRef _ctx;
	VulkanRendererRef _renderer;
};

