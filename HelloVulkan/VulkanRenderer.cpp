#include "VulkanRenderer.h"


struct Vertex {
	glm::vec4 position;
	glm::vec4 color;
};

struct ExampleUniform {
	glm::vec4 color;
};

VulkanRenderer::VulkanRenderer(VulkanContextRef ctx) :
	_ctx(ctx)
{

}

void VulkanRenderer::createDepthBuffer() {
	
	const vk::Rect2D rect = _swapchain->getRect();
	
	 _depthImage = make_shared<VulkanImage>(_ctx, glm::ivec2(rect.extent.width, rect.extent.height), vk::Format::eD16Unorm, vk::ImageUsageFlagBits::eDepthStencilAttachment);

	_depthImage->allocateDeviceMemory();
	_depthImage->createImageView(vk::ImageAspectFlagBits::eDepth);


}

void VulkanRenderer::targetSwapcahin(VulkanSwapchainRef swapchain)
{
	_swapchain = swapchain;

	createDepthBuffer();

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

	_renderPassCreated = true;

	createSurfaceFramebuffer(swapchain);
}



float rand1() {
	return (float)rand() / RAND_MAX;
}

void VulkanRenderer::createTriangle()
{
	int numVerts = 3;

	Vertex vs[3] = {
		Vertex({
		glm::vec4(0, -rand1(), 0.0, 1),
		glm::vec4(rand1(),rand1(), rand1(), 1)
			}),

		Vertex({
		glm::vec4(rand1(), rand1(), 0.0, 1),
		glm::vec4(rand1(), rand1(), rand1(), 1)
			}),

		Vertex({
		glm::vec4(-rand1(), rand1(), 0.0, 1),
		glm::vec4(1, rand1(), rand1(), rand1())
			}),


	};


	_vbuffer = make_shared<VulkanBuffer>(
		_ctx,
		vk::BufferUsageFlagBits::eVertexBuffer,
		sizeof(Vertex) * numVerts,
		&vs[0]
		);

	uint16_t iData[3] = {
		0, 1, 2
	};

	_ibuffer = make_shared<VulkanBuffer>(
		_ctx,
		vk::BufferUsageFlagBits::eIndexBuffer,
		sizeof(uint16_t) * numVerts,
		iData
	);


	ExampleUniform us[1] = {
		{
			glm::vec4(0.0, 1.0, 0.0, 1.0)
		}
	};

	_ubuffer = make_shared<VulkanBuffer>(
		_ctx,
		vk::BufferUsageFlagBits::eUniformBuffer,
		sizeof(ExampleUniform),
		us
	);

	_uniformSet = _uniformLayout->createSet();

	_uniformSet->bindBuffer(0, _ubuffer->getDBI());

	_uniformSet->update();

}

void VulkanRenderer::renderTriangle(VulkanTaskRef task) {
	
	uint32 swapchainImageIndex = _swapchain->getRenderingIndex();

	//vk::CommandBufferBeginInfo bgi;
	//bgi.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

	//_ctx->cmd().begin(&bgi);

	auto cmd = task->cmdb();

	const std::array<float, 4> clearColor = { 0.3f, 0.3f, 0.3f, 1.0f };


	vk::ClearValue clears[2] = {
		vk::ClearColorValue(clearColor),
		vk::ClearDepthStencilValue(1.0f, 0)
	};

	const vk::Rect2D swapRect = _swapchain->getRect();


	cmd.beginRenderPass(
		vk::RenderPassBeginInfo(
			_renderPass,
			_framebuffers[swapchainImageIndex],
			swapRect,
			2,
			clears
		), 

		vk::SubpassContents::eInline
	);

	cmd.bindPipeline(
		vk::PipelineBindPoint::eGraphics,
		_pipeline
	);

	/*
	_ctx->cmd().bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		_pipelineLayout,
		0,
		0,
		nullptr,
		0,
		nullptr);
	*/


	_vbuffer->bindVertex(cmd);

	_ibuffer->bindIndex(cmd);

	_uniformSet->bind(cmd, _pipelineLayout);

	auto viewport = vk::Viewport(
		0.0f,
		0.0f,
		(float)swapRect.extent.width,
		(float)swapRect.extent.height,
		0.0,
		1.0
	);

	cmd.setViewport(
		0,
		1,
		&viewport
	);


	cmd.setScissor(
		0, 1, &swapRect
	);

	cmd.drawIndexed(3, 1, 0, 0, 0);

	cmd.endRenderPass();

}

void VulkanRenderer::createGraphicsPipeline() {

	/*
	auto shader = make_shared<VulkanShader>(
			_ctx,
		"shaders/pos_color_vert.spv",
		"shaders/pos_color_frag.spv"
	);*/


	_uniformLayout = _ctx->makeUniformLayout(
		{
			VULB()
		}
	);


	std::vector<VulkanUniformLayoutRef> ulrs = {
		_uniformLayout
	};

	auto shader = make_shared<VulkanShader>(
		_ctx,
		"shaders/pos_color_vert.spv",
		"shaders/uniform_color_frag.spv",
		ulrs
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
			offsetof(Vertex, position) //Offset
		),

		//Color
		vk::VertexInputAttributeDescription(
			1,
			0,
			vk::Format::eR32G32B32A32Sfloat,
			offsetof(Vertex, color)
		),
	};

	auto vis = vk::PipelineVertexInputStateCreateInfo(
		vk::PipelineVertexInputStateCreateFlags(),
		1,
		&vi_binding,
		2,
		vi_attribs
	);

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

	_layoutCreated = true;

	 _pipeline =  _ctx->getDevice().createGraphicsPipeline(
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
			_renderPass,
			0, //Subpass
			nullptr, //base pipeline handle
			0 //base pipeline index
		)
	 );


	 _pipelineCreated = true;


}


VulkanRenderer::~VulkanRenderer()
{

	for (auto &fb : _framebuffers) {
		_ctx->getDevice().destroyFramebuffer(fb);
	}

	if (_renderPassCreated) {
		_ctx->getDevice().destroyRenderPass(_renderPass);
	}

	if (_pipelineCreated) {
		_ctx->getDevice().destroyPipeline(_pipeline);
	}

	if (_layoutCreated) {
		_ctx->getDevice().destroyPipelineLayout(_pipelineLayout);
	}



}

void VulkanRenderer::createSurfaceFramebuffer(VulkanSwapchainRef swapchain)
{
	vector<VulkanImageRef> swapImages = swapchain->getImages();

	for (auto img : swapImages) {

		vk::ImageView fbAttachments[2] = {
			img->getImageView(),
			_depthImage->getImageView()
		};



		_framebuffers.push_back(

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
			)

		);
	}
}
