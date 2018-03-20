#include "VulkanTask.h"



VulkanTask::VulkanTask(VulkanContextRef ctx) :
	_ctx(ctx)
{
	_fence = _ctx->getDevice().createFence(
		vk::FenceCreateInfo()
	);

	_commandBuffer = _ctx->getDevice().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(_ctx->getCommandPool(), vk::CommandBufferLevel::ePrimary, 1)
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

void VulkanTask::execute()
{
	vk::PipelineStageFlags wait_flags = vk::PipelineStageFlagBits::eBottomOfPipe;

	auto submit = vk::SubmitInfo(
		0,
		nullptr,
		&wait_flags,
		1,
		&_commandBuffer,
		0,
		nullptr
	);
	vk::Result res;

	res = _ctx->getQueue().submit(
		1,
		&submit,
		_fence
	);

	do {
		res = _ctx->getDevice().waitForFences(1, &_fence, VK_TRUE, 100000000);
	} while (res == vk::Result::eTimeout);

	_ctx->getDevice().resetFences(
		1,
		&_fence
	);
}


void VulkanTask::execute(vk::Semaphore &semaphore)
{
	vk::PipelineStageFlags wait_flags = vk::PipelineStageFlagBits::eBottomOfPipe;

	auto submit = vk::SubmitInfo(
		1,
		&semaphore,
		&wait_flags,
		1,
		&_commandBuffer,
		0,
		nullptr
	);
	vk::Result res;

	res = _ctx->getQueue().submit(
		1,
		&submit,
		_fence
	);

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
		_ctx->getCommandPool(),
		1,
		&_commandBuffer
	);


	_ctx->getDevice().destroyFence(
		_fence
	);
}
