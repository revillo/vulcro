#pragma once
#include "VulkanContext.h"
#include "VulkanTask.h"

class VulkanTaskGroup {

public:
	VulkanTaskGroup(VulkanContextRef ctx, uint32_t numTasks, vk::CommandPool pool);

	void record(function<void(vk::CommandBuffer *, uint32_t taskNumber)> commands);

	VulkanTaskRef at(uint32_t taskNumber) {
		return _tasks[taskNumber];
	}

	~VulkanTaskGroup();
	
private:

	VulkanContextRef _ctx;
	vector<VulkanTaskRef> _tasks;
	vk::CommandPool _pool;
	vector<vk::CommandBuffer> _commandBuffers;

};