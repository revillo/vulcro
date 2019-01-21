#include "Vulcro.h"
#include <iostream>
#include <vector>
struct Vertex {
	glm::vec4 position;
};

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
	//renderer->addStorageImage(colorTarget);
	renderer->targetSwapcahin(swapchain);


	//Make buffers for a triangle and add to a ray tracing scene

	std::vector<Vertex> verts = std::vector<Vertex>({
		{glm::vec4(0.0, -1.0, 0.0, 1.0)},
		{glm::vec4(1.0, 1.0, 0.0, 1.0)},
		{glm::vec4(-1.0, 1.0, 0.0, 1.0)}
	});

	auto vbuf = vctx->makeVBO<Vertex>(
		{
			//Position
			vk::Format::eR32G32B32A32Sfloat
		}
	, 3, verts.data());

	auto ibuf = vctx->makeIBO({
		0, 1, 2
	});

	RTGeometry triangle(ibuf, vbuf);

	auto rayScene = vctx->makeRayTracingScene();
	rayScene->addGeometry(triangle);

	auto buildSceneTask = vctx->makeTask(BUILD_SCENE_POOL, false);

	buildSceneTask->record([&](vk::CommandBuffer * cmd) {
		rayScene->build(cmd);
	});

	buildSceneTask->execute(true);
	


	//Setup raytracing shaders and pipeline
	/*
	auto set = vctx->makeSet({
		ULB(1, vk::DescriptorType::eAccelerationStructureNV),
		ULB(1, vk::DescriptorType::eStorageImage)
	});
	*/

	auto sceneSet = vctx->makeSet({
		ULB(1, vk::DescriptorType::eAccelerationStructureNV)
	});

	auto imgSet = vctx->makeSet({
		ULB(1, vk::DescriptorType::eStorageImage)
	});

	auto rtShaderBuilder = vctx->makeRayTracingShaderBuilder("shaders/raygen.spv", {
		sceneSet->getLayout(),
		imgSet->getLayout()
	});

	rtShaderBuilder->addHitGroup("shaders/ray_chit.spv", nullptr);
	rtShaderBuilder->addMissGroup("shaders/ray_miss.spv");

	auto rtPipeline = vctx->makeRayTracingPipeline(rtShaderBuilder);

	sceneSet->bindRTScene(0, rayScene);
	imgSet->bindStorageImage(0, colorTarget);

	
	auto rtTask = vctx->makeTask(BUILD_SCENE_POOL, false);

	rtTask->record([&](vk::CommandBuffer * cmd) {

		colorTarget->transitionLayout(cmd, vk::ImageLayout::eGeneral);

		rtPipeline->bind(cmd);
		rtPipeline->bindSets(cmd, { sceneSet, imgSet });
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
			finalPipeline->bindUniformSets(cmd, { finalSet });
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
