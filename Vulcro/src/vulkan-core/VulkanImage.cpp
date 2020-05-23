#include "VulkanImage.h"


vk::ImageUsageFlags VulkanImage::SAMPLED_STORAGE = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
vk::ImageUsageFlags VulkanImage::SAMPLED_COLOR_ATTACHMENT = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

VulkanImage::VulkanImage(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, glm::uvec3 size, vk::ImageType imageType)
	:mContext(ctx),
	mFormat(format),
	mUsage(usage),
	mSize(size),
	mImageType(imageType)
{

}

VulkanImage::VulkanImage(VulkanContextPtr ctx, vk::Image image, vk::Format format, glm::uvec3 size, vk::ImageType imageType)
	:mContext(ctx),
	mFormat(format),
	mImage(image),
	mSize(size),
	mImageType(imageType)
{
	// TODO create a sampler factor
	mSampler = mContext->getNearestSampler();
}

void VulkanImage::createImage()
{
	mImage = mContext->getDevice().createImage(
		vk::ImageCreateInfo(vk::ImageCreateFlags(),
			mImageType,
			mFormat,
			vk::Extent3D(mSize.x, mSize.y, mSize.z),
			mMipLevels, //Mip Levels
			1, //Layers
			vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal,
			mUsage,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			vk::ImageLayout::eUndefined
		)
	);

	mImageCreated = true;
}

void VulkanImage::allocateDeviceMemory(vk::MemoryPropertyFlags memFlags)
{
	auto memProps = mContext->getPhysicalDevice().getMemoryProperties();

	auto req = mContext->getDevice().getImageMemoryRequirements(mImage);


    auto memTypeRes = mContext->getBestMemoryIndex(req, memFlags);

    assert(memTypeRes.isValid);

	
	mMemorySize = req.size;

	mMemory = mContext->getDevice().allocateMemory(
		vk::MemoryAllocateInfo(req.size, memTypeRes.value)
	);

#ifdef VULCRO_PRINT_ALLOCATIONS
    printf("VulkanImage: Allocating memory %lli \n", req.size);
#endif

	mContext->getDevice().bindImageMemory(mImage, mMemory, 0);

	mMemoryAllocated = true;

	// If the memory is host visible then map the memory, keeping the memory mapped does not come at a performance cost
	if ((memFlags & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible)
	{
		mMemoryMapping = mContext->getDevice().mapMemory(
			mMemory,
			0,
			mMemorySize,
			vk::MemoryMapFlags()
		);
	}
}

void VulkanImage::transitionLayout(vk::CommandBuffer * cmd, vk::ImageLayout layout)
{
	vk::ImageMemoryBarrier imgbarr(
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eUndefined,
		layout,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		getImage(),
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0,
			mMipLevels,
			0,
			1
		)
	);

	cmd->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlags(0),
		{},
		{},
		{ imgbarr }
	);
}

void VulkanImage::upload(uint64_t size, void* data)
{
	assert(isMemoryMapped());
	memcpy(mMemoryMapping, data, size);
}

vk::DescriptorImageInfo VulkanImage::getDII(uint16_t mipLevel)
{
	vk::ImageLayout layout;

    if (mUsage & vk::ImageUsageFlagBits::eColorAttachment)
    {
        layout = vk::ImageLayout::eColorAttachmentOptimal;
    }
    else if (mUsage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
    {
        layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }
    else
    {
        layout = vk::ImageLayout::eGeneral;
    }

    vk::ImageView imageView = mImageView;

    if (mipLevel > 0 && mMipViews.size() > 0) 
    {
        uint16_t level = glm::min<uint16_t>(mMipViews.size(), mipLevel);
        //MipViews are off by one indexed
        imageView = mMipViews[level - 1];
    }

    return vk::DescriptorImageInfo
    (
        mSampler,
        imageView,
        layout
    );
}

vk::DescriptorType VulkanImage::getDescriptorType()
{
	if (mUsage & vk::ImageUsageFlagBits::eSampled)
    {
		return vk::DescriptorType::eCombinedImageSampler;
	}
	else if (mUsage & vk::ImageUsageFlagBits::eStorage)
    {
		return vk::DescriptorType::eStorageImage;
    }
    else
    {
        abort();
    }

};

VulkanImage::~VulkanImage()
{
	if (isMemoryMapped())
    {
		mContext->getDevice().unmapMemory(mMemory);
	}

    if (mViewCreated)
    {
        mContext->getDevice().destroyImageView(mImageView);
    }

    if (mImageCreated)
    {
        mContext->getDevice().destroyImage(mImage);
    }

	if (mMemoryAllocated)
    {
		mContext->getDevice().freeMemory(mMemory);
	}
}

/**************************************************
 * 1D
 * ************************************************/

VulkanImage1D::VulkanImage1D(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, float size) : VulkanImage(ctx, usage, format, glm::uvec3(size, 1, 1), vk::ImageType::e1D)
{
	createImage();
	allocateDeviceMemory();
	createImageView(vk::ImageAspectFlagBits::eColor);
}

VulkanImage1D::VulkanImage1D(VulkanContextPtr ctx, vk::Image image, vk::Format format, int size) : VulkanImage(ctx, image, format, glm::uvec3(size, 1, 1), vk::ImageType::e1D)
{

}

void VulkanImage1D::createImageView(vk::ImageAspectFlags aspectFlags)
{
	vk::ComponentMapping cmap;
	cmap.r = vk::ComponentSwizzle::eR;
	cmap.g = vk::ComponentSwizzle::eG;
	cmap.b = vk::ComponentSwizzle::eB;
	cmap.a = vk::ComponentSwizzle::eA;

	vk::ImageSubresourceRange irange;
	irange.baseMipLevel = 0;
	irange.levelCount = 1;
	irange.setBaseArrayLayer(0);
	irange.layerCount = 1;
	irange.aspectMask = aspectFlags;

	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::e1D,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;
}

void VulkanImage1D::resize(uint64_t size)
{
	if (mViewCreated) mContext->getDevice().destroyImageView(mImageView);
	if (mImageCreated) mContext->getDevice().destroyImage(mImage);
	if (mMemoryAllocated) mContext->getDevice().freeMemory(mMemory);

	mSize.x = size;

	if (mImageCreated) createImage();
	if (mMemoryAllocated) allocateDeviceMemory();
	if (mViewCreated) createImageView();
}

/**************************************************
 * 2D
 * ************************************************/

VulkanImage2D::VulkanImage2D(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, glm::uvec2 size, vk::MemoryPropertyFlags memFlags) :
	VulkanImage(ctx, usage, format, glm::uvec3(size.x, size.y, 1),  vk::ImageType::e2D)
{
	createImage();
	allocateDeviceMemory(memFlags);

	if (usage & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
		createImageView(vk::ImageAspectFlagBits::eDepth);
	}
	else {
		createImageView(vk::ImageAspectFlagBits::eColor);
	}
}

VulkanImage2D::VulkanImage2D(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, glm::uvec2 size, uint16_t mipLevels, vk::MemoryPropertyFlags memFlags) :
	VulkanImage(ctx, usage, format, glm::uvec3(size.x, size.y, 1), vk::ImageType::e2D)
{
	mMipLevels = mipLevels;

	mUsage = mUsage | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

	createImage();
	allocateDeviceMemory(memFlags);

	if (usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
    {
		createImageView(vk::ImageAspectFlagBits::eDepth);
	}
	else {
		createImageView(vk::ImageAspectFlagBits::eColor);
	}
}

VulkanImage2D::VulkanImage2D(VulkanContextPtr ctx, vk::Image image, vk::Format format, glm::uvec2 size) :
	VulkanImage(ctx, image, format, glm::uvec3(size.x, size.y, 1), vk::ImageType::e2D)
{

}

void VulkanImage2D::createImageView(vk::ImageAspectFlags aspectFlags) {
	vk::ComponentMapping cmap;
	cmap.r = vk::ComponentSwizzle::eR;
	cmap.g = vk::ComponentSwizzle::eG;
	cmap.b = vk::ComponentSwizzle::eB;
	cmap.a = vk::ComponentSwizzle::eA;

	vk::ImageSubresourceRange irange;
	irange.baseMipLevel = 0;
	irange.levelCount = mMipLevels;
	irange.setBaseArrayLayer(0);
	irange.layerCount = 1;
	irange.aspectMask = aspectFlags;
	
	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::e2D,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;

    //Create Image Views for accessing individual mip levels

    mMipViews.resize(mMipLevels);

    for (int i = 1; i < mMipLevels; i++) 
    {
        irange.baseMipLevel = i;
        irange.levelCount = 1;

        mMipViews[i-1] = mContext->getDevice().createImageView(
            vk::ImageViewCreateInfo(
                vk::ImageViewCreateFlags(),
                mImage,
                vk::ImageViewType::e2D,
                mFormat,
                cmap,
                irange
            )
        );
    }
}

#include "VulkanTask.h"
#include "VulkanBuffer.h"

void VulkanImage2D::loadFromBuffer(VulkanBufferRef stagingBuffer, vk::CommandBuffer * cmd)
{
    VulkanTaskRef task = nullptr;

    if (!cmd)
    {
        auto pool = mContext->makeTaskPool(vk::CommandPoolCreateFlagBits::eTransient);
        task = mContext->makeTask(pool);
        task->begin();
        cmd = &task->getCommandBuffer();
    }

    transitionLayout(cmd, vk::ImageLayout::eTransferDstOptimal);

    vk::BufferImageCopy bic;
    bic.bufferRowLength = 0;
    bic.bufferOffset = 0;
    bic.bufferImageHeight = 0;

    bic.imageExtent = vk::Extent3D{ (uint32_t)mSize.x, (uint32_t)mSize.y, 1 };
    bic.imageOffset = vk::Offset3D{ 0, 0, 0 };

    bic.imageSubresource.baseArrayLayer = 0;
    bic.imageSubresource.layerCount = 1;
    bic.imageSubresource.mipLevel = 0;
    bic.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;

    cmd->copyBufferToImage(
        stagingBuffer->getBuffer(),
        getImage(),
        vk::ImageLayout::eTransferDstOptimal,
        {
            bic
        }
    );

    transitionLayout(cmd);


    if (task)
    {
        task->end();
        task->execute(true);
    }
}

void VulkanImage2D::loadFromMemory(void * pixelData, uint64_t sizeBytes, VulkanBufferRef stagingBuffer, vk::CommandBuffer * cmd)
{

    if (!stagingBuffer) 
    {
        stagingBuffer = mContext->makeBuffer(vk::BufferUsageFlagBits::eTransferSrc, sizeBytes, VulkanBuffer::CPU_ALOT, (void*)pixelData);
    }
    else
    {
        stagingBuffer->upload(sizeBytes, pixelData);
    }

    
            
    loadFromBuffer(stagingBuffer, cmd);
    

}

void VulkanImage2D::generateMipmaps(vk::CommandBuffer * cmd)
{

    VulkanTaskRef task = nullptr;

    if (!cmd)
    {
        task = mContext->makeTask();
        task->begin();
        cmd = &task->getCommandBuffer();
    }

	vk::ImageMemoryBarrier barrier = {};
	barrier.image = mImage;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	barrier.oldLayout = vk::ImageLayout::eGeneral;
	barrier.newLayout = vk::ImageLayout::eGeneral;

	int32_t mipWidth = mSize.x;
	int32_t mipHeight = mSize.y;

	for (uint16_t mipLevel = 1; mipLevel < mMipLevels; mipLevel++) 
	{
		barrier.subresourceRange.baseMipLevel = mipLevel - 1;
		//barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		//barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;

		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		cmd->pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags{}, nullptr, nullptr, { barrier }
		);

		vk::ImageBlit blit = {};
		blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
		blit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.srcSubresource.mipLevel = mipLevel - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
		blit.dstOffsets[1] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.dstSubresource.mipLevel = mipLevel;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		cmd->blitImage(
			//mImage, vk::ImageLayout::eTransferSrcOptimal,
			//mImage, vk::ImageLayout::eTransferSrcOptimal,
			mImage, vk::ImageLayout::eGeneral,
			mImage, vk::ImageLayout::eGeneral,
			{ blit }, vk::Filter::eLinear
		);

		//barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		//barrier.newLayout = vk::ImageLayout::eGeneral;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		cmd->pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlags{}, nullptr, nullptr, { barrier }
		);

        if (mipWidth > 1) 
        {
            mipWidth /= 2;
        }
        if (mipHeight > 1)
        {
            mipHeight /= 2;
        }
	}

	barrier.subresourceRange.baseMipLevel = mMipLevels - 1;
	//barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	//barrier.newLayout = vk::ImageLayout::eGeneral;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	cmd->pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::DependencyFlags{}, nullptr, nullptr, { barrier }
	);

    if (task)
    {
        task->end();
        task->execute(true);
    }

}

void VulkanImage2D::resize(uvec2 size)
{
	if (mViewCreated) mContext->getDevice().destroyImageView(mImageView);
	if (mImageCreated) mContext->getDevice().destroyImage(mImage);
	if (mMemoryAllocated) mContext->getDevice().freeMemory(mMemory);
    
    for (int i = 0; i < mMipViews.size(); ++i) 
    {
        mContext->getDevice().destroyImageView(mMipViews[i]);
    }

	mSize = uvec3(size.x, size.y, 1);

	if (mImageCreated) createImage();
	if (mMemoryAllocated) allocateDeviceMemory();
	if (mViewCreated) createImageView();
}

/**************************************************
* 3D
* ************************************************/

VulkanImage3D::VulkanImage3D(VulkanContextPtr ctx, vk::ImageUsageFlags usage, vk::Format format, glm::uvec3 size) :
	VulkanImage(ctx, usage, format, glm::uvec3(size.x, size.y, size.z), vk::ImageType::e3D)
{
	createImage();
	allocateDeviceMemory();
	createImageView(vk::ImageAspectFlagBits::eColor);
}

VulkanImage3D::VulkanImage3D(VulkanContextPtr ctx, vk::Image image, vk::Format format, glm::uvec3 size) :
	VulkanImage(ctx, image, format, glm::uvec3(size.x, size.y, size.z), vk::ImageType::e3D)
{

}

void VulkanImage3D::createImageView(vk::ImageAspectFlags aspectFlags)
{
	vk::ComponentMapping cmap;
	cmap.r = vk::ComponentSwizzle::eR;
	cmap.g = vk::ComponentSwizzle::eG;
	cmap.b = vk::ComponentSwizzle::eB;
	cmap.a = vk::ComponentSwizzle::eA;

	vk::ImageSubresourceRange irange;
	irange.baseMipLevel = 0;
	irange.levelCount = 1;
	irange.setBaseArrayLayer(0);
	irange.layerCount = 1;
	irange.aspectMask = aspectFlags;

	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::e3D,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;
}

/**************************************************
 * Cube
 * ************************************************/

VulkanImageCube::VulkanImageCube(VulkanContextPtr ctx, vk::ImageUsageFlags usage, glm::uvec2 size, vk::Format format) :
	VulkanImage(ctx, usage, format, glm::uvec3(size.x, size.y, 1), vk::ImageType::e2D)
{
	createImage();
	allocateDeviceMemory();
	createImageView(vk::ImageAspectFlagBits::eColor);
}

void VulkanImageCube::createImage()
{

	mImage = mContext->getDevice().createImage(
		vk::ImageCreateInfo(vk::ImageCreateFlagBits::eCubeCompatible,
			mImageType,
			mFormat,
			vk::Extent3D(mSize.x, mSize.y, mSize.z),
			1, //Mip Levels
			6, //Layers
			vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal,
			mUsage,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			vk::ImageLayout::eUndefined
		)
	);

	mImageCreated = true;
}

void VulkanImageCube::createImageView(vk::ImageAspectFlags aspectFlags)
{
	vk::ComponentMapping cmap;
	cmap.r = vk::ComponentSwizzle::eR;
	cmap.g = vk::ComponentSwizzle::eG;
	cmap.b = vk::ComponentSwizzle::eB;
	cmap.a = vk::ComponentSwizzle::eA;

	vk::ImageSubresourceRange irange;
	irange.baseMipLevel = 0;
	irange.levelCount = 1;
	irange.setBaseArrayLayer(0);
	irange.layerCount = 6;
	irange.aspectMask = aspectFlags;

	mImageView = mContext->getDevice().createImageView(
		vk::ImageViewCreateInfo(
			vk::ImageViewCreateFlags(),
			mImage,
			vk::ImageViewType::eCube,
			mFormat,
			cmap,
			irange
		)
	);

	mViewCreated = true;
}
