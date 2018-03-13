#include "VulkanRenderer.h"



VulkanRenderer::VulkanRenderer(VulkanContextRef ctx, ivec2 size) :
	_ctx(ctx),
	_size(size)
{
	createDepthBuffer();
}

void VulkanRenderer::createDepthBuffer() {
	auto imgRef = make_shared<VulkanImage>(_ctx, _size, vk::Format::eD16Unorm, vk::ImageUsageFlagBits::eDepthStencilAttachment);

	imgRef->allocateDeviceMemory();
	imgRef->createImageView(vk::ImageAspectFlagBits::eDepth);

	_depthImage = imgRef;
}

void VulkanRenderer::createSurfaceRenderPass(VulkanSwapchainRef swapchain)
{

	vk::AttachmentDescription attachments[2] = {
		vk::AttachmentDescription(
			vk::AttachmentDescriptionFlags(),
			swapchain->getFormat(),
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		),

		vk::AttachmentDescription(
			vk::AttachmentDescriptionFlags(),
			_depthImage->getFormat(),
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		)
	};

	vk::AttachmentReference colorRef = vk::AttachmentReference(
		0,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	vk::AttachmentReference depthRef = vk::AttachmentReference(
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);


	vk::SubpassDescription subpass = vk::SubpassDescription(
		vk::SubpassDescriptionFlags(),
		vk::PipelineBindPoint::eGraphics,
		0,
		nullptr,
		1,
		&colorRef,
		nullptr, 
		&depthRef, 
		0, 
		nullptr
	);
	
	_renderPass = _ctx->getDevice().createRenderPass(
		vk::RenderPassCreateInfo(
			vk::RenderPassCreateFlags(),
			2,
			attachments,
			1,
			&subpass,
			0,
			nullptr
		)
	);



	createSurfaceFramebuffer(swapchain);
}

void VulkanRenderer::createGraphicsPipeline() {


	auto shader = make_shared<VulkanShader>(
			_ctx,
		"red",
		"shaders/redv.spv",
		"shaders/redf.spv"
		);


	struct Vertex {
		glm::vec4 position;
		glm::vec4 color;
	};

	
	Vertex vs[3] = {
		Vertex({
			glm::vec4(0, 0, 0, 1), 
			glm::vec4(1, 0, 0, 1)
		}),

		Vertex({
			glm::vec4(1, 1, 0, 1),
			glm::vec4(1, 0, 1, 1)
		}),

		Vertex({
			glm::vec4(-1, 1, 0, 1),
			glm::vec4(1, 1, 0, 1)
		})
	};


	auto vbuffer = make_shared<VulkanBuffer>(
		_ctx, 
		vk::BufferUsageFlagBits::eVertexBuffer, 
		sizeof(Vertex) * 3, 
		&vs[0]
	);

	auto vi_binding = vk::VertexInputBindingDescription(
		0,
		sizeof(Vertex),
		vk::VertexInputRate::eVertex
	);

	vk::VertexInputAttributeDescription vi_attribs[2] = {
		//Pos
		vk::VertexInputAttributeDescription(
			0, //Location
			0, //Binding 
			vk::Format::eR32G32B32A32Sfloat,
			0 //Offset
		),

		//Color
		vk::VertexInputAttributeDescription(
			1,
			0,
			vk::Format::eR32G32B32A32Sfloat,
			sizeof(glm::vec4)
		),
	};

	auto vis = vk::PipelineVertexInputStateCreateInfo(
		vk::PipelineVertexInputStateCreateFlags(),
		1,
		&vi_binding,
		2,
		vi_attribs
	);


	
	//Dynamic States?
	vk::DynamicState dynamicStateEnables[4];
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);

	auto dynamicState = vk::PipelineDynamicStateCreateInfo(
		vk::PipelineDynamicStateCreateFlags(),
		0,
		dynamicStateEnables
	);


	auto vas = vk::PipelineInputAssemblyStateCreateInfo(
		vk::PipelineInputAssemblyStateCreateFlags(),
		vk::PrimitiveTopology::eTriangleList,
		VK_FALSE // RESTART
	);


	auto rs = vk::PipelineRasterizationStateCreateInfo(
		vk::PipelineRasterizationStateCreateFlags(),
		VK_TRUE, // DEPTH CLAMP
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
		VK_TRUE, //DEPTH TEST
		VK_TRUE, //DEPTH WRITE
		vk::CompareOp::eAlways,
		VK_FALSE, //Bounds test
		VK_FALSE, //Stencil Test
		sopstate,
		sopstate,
		0, //Min Depth Bounds
		0 //Max Depth Bounds
	);


	auto cbas = vk::PipelineColorBlendAttachmentState();

	auto cbs = vk::PipelineColorBlendStateCreateInfo(
		vk::PipelineColorBlendStateCreateFlags(),
		VK_FALSE, //Logic Op
		vk::LogicOp::eNoOp,
		1,
		&cbas,
		{1,1,1,1}
	);

	dynamicStateEnables[0] = vk::DynamicState::eViewport;
	dynamicStateEnables[1] = vk::DynamicState::eScissor;

	/*
	, pDynamicState(pDynamicState_)
		, layout(layout_)
		, renderPass(renderPass_)
		, subpass(subpass_)
		, basePipelineHandle(basePipelineHandle_)
		, basePipelineIndex(basePipelineIndex_)
		*/

	vk::PipelineLayout layout = _ctx->getDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			0,
			nullptr, //Set layouts todo
			0,
			nullptr //Push constant Ranges
		)
	);

	 _pipeline =  _ctx->getDevice().createGraphicsPipeline(
		vk::PipelineCache(),
		vk::GraphicsPipelineCreateInfo(
			vk::PipelineCreateFlags(),
			2,
			&shader->getStages()[0],
			&vis,
			&vas,
			nullptr, //Tesselation State
			&vps,
			&rs,
			&mss,
			&dss,
			&cbs,
			&dynamicState,
			layout,
			_renderPass,
			0, //Subpass
			VK_NULL_HANDLE, //base pipeline handle
			0 //base pipeline index
		)
	);


}


VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::createSurfaceFramebuffer(VulkanSwapchainRef swapchain)
{
	vector<VulkanImageRef> swapImages = swapchain->getImages();

	for (auto img : swapImages) {

		vk::ImageView fbAttachments[2] = {
			img->getImageView(),
			_depthImage->getImageView()
		};

		_ctx->getDevice().createFramebuffer(
			vk::FramebufferCreateInfo(
				vk::FramebufferCreateFlags(),
				_renderPass,
				2,
				fbAttachments,
				img->getSize().x,
				img->getSize().y,
				1
			)
		);
	}
}
