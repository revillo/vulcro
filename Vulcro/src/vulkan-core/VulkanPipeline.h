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
        VulkanContextPtr ctx,
        VulkanShaderRef shader,
        VulkanRendererRef renderer,
        PipelineConfig config = PipelineConfig(),
        vector<ColorBlendConfig> colorBlendConfigs = {},
        uint32_t pushConstantSize = 0
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

    vector<vk::PipelineColorBlendAttachmentState> configureBlending(const vector<ColorBlendConfig>  & colorBlendConfigs);
    vk::PipelineDepthStencilStateCreateInfo configureDepthTest();


	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;

	vk::DescriptorSet _descriptorSets[16];

	VulkanShaderRef  _shader;
	VulkanContextPtr _ctx;
	VulkanRendererRef _renderer;
};

class VulkanComputePipeline {

public:

	VulkanComputePipeline(VulkanContextPtr ctx, VulkanShaderRef shader, uint32_t pushConstantSize = 0);

	void bind(vk::CommandBuffer * cmd);

	void bindSets(vk::CommandBuffer * cmd, vk::ArrayProxy<const VulkanSetRef> sets);

    template <typename T>
    void pushConstants(vk::CommandBuffer * cmd, T & data)
    {
        cmd->pushConstants<T>(_pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, { data });
    }

	void dispatch(vk::CommandBuffer * cmd, uvec3 blocks) {
		cmd->dispatch(blocks.x, blocks.y, blocks.z);
	}

	~VulkanComputePipeline();

private:
	vk::DescriptorSet _descriptorSets[16];

	VulkanShaderRef  _shader;
	VulkanContextPtr _ctx;
	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
};
