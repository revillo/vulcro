#include "VulkanTask.h"



VulkanTask::VulkanTask(VulkanContextRef ctx, vk::CommandPool pool) :
	_ctx(ctx),
	_pool(pool)
{
	_fence = _ctx->getDevice().createFence(
		vk::FenceCreateInfo()
	);

	_commandBuffer = _ctx->getDevice().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, 1)
	)[0];
}


void VulkanTask::record(function<void(vk::CommandBuffer*)> commands)
{
	begin();
	commands(&_commandBuffer);
	end();
}

void VulkanTask::begin() {

	vk::CommandBufferBeginInfo bgi;
	bgi.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

	_commandBuffer.begin(
		bgi
	);
}

void VulkanTask::end() {
	_commandBuffer.end();
}

void VulkanTask::execute(bool blockUntilFinished, vector<vk::Semaphore> inSems, vector<vk::Semaphore> outSems)
{
	vk::PipelineStageFlags wait_flags = vk::PipelineStageFlagBits::eTopOfPipe;

	vk::PipelineStageFlags flags[5] = { wait_flags, wait_flags, wait_flags, wait_flags, wait_flags };


	auto submit = vk::SubmitInfo(
		inSems.size(),
		inSems.size() > 0 ? &inSems[0] : nullptr,
		flags,
		1,
		&_commandBuffer,
		outSems.size(),
		outSems.size() > 0 ? &outSems[0] : nullptr
	);

	vk::Result res;

	res = _ctx->getQueue().submit(
		1,
		&submit,
		blockUntilFinished ? _fence : nullptr
	);

	if (blockUntilFinished) waitUntilDone();
}


void VulkanTask::waitUntilDone() {

	vk::Result res;

	do {
		res = _ctx->getDevice().waitForFences(1, &_fence, VK_TRUE, 100000000);
	} while (res == vk::Result::eTimeout);

	
	_ctx->getDevice().resetFences(
		1,
		&_fence
	);
}

VulkanTask::~VulkanTask()
{

	_ctx->getDevice().freeCommandBuffers(
		_pool,
		1,
		&_commandBuffer
	);


	_ctx->getDevice().destroyFence(
		_fence
	);
}
