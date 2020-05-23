#pragma once
#include "VulkanContext.h"
#include "VulkanTask.h"

class VulkanTaskGroup {

public:

	VulkanTaskGroup(VulkanContextPtr ctx, uint32_t numTasks, VulkanTaskPoolRef pool);

    //Deprecated
    VulkanTaskGroup(VulkanContextPtr ctx, uint32_t numTasks, vk::CommandPool pool);


	void record(function<void(vk::CommandBuffer *, uint32_t taskNumber)> commands);
	
    //void recordParallel(function<void(vk::CommandBuffer *, uint32_t taskNumber)> commands);
    
    void resize(uint32_t numTasks);

    //Distribues tasks across available queues, waits on a fence on this thread
    VulkanTaskResult executeAcrossQueues();

	VulkanTaskRef at(uint32_t taskNumber) {
		return _tasks[taskNumber];
	}

	~VulkanTaskGroup();
	
private:

	VulkanContextPtr _ctx;
	vector<VulkanTaskRef> _tasks;
	vk::CommandPool _pool;
	vector<vk::CommandBuffer> _commandBuffers;
    vector<vk::Fence> _fences;

    VulkanTaskPoolRef mTaskPool;
};