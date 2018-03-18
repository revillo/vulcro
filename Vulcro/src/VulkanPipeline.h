#pragma once
#include "VulkanContext.h"
#include "VulkanRenderer.h"

class VulkanPipeline {

public:
	VulkanPipeline(VulkanContextRef ctx, VulkanShaderRef shader, VulkanRendererRef renderer);

	vk::Pipeline getPipeline() {
		return _pipeline;
	}

	vk::PipelineLayout getLayout() {
		return _pipelineLayout;
	}

	~VulkanPipeline();

private:

	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;


	VulkanShaderRef  _shader;
	VulkanContextRef _ctx;
	VulkanRendererRef _renderer;
};

typedef shared_ptr<VulkanPipeline> VulkanPipelineRef;