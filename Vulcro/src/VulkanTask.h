#pragma once

#include "General.h"
#include "vulkan\vulkan.hpp"
#include "VulkanContext.h"

class VulkanTask
{
public:
	VulkanTask(VulkanContextRef ctx, vk::CommandPool pool, bool autoReset = false);
	VulkanTask(VulkanContextRef ctx, vk::CommandBuffer &cb, bool autoReset = false);

	~VulkanTask();


	void record(function<void(vk::CommandBuffer*)> commands);

	void begin();
	void end();
	
	void execute(bool blockUntilFinished = false, temps<vk::Semaphore> inSems = {}, temps<vk::Semaphore> outSems = {});

	vk::CommandBuffer &cmdb() {
		return _commandBuffer;
	}

protected:

	bool _autoReset;

	void waitUntilDone();

	bool createdCommandBuffer = false;

	VulkanContextRef _ctx;
	vk::CommandBuffer _commandBuffer;
	vk::Fence _fence;
	vk::CommandPool _pool;
};

