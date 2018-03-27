#include "VulkanRenderer.h"


VulkanRenderer::VulkanRenderer(VulkanContextRef ctx) :
	_ctx(ctx)
{

}

void VulkanRenderer::createDepthBuffer() {
	
	_depthImage = _ctx->makeImage(vk::ImageUsageFlagBits::eDepthStencilAttachment, 
        glm::ivec2(_fullRect.extent.width, _fullRect.extent.height), vk::Format::eD32Sfloat);
	
    _depthImage->allocateDeviceMemory();
	_depthImage->createImageView(vk::ImageAspectFlagBits::eDepth);

}

void VulkanRenderer::targetSwapcahin(VulkanSwapchainRef swapchain, bool useDepth)
{
	_useDepth = useDepth;
	_swapchain = swapchain;
    _fullRect = _swapchain->getRect();

	if (useDepth) createDepthBuffer();
	else _depthImage = nullptr;


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
            _useDepth ? _depthImage->getFormat() : vk::Format::eD16Unorm,
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
		useDepth ? &depthRef : nullptr, 
		0, 
		nullptr
	);
	
	_renderPass = _ctx->getDevice().createRenderPass(
		vk::RenderPassCreateInfo(
			vk::RenderPassCreateFlags(),
			useDepth ? 2 : 1,
			attachments,
			1,
			&subpass,
			0,
			nullptr
		)
	);

	_renderPassCreated = true;

	createSwapchainFramebuffers(swapchain);
}

void VulkanRenderer::targetImages(vector<VulkanImageRef> images, bool useDepth)
{
	_useDepth = useDepth;

    _fullRect.offset.x = 0;
    _fullRect.offset.y = 0;
    _fullRect.extent.width = images[0]->getSize().x;
    _fullRect.extent.height = images[0]->getSize().y;


	if (_useDepth)
		createDepthBuffer();
	else
		_depthImage = nullptr;

	_swapchain = nullptr;
	_images = images;

	vector<vk::AttachmentDescription> attachments;
	attachments.reserve(images.size() + 1);

	vector<vk::AttachmentReference> colorRefs;


	uint32 attachment = 0;

	for (auto &image : images) {
		
		attachments.push_back(
			vk::AttachmentDescription(
				vk::AttachmentDescriptionFlags(),
				image->getFormat(),
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined, //Initial
				vk::ImageLayout::eColorAttachmentOptimal //Final
			)
		);

		colorRefs.push_back(vk::AttachmentReference(
			attachment++,
			vk::ImageLayout::eColorAttachmentOptimal
		));

	}

	if (_useDepth) {
		attachments.push_back(vk::AttachmentDescription(
			vk::AttachmentDescriptionFlags(),
			_depthImage->getFormat(),
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal)
		);
	}
	

	vk::AttachmentReference depthRef = vk::AttachmentReference(
		attachment++,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);


	vk::SubpassDescription subpass = vk::SubpassDescription(
		vk::SubpassDescriptionFlags(),
		vk::PipelineBindPoint::eGraphics,
		0,
		nullptr,
		static_cast<uint32>(colorRefs.size()),
		&colorRefs[0],
		nullptr,
		_useDepth ? &depthRef : nullptr,
		0,
		nullptr
	);

	_renderPass = _ctx->getDevice().createRenderPass(
		vk::RenderPassCreateInfo(
			vk::RenderPassCreateFlags(),
			static_cast<uint32>(attachments.size()),
			&attachments[0],
			1,
			&subpass,
			0,
			nullptr
		)
	);

	_renderPassCreated = true;

	createImagesFramebuffer();
}


void VulkanRenderer::record(vk::CommandBuffer * cmd, function<void()> commands, int32 whichFramebuffer)
{
	begin(cmd, whichFramebuffer);

	commands();

	end(cmd);
}

void VulkanRenderer::begin(vk::CommandBuffer * cmd, int32 whichFramebuffer) {

	uint32 framebufferIndex = 0;
	const std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	std::vector<vk::ClearValue> clears;

	if (_swapchain != nullptr) {
		framebufferIndex = _swapchain->getRenderingIndex(); //todo

		if (_clearColors.size() == 0)
			clears.push_back(vk::ClearColorValue(clearColor));
		else
			clears.push_back(vk::ClearColorValue(_clearColors[0]));
	}
	else {

		int count = 0;
		for (auto &image : _images) {
			if (_clearColors.size() <= count)
				clears.push_back(vk::ClearColorValue(clearColor));
			else
				clears.push_back(vk::ClearColorValue(_clearColors[count++]));
		}
	}

	if (whichFramebuffer >= 0) {
		framebufferIndex = whichFramebuffer;
	}

	if (_useDepth) {
		clears.push_back(vk::ClearDepthStencilValue(1.0, 0));
	}

	cmd->beginRenderPass(
		vk::RenderPassBeginInfo(
			_renderPass,
			_framebuffers[framebufferIndex],
			_fullRect,
			static_cast<uint32>(clears.size()),
			&clears[0]
		),

		vk::SubpassContents::eInline
	);
}

void VulkanRenderer::resize()
{
	for (auto &fb : _framebuffers) {
		_ctx->getDevice().destroyFramebuffer(fb);
	}


	_framebuffers.clear();

	

	if (_swapchain) {
		
		_fullRect = _swapchain->getRect();

		if (_useDepth) {
			createDepthBuffer();
		}

		createSwapchainFramebuffers(_swapchain);

	}
	else {

		_fullRect = _images[0]->getFullRect();

		if (_useDepth) {
			createDepthBuffer();
		}

		createImagesFramebuffer();

	}
}

void VulkanRenderer::end(vk::CommandBuffer * cmd) {
	cmd->endRenderPass();
}

void VulkanRenderer::createImagesFramebuffer() {
	
	vector<vk::ImageView> fbAttachments;

	for (auto img : _images) {
		fbAttachments.push_back(img->getImageView());
	}

	if (_useDepth) {
		fbAttachments.push_back(_depthImage->getImageView());
	}

	_framebuffers.push_back(

		_ctx->getDevice().createFramebuffer(
			vk::FramebufferCreateInfo(
				vk::FramebufferCreateFlags(),
				_renderPass,
				static_cast<uint32>(fbAttachments.size()),
				&fbAttachments[0],
				_fullRect.extent.width,
				_fullRect.extent.height,
				1
			)
		)
	);
}

void VulkanRenderer::createSwapchainFramebuffers(VulkanSwapchainRef swapchain)
{
	vector<VulkanImageRef> swapImages = swapchain->getImages();

	for (auto img : swapImages) {

		vk::ImageView fbAttachments[2] = {
			img->getImageView(),
			_useDepth ? _depthImage->getImageView() : vk::ImageView()
		};

		_framebuffers.push_back(

			_ctx->getDevice().createFramebuffer(
				vk::FramebufferCreateInfo(
					vk::FramebufferCreateFlags(),
					_renderPass,
					_useDepth ? 2 : 1,
					fbAttachments,
					img->getSize().x,
					img->getSize().y,
					1
				)
			)

		);
	}
}

VulkanRenderer::~VulkanRenderer()
{

	for (auto &fb : _framebuffers) {
		_ctx->getDevice().destroyFramebuffer(fb);
	}

	if (_renderPassCreated) {
		_ctx->getDevice().destroyRenderPass(_renderPass);
	}

}