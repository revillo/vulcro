![alt text](grass.png "Grass")

#Grass

This demo renders 160k blades of grass. 

The goal is to show off some more advanced capabilities with vulkan.

There is still some bloat left to cut down on, so check back soon.

### Deferred Rendering

Instead of targeting the swapchain, we can target a group of images to render different buffers to.

Here, I'll render color and emissive, which is used for the bloom effect.

```c++
colorTarget = vctx->makeImage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, size, vk::Format::eR8G8B8A8Unorm);
colorTarget->allocateDeviceMemory();
colorTarget->createImageView();
colorTarget->createSampler();

emissiveTarget = vctx->makeImage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, size, vk::Format::eR8G8B8A8Unorm);
emissiveTarget->allocateDeviceMemory();
emissiveTarget->createImageView();
emissiveTarget->createSampler();

sceneRenderer->targetImages({
  colorTarget,
  emissiveTarget
}, true);
```

We can configure our shader to write to these outputs in glsl:

```c++
layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 OutEmissive;
```

Then we can use semaphores so that the final render pass must wait for the scene to finish rendering before compositing images:

```c++
sceneTask->execute(false, {}, { sceneSemaphore });
//Final compositing task waits on scene render and swapchain to signal that buffers are ready for IO
finalTask->execute(true, { swapchain->getSemaphore(), sceneSemaphore });

```
Semaphores provide a greatly simplified form of thread scheduling on the gpu.

###Instanced Rendering

Instanced rendering is easily accomplished by setting the parameter in the draw call:
We only need pass a single blade of grass into a vbo, and render it a lot.

```c++
cmd->draw(grassLevels * 2, 160000, 0, 0);
```

Then we can access this index in our shader to configure every blade of grass:

```c++
int x = gl_InstanceIndex / gridSize;
int z = gl_InstanceIndex % gridSize;
//Do shader magic
```