#include "VulkanPipeline.h"

VulkanPipeline::VulkanPipeline(VulkanContextRef ctx, VulkanShaderRef shader, VulkanRendererRef renderer) :
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
		vk::PrimitiveTopology::eTriangleList,
		VK_FALSE // RESTART
	);


	auto rs = vk::PipelineRasterizationStateCreateInfo(
		vk::PipelineRasterizationStateCreateFlags(),
		VK_FALSE, // DEPTH CLAMP
		VK_FALSE, // DISCARD
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eNone,
		vk::FrontFace::eCounterClockwise,
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


	auto sopstate = vk::StencilOpState();

	auto dss = vk::PipelineDepthStencilStateCreateInfo(
		vk::PipelineDepthStencilStateCreateFlags(),
		VK_FALSE, //DEPTH TEST
		VK_FALSE, //DEPTH WRITE
		vk::CompareOp::eLessOrEqual,
		VK_FALSE, //Bounds test
		VK_FALSE, //Stencil Test
		sopstate,
		sopstate,
		0, //Min Depth Bounds
		0 //Max Depth Bounds
	);


	auto cbas = vk::PipelineColorBlendAttachmentState(
		VK_TRUE, //Color Blend
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
	);

	const std::array<float, 4> blendConstants = { 1.0, 1.0, 1.0, 1.0 };

	auto cbs = vk::PipelineColorBlendStateCreateInfo(
		vk::PipelineColorBlendStateCreateFlags(),
		VK_FALSE, //Logic Op
		vk::LogicOp::eNoOp,
		1,
		&cbas,
		blendConstants
	);

	auto uniLayouts = shader->getDescriptorSetLayouts();

	_pipelineLayout = _ctx->getDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			uniLayouts.size(),
			uniLayouts.size() ? &uniLayouts[0] : nullptr,
			0,
			nullptr //Push constant Ranges
		)
	);

	//_layoutCreated = true;

	_pipeline = _ctx->getDevice().createGraphicsPipeline(
		nullptr,
		vk::GraphicsPipelineCreateInfo(
			vk::PipelineCreateFlags(),
			2,
			&shader->getStages()[0],
			&vis,
			&ias,
			nullptr, //Tesselation State
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

VulkanPipeline::~VulkanPipeline() {

	_ctx->getDevice().destroyPipeline(_pipeline);
	
	_ctx->getDevice().destroyPipelineLayout(_pipelineLayout);
	
}
