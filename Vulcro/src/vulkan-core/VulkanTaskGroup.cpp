#include "VulkanTaskGroup.h"
#include "VulkanTaskPool.h"

#include <future>

VulkanTaskGroup::VulkanTaskGroup(VulkanContextPtr ctx, uint32_t numTasks, VulkanTaskPoolRef pool) :
    VulkanTaskGroup(ctx, numTasks, pool->getPool())
{
    mTaskPool = pool;
}

VulkanTaskGroup::VulkanTaskGroup(VulkanContextPtr ctx, uint32_t numTasks, vk::CommandPool pool) :
	_ctx(ctx),
	_pool(pool)
{
	_commandBuffers = _ctx->getDevice().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, numTasks)
	);

	for (auto & buffer : _commandBuffers) {
		_tasks.push_back(make_shared<VulkanTask>(_ctx, buffer));
	}
	
    for (int f = 0; f < _ctx->getQueueCount(); f++)
    {
        _fences.push_back(_ctx->getDevice().createFence(
            vk::FenceCreateInfo()
        ));
    }
}


void VulkanTaskGroup::resize(uint32_t numTasks)
{
    if (numTasks < _tasks.size())
    {
        _ctx->getDevice().freeCommandBuffers(
            _pool,
            static_cast<uint32_t>(_commandBuffers.size() - numTasks),
            &_commandBuffers[numTasks]
        );

        _tasks.resize(numTasks);

    }
    else if(numTasks > _tasks.size())
    {
        auto moreTasks = numTasks - _tasks.size();

        auto newBuffers = _ctx->getDevice().allocateCommandBuffers(
            vk::CommandBufferAllocateInfo(_pool, vk::CommandBufferLevel::ePrimary, moreTasks)
        );

        for (int i = 0; i < moreTasks; i++)
        {
            _tasks.push_back(make_shared<VulkanTask>(_ctx, newBuffers[i]));
            _commandBuffers.push_back(newBuffers[i]);
        }

    }

}

VulkanTaskResult VulkanTaskGroup::executeAcrossQueues()
{
    // The number of queues we have available to submit tasks
    auto queueCount = _ctx->getQueueCount();

    // The number of tasks we're looking to submit
    auto taskCount = _tasks.size();
    int queuesUsed = 0;

    // Todo: in the future it would be best to sort tasks by how long they're likely to take
    //      and submit to queues based on a more robust metric
    // In the mean time, if we submit the tasks evenly, how many should we submit per queue?
    int commandsToSubmitPerQueue = (taskCount / queueCount) + 1;
    uint64_t commandsSubmitted = 0;

    for (int queueIndex = 0; queueIndex < queueCount; ++queueIndex)
    {
        // How many tasks are there left to submit?
        auto tasksLeft = taskCount - commandsSubmitted;

        // Exit early if there are no tasks left
        if (tasksLeft == 0)
        {
            break;
        }

        // Submit fewer tasks if there are only so many left
        auto tasksToSubmit = tasksLeft >= commandsToSubmitPerQueue ? commandsToSubmitPerQueue : tasksLeft;

        // Collect commands for each task
        std::vector<vk::CommandBuffer> cmds;
        for (int commandIndex = 0; commandIndex < tasksToSubmit; ++commandIndex)
        {
            cmds.push_back(_tasks[commandsSubmitted + commandIndex]->getCommandBuffer());
        }

        // Increment the total number of commands submitted
        commandsSubmitted += tasksToSubmit;

        // Increment the number of queue's we've used so that
        // we know how many to wait on
        ++queuesUsed;

        // Create a description of the command list we're submiting
        auto submit = vk::SubmitInfo(
            0,
            nullptr,
            nullptr,
            cmds.size(),
            cmds.data(),
            0,
            nullptr
        );

        // Submit our command list to the queue at 'queueIndex'
        _ctx->getQueue(queueIndex).submit(
            1,
            &submit,
            _fences[queueIndex]
        );
    }

    VulkanTaskResult result = VulkanTaskResult::SUCCESS;

    // We'll now wait for a result from each queue
    for (int i = 0; i < queuesUsed; ++i)
    {
        // Count how many times we have timed out
        int limiter = 0;

        // Wait for the fence to be passed or timed out
        vk::Result res = _ctx->getDevice().waitForFences(1, &_fences[i], VK_TRUE, 1000000);

        if (res != vk::Result::eSuccess)
        {
            result = VulkanTaskResult::ERROR_LOOSE;
        }
    }

    _ctx->getDevice().resetFences(
        queuesUsed,
        _fences.data()
    );


    return result;
}

void VulkanTaskGroup::record(function<void(vk::CommandBuffer*, uint32_t taskNumber)> commands)
{
	for (uint32_t i = 0; i < _tasks.size(); i++) {
		_tasks[i]->begin();
		commands(&_tasks[i]->getCommandBuffer(), i);
		_tasks[i]->end();
	}
}

VulkanTaskGroup::~VulkanTaskGroup()
{
	_tasks.clear();

	_ctx->getDevice().freeCommandBuffers(
		_pool,
		static_cast<uint32_t>(_commandBuffers.size()),
		&_commandBuffers[0]
	);

    for (auto & fence : _fences)
    {
        _ctx->getDevice().destroyFence(
            fence
        );
    }

}

