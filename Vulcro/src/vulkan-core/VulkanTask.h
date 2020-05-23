#pragma once

#include "General.h"
#include "vulkan/vulkan.hpp"
#include "VulkanContext.h"

enum class VulkanTaskResult
{
    SUCCESS,
    ERROR_LOOSE, // Used for errors that the program can continue running through
    ERROR_FATAL  // Used for errors that the program cannot recover from
};


class VulkanTask
{
public:

    /************************************
        Constructors / Destructor
     ************************************/

    VulkanTask(VulkanContextPtr ctx, VulkanTaskPoolRef taskPool);
	VulkanTask(VulkanContextPtr ctx, vk::CommandPool pool);
	VulkanTask(VulkanContextPtr ctx, vk::CommandBuffer &cb);

	~VulkanTask();

    /************************************
        Functions
    ************************************/

	void record(function<void(vk::CommandBuffer*)> commands);

	void begin();

	void end();
	
	void execute(bool blockUntilFinished = false, temps<vk::Semaphore> inSems = {}, temps<vk::Semaphore> outSems = {});

    /************************************
        Getters / Setters
    ************************************/

	vk::CommandBuffer & getCommandBuffer() {
		return mCommandBuffer;
	}

protected:

    /************************************
        Functions
    ************************************/

    void waitUntilDone();

    /************************************
        Members
    ************************************/

	bool mCreatedCommandBuffer = false;

	VulkanContextPtr mCtx;
	vk::CommandBuffer mCommandBuffer;
	vk::Fence mFence;
	vk::CommandPool mPool;

    VulkanTaskPoolRef mTaskPool = nullptr;
};

