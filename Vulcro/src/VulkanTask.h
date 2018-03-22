#pragma once

#include "General.h"
#include "vulkan\vulkan.hpp"
#include "VulkanContext.h"

class VulkanTask
{
public:
	VulkanTask(VulkanContextRef ctx, vk::CommandPool pool);
	~VulkanTask();


	void record(function<void(vk::CommandBuffer*)> commands);

	void begin();
	void end();
	
	void execute(bool blockUntilFinished = false, vector<vk::Semaphore> inSems = {}, vector<vk::Semaphore> outSems = {});

	vk::CommandBuffer &cmdb() {
		return _commandBuffer;
	}

private:

	void waitUntilDone();

	VulkanContextRef _ctx;
	vk::CommandBuffer _commandBuffer;
	vk::Fence _fence;
	vk::CommandPool _pool;
};

