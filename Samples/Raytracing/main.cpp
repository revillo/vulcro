#include "Vulcro.h"
#include <iostream>
#include <vector>

#include "shaders/common.h"


int main()
{
	VulkanWindow window(0, 0, 512, 512, SDL_WINDOW_RESIZABLE);

    std::vector<const char *> extensions = { "VK_KHR_swapchain", "VK_KHR_get_memory_requirements2", "VK_NV_ray_tracing" };
    auto vdm = std::make_unique<vke::VulkanDeviceManager>(window.getInstance());
    auto devices = vdm->findPhysicalDevicesWithCapabilities(extensions, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);
    auto vctx = window.createContext(vdm->getPhysicalDevice(devices[0]), extensions);

    glm::ivec2 sceneSize(512, 512);

	auto colorTarget = vctx->makeImage2D(VulkanImage2D::SAMPLED_STORAGE, vk::Format::eR8G8B8A8Unorm, sceneSize);
	colorTarget->setSampler(vctx->getNearestSampler());

	auto renderer = vctx->makeRenderer();
	auto swapchain = vctx->makeSwapchain(window.getSurface());
	renderer->targetSwapcahin(swapchain);

	//Make buffers for a triangle and add to a ray tracing scene
	glm::vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
	glm::vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
	glm::vec4 green = vec4(0.0, 1.0, 0.0, 1.0);

	glm::vec4 purple = vec4(1.0, 0.0, 1.0, 1.0);
	glm::vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
	glm::vec4 cyan = vec4(0.0, 1.0, 1.0, 1.0);

	std::vector<Vertex> verts = std::vector<Vertex>({
		{glm::vec4(0.0, -1.0, 0.0, 1.0), red},
		{glm::vec4(1.0, 0.0, 0.0, 1.0), blue},
		{glm::vec4(-1.0, 0.0, 0.0, 1.0), green},

		{glm::vec4(0.0, 1.0, 0.0, 1.0), purple},
		{glm::vec4(1.0, 0.0, 0.0, 1.0), yellow},
		{glm::vec4(-1.0, 0.0, 0.0, 1.0), cyan}
	});

	auto vbuf = vctx->makeVBO<Vertex>(
		{
			vk::Format::eR32G32B32A32Sfloat, //Position
			vk::Format::eR32G32B32A32Sfloat  //Color
		}
	, verts.size(), verts.data());

	auto ibuf = vctx->makeIBO({
		0, 1, 2
	});


	//vertex buffer windows
	auto topTriangleVB = vctx->makeVBO(vbuf, 0, 3);
	auto bottomTriangleVB = vctx->makeVBO(vbuf, 3, 3);

	auto rayScene = vctx->makeRayTracingScene();

	rayScene->addGeometry(1, vctx->makeRayTracingGeometry(ibuf, topTriangleVB));
	rayScene->addGeometry(2, vctx->makeRayTracingGeometry(ibuf, bottomTriangleVB));
    
    rayScene->addInstance(1, glm::mat4(1.0f));
    rayScene->addInstance(2, glm::mat4(1.0f));

	auto buildSceneTask = vctx->makeTask();

	buildSceneTask->record([&](vk::CommandBuffer * cmd) {
		rayScene->build(cmd);
	});

	buildSceneTask->execute(true);
	


	//Setup raytracing shaders and pipeline

	auto rtSet = vctx->makeSet({
		{1, vk::DescriptorType::eAccelerationStructureNV},
		{1, vk::DescriptorType::eStorageImage},
		{1, vk::DescriptorType::eStorageBuffer}
	});

	auto rtShader = vctx->makeRayTracingShaderBuilder("shaders/ray_gen.spv", {
		rtSet->getLayout()
	});

	rtShader->addHitGroup("shaders/ray_chit.spv", nullptr);
	rtShader->addMissGroup("shaders/ray_miss.spv");

	auto rtPipeline = vctx->makeRayTracingPipeline(rtShader);

	rtSet->bindRTScene(0, rayScene);
	rtSet->bindStorageImage(1, colorTarget);
	rtSet->bindBuffer(2, vbuf->getSharedBuffer());
	
	auto rtTask = vctx->makeTask();

	rtTask->record([&](vk::CommandBuffer * cmd) {

		colorTarget->transitionLayout(cmd, vk::ImageLayout::eGeneral);

		rtPipeline->bind(cmd);
		rtPipeline->bindSets(cmd, { rtSet });
		rtPipeline->traceRays(cmd, glm::uvec2(512, 512));
	
	});

	rtTask->execute(true);
	
	auto finalSet = vctx->makeSet({
		SLB(1, vk::DescriptorType::eCombinedImageSampler, &colorTarget->getSampler())
	});

	//finalSet->bindStorageImage(0, colorTarget);
	finalSet->bindImage(0, colorTarget);

	auto finalPipeline = vctx->makePipeline(
		vctx->makeShader(
			"shaders/screen_vert.spv",
			"shaders/screen_frag.spv",
			{
			},
			{
				finalSet->getLayout()
			}
		),
		renderer,
		{ 
			vk::PrimitiveTopology::eTriangleStrip
		}
	);

	auto finalTasks = vctx->makeTaskGroup(swapchain->numImages());

	finalTasks->record([&](vk::CommandBuffer * cmd, uint32_t taskNumber) {

		cmd->setViewport(0, 1, &renderer->getFullViewport());
		cmd->setScissor(0, 1, &renderer->getFullRect());

		renderer->record(cmd, [&] {
			
			finalPipeline->bind(cmd);
			finalPipeline->bindSets(cmd, { finalSet });
			cmd->draw(4, 1, 0, 0);

		}, taskNumber);

	});

	window.run([&] {

		if (!swapchain->nextFrame()) {
			//resize();
			return;
		}

		finalTasks->at(swapchain->getRenderingIndex())->execute(
			true, // Block on CPU until completed
			{ swapchain->getSemaphore() } //Wait for swapchain to be ready before rendering
		);

		//Present current frame to screen
		if (!swapchain->present()) {
			//resize();
			return;
		}

		SDL_Delay(1);
	});

    return 0;
}
