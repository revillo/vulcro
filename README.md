# baby-vulkan

![Alt text](triangle.png?raw=true "Triangle")

This is a simple Hello Triangle application using Vulkan's C++ library created mainly as a reference.
I exclusively used constructor style commands instead of structs, for better or worse.

There are some light C++ abstractions, eg:

```c++

int main()
{
	auto window = VulkanWindow(0, 0, 400, 400);
	
	auto renderer = make_shared<VulkanRenderer>(window.getContext(), ivec2(0, 0));
	auto swapchain = make_shared<VulkanSwapchain>(window.getContext(), window.getSurface());

	renderer->createSurfaceRenderPass(swapchain);
	renderer->createGraphicsPipeline();
	renderer->renderTriangle();

	window.run();

    return 0;
}

```

# Running it
Theoretically it should be cross platform, but I've only included Visual Studio 2017 files for building on Windows x32/x64

Requires the LunarG Vulkan SDK. (Packages Vulkan and SDL libraries)
