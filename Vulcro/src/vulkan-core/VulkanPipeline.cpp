#include "VulkanPipeline.h"

VulkanRenderPipeline::VulkanRenderPipeline(VulkanContextPtr ctx, VulkanShaderRef shader, VulkanRendererRef renderer,
	PipelineConfig config,
	vector<ColorBlendConfig> colorBlendConfigs,
    uint32_t pushConstantSize
) :
	_shader(shader),
	_ctx(ctx),
	_renderer(renderer)
{
	auto vis = _shader->getVIS();

	vk::DynamicState dynamicStateEnables[2];
	dynamicStateEnables[0] = vk::DynamicState::eViewport;
	dynamicStateEnables[1] = vk::DynamicState::eScissor;

	auto dynamicState = vk::PipelineDynamicStateCreateInfo(
		vk::PipelineDynamicStateCreateFlags(),
		2,
		dynamicStateEnables
	);


	auto ias = vk::PipelineInputAssemblyStateCreateInfo(
		vk::PipelineInputAssemblyStateCreateFlags(),
		config.topology,
		VK_FALSE // RESTART
	);


	auto rs = vk::PipelineRasterizationStateCreateInfo(
		vk::PipelineRasterizationStateCreateFlags(),
		VK_FALSE, // DEPTH CLAMP
		VK_FALSE, // DISCARD
		vk::PolygonMode::eFill,
		config.cullFlags,
		config.frontFace,
		VK_FALSE, //DEPTH BIAS
		0,
		0,
		0, //Slope
		1.0 //Line Width
	);

	auto vps = vk::PipelineViewportStateCreateInfo(
		vk::PipelineViewportStateCreateFlags(),
		1,
		nullptr, //Viewports (dynamic)
		1,
		nullptr //Scissor (dynamic)
	);

	auto mss = vk::PipelineMultisampleStateCreateInfo(
		vk::PipelineMultisampleStateCreateFlags(),
		vk::SampleCountFlagBits::e1,
		VK_FALSE, //Sample Shading
		0.0, //min sample
		nullptr, //mask
		VK_FALSE, //Alpha to Converge
		VK_FALSE //Alpha to One
	);



    auto dss = configureDepthTest();

    auto cbas = configureBlending(colorBlendConfigs);

	const std::array<float, 4> blendConstants = { 1.0, 1.0, 1.0, 1.0 };

	auto cbs = vk::PipelineColorBlendStateCreateInfo(
		vk::PipelineColorBlendStateCreateFlags(),
		VK_FALSE, //Logic Op
		vk::LogicOp::eNoOp,
		static_cast<uint32_t>(cbas.size()),
        cbas.size() > 0 ? &cbas[0] : nullptr,
		blendConstants
	);

	auto uniLayouts = shader->getDescriptorSetLayouts();


    auto range = vk::PushConstantRange(vk::ShaderStageFlagBits::eAll, 0, pushConstantSize);

	_pipelineLayout = _ctx->getDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			static_cast<uint32_t>(uniLayouts.size()),
			uniLayouts.size() > 0 ? &uniLayouts[0] : nullptr,
            pushConstantSize > 0 ? 1 : 0, //push
            pushConstantSize > 0 ? &range : nullptr //push constant ranges
		)
	);

	auto tsci = vk::PipelineTessellationStateCreateInfo(
		vk::PipelineTessellationStateCreateFlags(),
		config.patchCount
	);

	uint32_t numShaders = static_cast<uint32_t>(shader->getStages().size());

	_pipeline = _ctx->getDevice().createGraphicsPipeline(
		nullptr,
		vk::GraphicsPipelineCreateInfo(
			vk::PipelineCreateFlags(),
			numShaders,
			&shader->getStages()[0],
			&vis,
			&ias,
			numShaders > 2 ? &tsci : nullptr, //Tesselation State
			&vps,
			&rs,
			&mss,
			&dss,
			&cbs,
			&dynamicState,
			_pipelineLayout,
			_renderer->getRenderPass(),
			0, //Subpass
			nullptr, //base pipeline handle
			0 //base pipeline index
		)
	);
}

void VulkanRenderPipeline::bind(vk::CommandBuffer * cmd)
{
	cmd->bindPipeline(
		vk::PipelineBindPoint::eGraphics,
		getPipeline()
	);

}

void VulkanRenderPipeline::bindSets(vk::CommandBuffer * cmd, vk::ArrayProxy<const VulkanSetRef> descriptorSets)
{
	int i = 0;

	for (auto &set : descriptorSets) {
		if (set != nullptr) {
			_descriptorSets[i++] = set->getDescriptorSet();
		}
	}

	cmd->bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		_pipelineLayout,
		0,
		i,
		i > 0 ? _descriptorSets : nullptr,
		0,
		nullptr
	);
}

VulkanRenderPipeline::~VulkanRenderPipeline() {

	_ctx->getDevice().destroyPipeline(_pipeline);
	
	_ctx->getDevice().destroyPipelineLayout(_pipelineLayout);
	
}

vector<vk::PipelineColorBlendAttachmentState> VulkanRenderPipeline::configureBlending(const vector<ColorBlendConfig>  & colorBlendConfigs)
{
    int numTargets = _renderer->getNumTargets();
    vector<vk::PipelineColorBlendAttachmentState> pcbas;

    for (int i = 0; i < numTargets; i++) {

        ColorBlendConfig cbc;

        if (colorBlendConfigs.size() > i)
        {
            cbc = colorBlendConfigs[i];
        }


        if (cbc.blend == VULCRO_BLEND_OPAQUE)
        {
            pcbas.push_back(vk::PipelineColorBlendAttachmentState(
                VK_FALSE, //Color Blend
                vk::BlendFactor::eOne, //src blend factor
                vk::BlendFactor::eZero, //dst blend factor
                vk::BlendOp::eAdd,
                vk::BlendFactor::eOne, //src alpha
                vk::BlendFactor::eZero, //dest alpha
                vk::BlendOp::eAdd, // alpha op
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA
            ));
        }
        else
        {
            pcbas.push_back(vk::PipelineColorBlendAttachmentState(
                VK_TRUE,
                vk::BlendFactor::eSrcAlpha,
                vk::BlendFactor::eOneMinusSrcAlpha,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA
            ));
        }
        
    }

    return pcbas;
}

vk::PipelineDepthStencilStateCreateInfo VulkanRenderPipeline::configureDepthTest()
{

	auto sopstate = vk::StencilOpState();

    bool hasDepth = _renderer->hasDepth();

    return vk::PipelineDepthStencilStateCreateInfo(
        vk::PipelineDepthStencilStateCreateFlags(),
        hasDepth, //DEPTH TEST
        hasDepth, //DEPTH WRITE
        vk::CompareOp::eLess,
        VK_FALSE, //Bounds test
        VK_FALSE, //Stencil Test
        sopstate,
        sopstate,
        0, //Min Depth Bounds
        1.0 //Max Depth Bounds
    );
}

VulkanComputePipeline::VulkanComputePipeline(VulkanContextPtr ctx, VulkanShaderRef shader, uint32_t pushConstantSize) :
_shader(shader),
_ctx(ctx)
{
	auto range = vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0, pushConstantSize);

	_pipelineLayout = _ctx->getDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			static_cast<uint32_t>(_shader->getDescriptorSetLayouts().size()),
			&_shader->getDescriptorSetLayouts()[0],
			pushConstantSize > 0 ? 1 : 0, //push
            pushConstantSize > 0 ? &range : nullptr //push constant ranges
		)
	);
	
	_pipeline = _ctx->getDevice().createComputePipeline(
		nullptr, //cache
		vk::ComputePipelineCreateInfo(
			vk::PipelineCreateFlags(),
			_shader->getStages()[0],
			_pipelineLayout,
			nullptr,
			0
		)
	);


}

void VulkanComputePipeline::bind(vk::CommandBuffer * cmd)
{
	cmd->bindPipeline(
		vk::PipelineBindPoint::eCompute,
		_pipeline
	);
}

void VulkanComputePipeline::bindSets(vk::CommandBuffer * cmd, vk::ArrayProxy<const VulkanSetRef> descriptorSets)
{

	int i = 0;

	for (auto &set : descriptorSets) {
		if (set != nullptr) {
			_descriptorSets[i++] = set->getDescriptorSet();
		}
	}

	cmd->bindDescriptorSets(
		vk::PipelineBindPoint::eCompute,
		_pipelineLayout,
		0,
		i,
		i > 0 ? _descriptorSets : nullptr,
		0,
		nullptr
	);
}

VulkanComputePipeline::~VulkanComputePipeline()
{
	_ctx->getDevice().destroyPipelineLayout(_pipelineLayout);
	_ctx->getDevice().destroyPipeline(_pipeline);
}
