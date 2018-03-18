#pragma once

#include "General.h"
#include "vulkan\vulkan.hpp"
#include "VulkanContext.h"

class VulkanTask
{
public:
	VulkanTask(VulkanContextRef ctx);
	~VulkanTask();


	void record(function<void(vk::CommandBuffer)> commands);

	void begin();
	void end();

	void execute(vk::Semaphore &semaphore);

	vk::CommandBuffer &cmdb() {
		return _commandBuffer;
	}

private:

	VulkanContextRef _ctx;
	vk::CommandBuffer _commandBuffer;
	vk::Fence _fence;
};

