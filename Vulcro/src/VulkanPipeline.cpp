#include "VulkanPipeline.h"

VulkanPipeline::VulkanPipeline(VulkanContextRef ctx, VulkanShaderRef shader, VulkanRendererRef renderer,
	PipelineConfig config,
	vector<ColorBlendConfig> colorBlendConfigs
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

    auto cbas = configureBlending();

	const std::array<float, 4> blendConstants = { 1.0, 1.0, 1.0, 1.0 };

	auto cbs = vk::PipelineColorBlendStateCreateInfo(
		vk::PipelineColorBlendStateCreateFlags(),
		VK_FALSE, //Logic Op
		vk::LogicOp::eNoOp,
		static_cast<uint32>(cbas.size()),
		&cbas[0],
		blendConstants
	);

	auto uniLayouts = shader->getDescriptorSetLayouts();

	_pipelineLayout = _ctx->getDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			static_cast<uint32>(uniLayouts.size()),
			uniLayouts.size() > 0 ? &uniLayouts[0] : nullptr,
			0,
			nullptr //Push constant Ranges
		)
	);

	auto tsci = vk::PipelineTessellationStateCreateInfo(
		vk::PipelineTessellationStateCreateFlags(),
		config.patchCount
	);

	uint32 numShaders = static_cast<uint32>(shader->getStages().size());

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

void VulkanPipeline::bind(vk::CommandBuffer * cmd)
{
	cmd->bindPipeline(
		vk::PipelineBindPoint::eGraphics,
		getPipeline()
	);

}

void VulkanPipeline::bindUniformSets(vk::CommandBuffer * cmd, const VulkanSetRef * sets, uint32 numSets)
{
	int skips = 0;

	for (int i = 0; i < numSets; i++) {
		if (sets[i] != nullptr) 
			_descriptorSets[i] = sets[i]->getDescriptorSet();
		else ++skips;
	}

	numSets -= skips;

	cmd->bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		_pipelineLayout,
		0,
		numSets,
		numSets > 0 ? _descriptorSets : nullptr,
		0,
		nullptr
	);
}

void VulkanPipeline::bindUniformSets(vk::CommandBuffer * cmd, temps<VulkanSetRef> sets)
{
	bindUniformSets(cmd, sets.begin(), sets.size());
}

void VulkanPipeline::bindUniformSets(vk::CommandBuffer * cmd, vector<VulkanSetRef>& sets)
{
	bindUniformSets(cmd, &sets[0], sets.size());
}

VulkanPipeline::~VulkanPipeline() {

	_ctx->getDevice().destroyPipeline(_pipeline);
	
	_ctx->getDevice().destroyPipelineLayout(_pipelineLayout);
	
}

vector<vk::PipelineColorBlendAttachmentState> VulkanPipeline::configureBlending()
{
    int numTargets = _renderer->getNumTargets();
    vector<vk::PipelineColorBlendAttachmentState> pcbas;

    for (int i = 0; i < numTargets; i++) {

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

    return pcbas;
}

vk::PipelineDepthStencilStateCreateInfo VulkanPipeline::configureDepthTest()
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

VulkanComputePipeline::VulkanComputePipeline(VulkanContextRef ctx, VulkanShaderRef shader) :
_shader(shader),
_ctx(ctx)
{
	
	_pipelineLayout = _ctx->getDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			static_cast<uint32>(_shader->getDescriptorSetLayouts().size()),
			&_shader->getDescriptorSetLayouts()[0],
			0, //push
			nullptr //push constants
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

void VulkanComputePipeline::bindUniformSets(vk::CommandBuffer * cmd, temps<VulkanSetRef> sets)
{
	bindUniformSets(cmd, sets.begin(), sets.size());
}

void VulkanComputePipeline::bindUniformSets(vk::CommandBuffer * cmd, vector<VulkanSetRef>& sets)
{
	bindUniformSets(cmd, &sets[0], sets.size());
}

void VulkanComputePipeline::bindUniformSets(vk::CommandBuffer * cmd, const VulkanSetRef * sets, uint32 numSets)
{
	
	for (int i = 0; i < numSets; i++) {
		_descriptorSets[i] = sets[i]->getDescriptorSet();
	}

	cmd->bindDescriptorSets(
		vk::PipelineBindPoint::eCompute,
		_pipelineLayout,
		0,
		numSets,
		numSets > 0 ? _descriptorSets : nullptr,
		0,
		nullptr
	);
}
VulkanComputePipeline::~VulkanComputePipeline()
{
	_ctx->getDevice().destroyPipelineLayout(_pipelineLayout);
	_ctx->getDevice().destroyPipeline(_pipeline);
}
