###### Vulcro is in beta

# Vulcro - Vulkan Convenience Framework
### Now with Ray Tracing!


Raw Vulkan programming can get verbose: It takes 3000 lines of C++ to render a triangle per the LunarG SDK sample code.

The goal of Vulcro is to allow developers to write concise, readable code without sacrificing any of the functionality and flexibility that Vulkan offers. This means you can prototype your ideas quickly, or even develop full applications. Check out the samples folder to see it in action.

### What does it do?
By packaging core concepts into classes that will be familiar to graphics developers, leveraging nifty tools in C++11, and using sensible default parameters, Vulcro provides an elegant foundation to build graphics applications on top of.

Developers should still study the Vulkan API carefully to get the most out of Vulcro.

### What doesn't it do?

Vulcro is not an application, rendering engine or game engine. Rather, it provides tools to help you make those. 

It's also not a wrapper in the strict sense, as it often exposes "raw" Vulkan C++ code when there's no need to encapsulate it. 


### Features

#### Smart pointers and memory management

Vulcro objects are passed around with smart pointers. Maintain references to objects to keep their memory active, both on the CPU and on the GPU. This means that you don't have to worry about memory management, unless you really want to.

```c++
auto passMeAroundSwapchain = vctx->makeSwapchain(surface);
```

To mannually destroy an object, set all references to null:
```c++
//Frees associated memory unless someone else is using it
passMeAroundSwapchain = null;
```

#### Lambdas for command buffer recording

In Vulkan, we record a series of commands, like draw calls and buffer binding, into command buffers to be executed on the GPU.

Instead of writing code between state-machine like begin() and end() functions that the api provides, we can use nested lambdas:

```c++
//CommandBuffers are accessed through Tasks in Vulcro
auto triangleTask = vctx->makeTask();

triangleTask->record([&](vk::CommandBuffer * cmd) {
	cmd->setViewport(0, 1, &viewport);
	cmd->setScissor(0, 1, &viewRect);
		
	renderer->record(cmd, [&]() {
		pipeline->bind(cmd);
		vertexBuffer->bind(cmd);
		indexBuffer->bind(cmd);
        pipeline->bindSets(cmd, { uniformSet });
		cmd->drawIndexed(numVerts, 1, 0, 0, 0);
	});
});

//Submit command buffer, can block thread and/or synchronize via semaphores
bool blockThread = false;
triangleTask->execute(blockThread, {swapchain->getSemaphore()});
```

#### Task Groups

Groups of similar tasks can be recorded with a single lambda and then executed individually.
For example, we can record command buffers that render to every target image in the swapchain, and then execute tasks individually in the main loop.

```c++
auto taskGroup = vctx->makeTaskGroup(swapchain->numImages());

taskGroup->record([&](vk::CommandBuffer * cmd, uint32 taskNumber) {
  
  renderer->record(cmd, [&]() {

    cmd->setViewport(0, 1, &renderer->getFullViewport());

    cmd->setScissor(0, 1, &renderer->getFullRect());

    vbuf->bind(cmd);

    ibuf->bind(cmd);

    pipeline->bind(cmd);

    pipeline->bindSets(cmd, { uniformSet });

    cmd->drawIndexed(vbuf->getCount(), 1, 0, 0, 0);

  }, taskNumber); //Pass an index into the renderer to specify which framebuffer to target for this command buffer

});

//... in main loop

taskGroup->at(swapchain->getRenderingIndex())->execute(
  true, // Block on CPU until completed
  { swapchain->getSemaphore() } //Wait for swapchain to signal before rendering
);


```

#### Templated buffer helpers

You can use the helpers ibo, vbo, and ubo to easily coordinate data between cpu and gpu, and bind during rendering

```c++
struct ExampleVertex {
	glm::vec4 position;
	glm::vec3 normal;
};

struct ExampleUniform {
	glm::vec4 color;
};

//Make a uniform buffer with one ExampleUniform to hold color
auto ubuf = vctx->makeUBO<ExampleUniform>(1);

//Make the tree triangle vertices, and specify the format of each field
int numVerts = 3;

auto vbuf = vctx->makeDyanmicVBO<ExampleVertex>(
	{
		//Position
		vk::Format::eR32G32B32A32Sfloat,
		//Normal
		vk::Format::eR32G32B32Sfloat
	}
, numVerts);

//Index buffer is straightforward
auto ibuf = vctx->makeIBO({
	0, 1, 2
});
```

Update data

```c++
vbuf->at(0) = ExampleVertex({

	glm::vec4(1.0,0.0, 1.0, 1.0);
	glm::vec3(0.0, 1.0, 0.0);

});

```

Bind during rendering

```c++
ibuf->bind(cmd);
vbuf->bind(cmd);
```

#### Easy Resizing

```c++
auto resize = [=]() {
  if (!swapchain->resize()) {
    SDL_Delay(100);
    return;
  }
  renderer->resize();
  vctx->resetTasks();
  recordTasks();
};

//Inside main loop...

if (!swapchain->nextFrame()) {
  resize();
  return;
}

```

# Check the samples for more

[Hello Triangle](Samples/Triangle) - Simple triangle rasterizer showing basic features.

[Lots of Grass](Samples/Grass) - Deferred and instanced rendering.

[Raytracing](Samples/Raytracing) - Simple triangle raytracer

# Running it

Run CMake in root folder

While vulcro code is cross platform, only windows libraries/cmake are provided. Sorry, no other computer to test with!

