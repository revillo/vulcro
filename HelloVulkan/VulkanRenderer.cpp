#include "VulkanRenderer.h"



VulkanRenderer::VulkanRenderer(VulkanContextRef ctx, ivec2 size) :
	_ctx(ctx),
	_size(size)
{
	createDepthBuffer();
}

void VulkanRenderer::createDepthBuffer() {
	auto imgRef = make_shared<VulkanImage>(_ctx, _size, vk::Format::eD16Unorm, vk::ImageUsageFlagBits::eDepthStencilAttachment);

	auto memProps = _ctx->getPhysicalDevice().getMemoryProperties();


	auto req = _ctx->getDevice().getImageMemoryRequirements(imgRef->deviceImage());

	uint32 memTypeIndex = 1000;
	auto reqBits = req.memoryTypeBits;
	auto p = vk::MemoryPropertyFlagBits::eDeviceLocal;


	for (int i = 0; i < memProps.memoryTypeCount; i++) {
		auto type = memProps.memoryTypes[i];
		if ((reqBits >> i) && (type.propertyFlags & p) == p) {

			memTypeIndex = i;
			break;
		}
	}


	auto memory = _ctx->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(req.size, memTypeIndex)
	);

	_ctx->getDevice().bindImageMemory(imgRef->deviceImage(), memory, 0);

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
