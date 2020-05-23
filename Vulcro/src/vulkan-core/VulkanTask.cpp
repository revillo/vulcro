#include "VulkanTask.h"

#include "VulkanTaskPool.h"

VulkanTask::VulkanTask(VulkanContextPtr ctx, VulkanTaskPoolRef pool) :
    VulkanTask(ctx, pool->getPool())
{
    mTaskPool = pool;
}


VulkanTask::VulkanTask(VulkanContextPtr ctx, vk::CommandPool pool) :
	mCtx(ctx),
	mPool(pool)
{
	mFence = mCtx->getDevice().createFence(vk::FenceCreateInfo());

	mCommandBuffer = mCtx->getDevice().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, 1)
	)[0];

	mCreatedCommandBuffer = true;
}

VulkanTask::VulkanTask(VulkanContextPtr ctx, vk::CommandBuffer & cb) :
	mCtx(ctx),
	mCommandBuffer(cb)
{
	mFence = mCtx->getDevice().createFence(vk::FenceCreateInfo());

	mCreatedCommandBuffer = false;
}

VulkanTask::~VulkanTask()
{
    // Free the command buffer if this task was responsible for allocating it
    if (mCreatedCommandBuffer)
    {
        mCtx->getDevice().freeCommandBuffers(mPool, 1, &mCommandBuffer);
    }

    // Destroy the fence
    mCtx->getDevice().destroyFence(mFence);
}

void VulkanTask::record(function<void(vk::CommandBuffer*)> commands)
{
	begin();
	commands(&mCommandBuffer);
	end();
}

void VulkanTask::begin()
{
	vk::CommandBufferBeginInfo bgi;
	//bgi.flags = mAutoReset ? vk::CommandBufferUsageFlagBits::eOneTimeSubmit : vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	//bgi.flags = mAutoReset ? vk::CommandBufferUsageFlagBits::eOneTimeSubmit : vk::CommandBufferUsageFlags(0);
    bgi.flags = vk::CommandBufferUsageFlags(0);

	mCommandBuffer.begin(bgi);
}

void VulkanTask::end()
{
	mCommandBuffer.end();
}

void VulkanTask::execute(bool blockUntilFinished, temps<vk::Semaphore> inSems, temps<vk::Semaphore> outSems)
{
	vk::PipelineStageFlags wait_flags = vk::PipelineStageFlagBits::eTopOfPipe;

	vk::PipelineStageFlags flags[5] = { wait_flags, wait_flags, wait_flags, wait_flags, wait_flags };

	auto submit = vk::SubmitInfo(
		static_cast<uint32_t>(inSems.size()),
		inSems.size() > 0 ? inSems.begin() : nullptr,
		flags,
		1,
		&mCommandBuffer,
		static_cast<uint32_t>(outSems.size()),
		outSems.size() > 0 ? outSems.begin() : nullptr
	);

    vk::Result res = mCtx->getQueue().submit(1, &submit, blockUntilFinished ? mFence : nullptr);

	if (blockUntilFinished) waitUntilDone();
}


void VulkanTask::waitUntilDone()
{	
    vk::Result res = mCtx->getDevice().waitForFences(1, &mFence, VK_TRUE, 1000 * 1000 * 1000 * 10);

    if (res == vk::Result::eTimeout)
    {
        std::cerr << "(VulkanTask - waitUntilDone) task timed out" << std::endl;
    }
	else if (res != vk::Result::eSuccess)
    {
        std::cerr << "(VulkanTask - waitUntilDone) task errored out" << std::endl;
	}

	mCtx->getDevice().resetFences(1, &mFence);
}


