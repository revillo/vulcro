# baby-vulkan

![Alt text](triangle.png?raw=true "Triangle")

This is a simple Hello Triangle application using Vulkan's C++ library created mainly as a reference.

I exclusively used constructor initializers instead of structs, for better or worse.

There are some light C++ abstractions to get through some of the boiler plate.
The included main function renders randomized triangles in a loop

```c++

#include "VulkanWindow.h"
#include "VulkanRenderer.h"

int main()
{
	auto window = VulkanWindow(0, 0, 400, 400);

	auto vkCtx = window.getContext();

	auto renderer = make_shared<VulkanRenderer>(vkCtx);
	auto swapchain = make_shared<VulkanSwapchain>(vkCtx, window.getSurface());

	renderer->targetSwapcahin(swapchain);
	renderer->createGraphicsPipeline();

	auto triangleTask = make_shared<VulkanTask>(vkCtx);

	window.run([=]() {

		//Create a random triangle
		renderer->createTriangle();

		//Wait for next available frame
		swapchain->nextFrame();

		//Record to command buffer
		triangleTask->begin();

		renderer->renderTriangle(triangleTask);

		triangleTask->end();

		//Submit command buffer
		triangleTask->execute(swapchain->getSemaphore());

		//Present current frame to screen
		swapchain->present();

		//Reset command buffers
		vkCtx->resetTasks();
	});

    return 0;
}

```

# Running it
Theoretically it should be cross platform, but I've only included Visual Studio 2017 files for building on Windows x32/x64

Requires the LunarG Vulkan SDK. (Packages Vulkan and SDL libraries)
