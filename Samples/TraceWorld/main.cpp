#include "Vulcro.h"
#include <iostream>
#include <vector>
#include <chrono>
#include "shaders/common.h"
#include "PerlinNoise.hpp"

#define BUILD_SCENE_POOL 1
#define RENDER_POOL 2


int main()
{
	auto window = VulkanWindow(0, 0, 512, 512, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	auto vctx = window.getContext();
	glm::ivec2 sceneSize(1024, 1024);

	auto colorTarget = vctx->makeImage2D(VulkanImage::SAMPLED_STORAGE, vk::Format::eR8G8B8A8Unorm, sceneSize);
	colorTarget->setSampler(vctx->getLinearSampler());

	auto renderer = vctx->makeRenderer();
	auto swapchain = vctx->makeSwapchain(window.getSurface());
	renderer->targetSwapcahin(swapchain);
	auto uGlobals = vctx->makeUBO<Globals>(1);

	uGlobals->at(0).camera = glm::mat4(1.0);
	uGlobals->at(0).sunDir = normalize(vec3(0.2, -0.5, -0.3));
	uGlobals->at(0).time = 0.0;

	std::vector<Vertex> verts;
	std::vector<uint32_t> indices;
	int indexOffset = 0;
	typedef uint32_t u32;
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	glm::mat4 vTransform;
	verts.reserve(90000);
	indices.reserve(400000);

	auto addFace = [&](u32 a, u32 b, u32 c, u32 d) {
		indices.push_back(indexOffset + a);
		indices.push_back(indexOffset + b);
		indices.push_back(indexOffset + c);

		indices.push_back(indexOffset + c);
		indices.push_back(indexOffset + d);
		indices.push_back(indexOffset + a);
	};

	auto addCube = [&](std::function<void(glm::vec4)> makeVertexFn, glm::vec3 pos, glm::vec3 scl) {
		vTransform = glm::mat4(1.0);
		vTransform = translate(vTransform, pos);
		vTransform = scale(vTransform, scl);

		const float side = 0.5;
		makeVertexFn({ -side, -side, side, 1.0 });
		makeVertexFn({ side, -side, side, 1.0 });
		makeVertexFn({ side, side, side, 1.0 });
		makeVertexFn({ -side, side, side, 1.0 });

		makeVertexFn({ -side, -side, -side, 1.0 });
		makeVertexFn({ side, -side,-side, 1.0 });
		makeVertexFn({ side, side, -side, 1.0 });
		makeVertexFn({ -side, side, -side, 1.0 });

		addFace(0, 1, 2, 3);
		addFace(1, 5, 6, 2);
		addFace(7, 6, 5, 4);

		addFace(4, 0, 3, 7);
		addFace(4, 5, 1, 0);
		addFace(3, 2, 6, 7);

		indexOffset += 8;
	};

	//Ground voxels
	auto addGroundVertex = [&](glm::vec4 v) {
		glm::vec4 pos = vTransform * v;
		if (v.y < 0.0 && pos.y < 0.0) color = vec4(0.4, 0.7, 0.3, 1.0);
		else color = vec4(0.4, 0.3, 0.0, 1.0);
		vec4 material = vec4(0.0, 1.0, 1.0, 1.0);
		verts.push_back({ pos, color , material });
	};

	int gridSize = 100;
	float worldSize = gridSize;

	auto Pnoise = siv::PerlinNoise(7);

	for (int x = 0; x < gridSize; x++) {
		for (int y = 0; y < gridSize; y++) {
			/*
			vec3 pos = glm::vec3(x, sin(x * 0.2) * 5.0 * cos(y * 0.2), y);
			vec3 scale = vec3(1.0, 1.0, 1.0);
			
			if (x % 20 == 0 && y % 20 == 0) {
				scale = vec3(3.0, 20.0, 1.0);
			}

			
			if (x > 40 && y > 40 && x < 60 && y < 60) {
				pos.y = 3.0;
			}

			float tdist = min(abs(x - 50), abs(y - 50));
			tdist = clamp(1.0 - tdist / 20.0, 0.0, 1.0);
			pos.y += tdist * 1.0;
			*/

			vec3 pos = vec3(x, -Pnoise.noise(x * 4.0 / worldSize, y * 4.0 / worldSize) * 10.0, y);
			vec3 scale(1.0);
			addCube(addGroundVertex, pos, scale);
		}
	}

	//Water
	auto addWaterVertex = [&](glm::vec4 v) {
		color = vec4(0.5, 0.5, 0.8, 0.5);
		vec4 material = vec4(0.8, 1.0, 1.0, 1.0);
		verts.push_back({ vTransform * v, color, material });
	};

	vTransform = glm::mat4(1.0);
	addWaterVertex({0.0, 0.0, 0.0, 1.0});
	addWaterVertex({ worldSize, 0.0, 0.0, 1.0 });
	addWaterVertex({ worldSize, 0.0, worldSize, 1.0 });
	addWaterVertex({ 0.0, 0.0, worldSize, 1.0 });

	addFace(0, 1, 2, 3);
	indexOffset += 4;

	//Temple

	auto addStoneVertex = [&](glm::vec4 v) {
		color = vec4(0.8, 0.8, 0.8, 1.0);
		vec4 material = vec4(0.0, 1.0, 1.0, 1.0);
		verts.push_back({ vTransform * v, color, material });
	};

	vec3 offset(3.0, 0.0, 0.0);
	addCube(addStoneVertex, vec3(55, -0.5, 50) + offset, vec3(10.0, 1.0, 10.0));
	addCube(addStoneVertex, vec3(55, -1.5, 50) + offset, vec3(8.0, 1.0, 8.0));
	addCube(addStoneVertex, vec3(55, -7.5, 50) + offset, vec3(8.0, 1.0, 8.0));
	addCube(addStoneVertex, vec3(55, -8.5, 50) + offset, vec3(6.0, 1.0, 6.0));
	addCube(addStoneVertex, vec3(55, -9.5, 50) + offset, vec3(4.0, 1.0, 4.0));

	addCube(addStoneVertex, vec3(51.5, -4.5, 46.5) + offset, vec3(1.0, 6.0, 1.0));
	addCube(addStoneVertex, vec3(58.5, -4.5, 46.5) + offset, vec3(1.0, 6.0, 1.0));
	addCube(addStoneVertex, vec3(58.5, -4.5, 53.5) + offset, vec3(1.0, 6.0, 1.0));
	addCube(addStoneVertex, vec3(51.5, -4.5, 53.5) + offset, vec3(1.0, 6.0, 1.0));

	addCube(addStoneVertex, vec3(65, 0.0, 50) + offset, vec3(10.0, 0.5, 2.0));


	auto vbuf = vctx->makeVBO<Vertex>(
		{
			vk::Format::eR32G32B32A32Sfloat, //Position
			vk::Format::eR32G32B32A32Sfloat,  //Color
			vk::Format::eR32G32B32A32Sfloat  //material
		}
	, verts.size(), verts.data());

	auto ibuf = vctx->makeIBO(indices);

	auto rayScene = vctx->makeRayTracingScene();
	rayScene->addGeometry(RTGeometry(ibuf, vbuf));
	

	auto buildSceneTask = vctx->makeTask(BUILD_SCENE_POOL, false);

	buildSceneTask->record([&](vk::CommandBuffer * cmd) {
		rayScene->build(cmd);
	});

	buildSceneTask->execute(true);

	


	//Setup raytracing shaders and pipeline

	auto rtSet = vctx->makeSet({
		uGlobals->getLayout(),
		{1, vk::DescriptorType::eAccelerationStructureNV},
		{1, vk::DescriptorType::eStorageImage},
		{1, vk::DescriptorType::eStorageBuffer},
		{1, vk::DescriptorType::eStorageBuffer}
	});

	auto rtShader = vctx->makeRayTracingShaderBuilder("shaders/ray_gen.spv", {
		rtSet->getLayout()
	});

	rtShader->addHitGroup("shaders/ray_chit.spv", nullptr);
	rtShader->addHitGroup("shaders/shadow_chit.spv", nullptr);
	rtShader->addMissGroup("shaders/ray_miss.spv");
	rtShader->addMissGroup("shaders/shadow_miss.spv");

	auto rtPipeline = vctx->makeRayTracingPipeline(rtShader);

	rtSet->bindBuffer(0, uGlobals);
	rtSet->bindRTScene(1, rayScene);
	rtSet->bindStorageImage(2, colorTarget);
	rtSet->bindBuffer(3, vbuf->getSharedBuffer());
	rtSet->bindBuffer(4, ibuf->getSharedBuffer());

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

	glm::vec2 cameraAngles(0.0);
	glm::vec3 & cameraPosition = (vec3)uGlobals->at(0).camera[3];
	cameraPosition = vec3(68.8, -5.0, 19.6);
	uGlobals->sync();

	auto startTime = std::chrono::system_clock::now();

	auto updateCamera = [&]() {

		auto currentTime = std::chrono::system_clock::now();
		float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0;
		float delta = (elapsed - uGlobals->at(0).time);
		//std::cout << (1.0f / delta) << std::endl;

		uGlobals->at(0).time = elapsed;
		uGlobals->sync();

		window.lockPointer(window.getMouseDown());


		auto & cameraMatrix = uGlobals->at(0).camera;
		vec3 movement(0.0);
		float speed = 10.0 * delta;

		if (window.isKeyDown(SDL_SCANCODE_W)) {
			movement += vec3(cameraMatrix[2]) * speed;
		}

		if (window.isKeyDown(SDL_SCANCODE_S)) {
			movement -= vec3(cameraMatrix[2]) * speed;
		}

		if (window.isKeyDown(SDL_SCANCODE_D)) {
			movement += vec3(cameraMatrix[0]) * speed;
		}

		if (window.isKeyDown(SDL_SCANCODE_A)) {
			movement -= vec3(cameraMatrix[0]) * speed;
		}
		float sensitivity = 0.3;
		vec2 move = window.getMouseMove();


		if (!window.getMouseDown()) {
			move = vec2(0.0);
		}

		move *= 0.01;
		//move *= delta;

		float cl = 10.0;
		move.x = clamp(move.x * sensitivity, -cl, cl);
		move.y = clamp(move.y * sensitivity, -cl, cl);

		cameraAngles += move;
		cameraPosition += movement;
		cameraAngles.y = clamp(cameraAngles.y, radians(-80.0f), radians(80.0f));

		cameraMatrix = mat4(1.0);
		cameraMatrix = rotate(cameraMatrix, cameraAngles.x, vec3(0.0, 1.0, 0.0));
		cameraMatrix = rotate(cameraMatrix, -cameraAngles.y, vec3(1.0, 0.0, 0.0));

		cameraMatrix[3] = vec4(cameraPosition, 1.0);


		uGlobals->sync();
	};

	auto finalTasks = vctx->makeTaskGroup(swapchain->numImages(), RENDER_POOL);

	auto rtTask = vctx->makeTask(RENDER_POOL, false);


	auto recordTasks = [&]() {

		finalTasks->record([&](vk::CommandBuffer * cmd, uint32_t taskNumber) {

			cmd->setViewport(0, 1, &renderer->getFullViewport());
			cmd->setScissor(0, 1, &renderer->getFullRect());

			renderer->record(cmd, [&] {

				finalPipeline->bind(cmd);
				finalPipeline->bindSets(cmd, { finalSet });
				cmd->draw(4, 1, 0, 0);

			}, taskNumber);

		});

		rtTask->record([&](vk::CommandBuffer * cmd) {

			colorTarget->transitionLayout(cmd, vk::ImageLayout::eGeneral);

			rtPipeline->bind(cmd);
			rtPipeline->bindSets(cmd, { rtSet });
			rtPipeline->traceRays(cmd, glm::uvec2(sceneSize));

		});

	};

	recordTasks();

	auto resize = [&]() {
		if (!swapchain->resize()) {
			SDL_Delay(100);
			return;
		}
		
		auto rect = swapchain->getRect();
		sceneSize.x = rect.extent.width * 1;
		sceneSize.y = rect.extent.height * 1;

		uGlobals->sync();

		colorTarget->resize(sceneSize);
		rtSet->bindStorageImage(2, colorTarget);
		finalSet->bindImage(0, colorTarget);
		renderer->resize();
		vctx->resetTasks(RENDER_POOL);
		recordTasks();
	};

	window.run([&] {

		updateCamera();
		rtTask->execute(true);


		if (!swapchain->nextFrame()) {
			resize();
			return;
		}

		finalTasks->at(swapchain->getRenderingIndex())->execute(
			true, // Block on CPU until completed
			{ swapchain->getSemaphore() } //Wait for swapchain to be ready before rendering
		);

		//Present current frame to screen
		if (!swapchain->present()) {
			resize();
			return;
		}

	});

    return 0;
}
