#include "Vulcro.h"
#include <iostream>
#include <vector>

#include "shaders/common.h"

#define BUILD_SCENE_POOL 1
#define RENDER_POOL 2

int main()
{
	auto window = VulkanWindow(0, 0, 512, 512, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	auto vctx = window.getContext();
	glm::ivec2 sceneSize(512, 512);

	auto colorTarget = vctx->makeImage(VulkanImage::SAMPLED_STORAGE, sceneSize, vk::Format::eR8G8B8A8Unorm);
	colorTarget->allocateDeviceMemory();
	colorTarget->createImageView();
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

	auto topTriangleVB = vctx->makeVBO(vbuf, 0, 3);
	auto bottomTriangleVB = vctx->makeVBO(vbuf, 3, 3);

	auto rayScene = vctx->makeRayTracingScene();

	rayScene->addGeometry(RTGeometry(ibuf, topTriangleVB));
	rayScene->addGeometry(RTGeometry(ibuf, bottomTriangleVB));

	auto buildSceneTask = vctx->makeTask(BUILD_SCENE_POOL, false);

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
	
	auto rtTask = vctx->makeTask(BUILD_SCENE_POOL, false);

	rtTask->record([&](vk::CommandBuffer * cmd) {

		colorTarget->transitionLayout(cmd, vk::ImageLayout::eGeneral);

		rtPipeline->bind(cmd);
		rtPipeline->bindSets(cmd, { rtSet });
		rtPipeline->traceRays(cmd, glm::uvec2(512, 512));
	
	});

	rtTask->execute(true);
	

	auto finalSet = vctx->makeSet({
		ULB(1, vk::DescriptorType::eCombinedImageSampler, &colorTarget->getSampler())
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

	auto finalTasks = vctx->makeTaskGroup(swapchain->numImages(), RENDER_POOL);

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

		SDL_Delay(100);
	});

    return 0;
}
