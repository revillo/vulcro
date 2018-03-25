#include "VulkanTaskGroup.h"

VulkanTaskGroup::VulkanTaskGroup(VulkanContextRef ctx, uint32 numTasks, vk::CommandPool pool) :
	_ctx(ctx),
	_pool(pool)
{
	_commandBuffers = _ctx->getDevice().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, numTasks)
	);

	for (auto & buffer : _commandBuffers) {
		_tasks.push_back(make_shared<VulkanTask>(_ctx, buffer));
	}
	
}

void VulkanTaskGroup::record(function<void(vk::CommandBuffer*, uint32 taskNumber)> commands)
{
	for (uint32 i = 0; i < _tasks.size(); i++) {
		_tasks[i]->begin();
		commands(&_tasks[i]->cmdb(), i);
		_tasks[i]->end();
	}
}

VulkanTaskGroup::~VulkanTaskGroup()
{
	_tasks.clear();

	_ctx->getDevice().freeCommandBuffers(
		_pool,
		_commandBuffers.size(),
		&_commandBuffers[0]
	);
}

