#include "VulkanRenderer.h"


VulkanRenderer::VulkanRenderer(VulkanContextRef ctx) :
	_ctx(ctx)
{

}

void VulkanRenderer::createDepthBuffer() {
	
	const vk::Rect2D rect = _swapchain->getRect();


	_depthImage = _ctx->makeImage(vk::ImageUsageFlagBits::eDepthStencilAttachment, glm::ivec2(rect.extent.width, rect.extent.height), vk::Format::eD16Unorm);
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


void VulkanRenderer::record(vk::CommandBuffer cmd, function<void()> commands)
{
	begin(cmd);

	commands();

	end(cmd);
}

void VulkanRenderer::begin(vk::CommandBuffer cmd) {
	uint32 swapchainImageIndex = _swapchain->getRenderingIndex();

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
}

void VulkanRenderer::end(vk::CommandBuffer cmd) {
	cmd.endRenderPass();
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

VulkanRenderer::~VulkanRenderer()
{

	for (auto &fb : _framebuffers) {
		_ctx->getDevice().destroyFramebuffer(fb);
	}

	if (_renderPassCreated) {
		_ctx->getDevice().destroyRenderPass(_renderPass);
	}

}