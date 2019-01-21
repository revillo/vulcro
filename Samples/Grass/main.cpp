#include "Vulcro.h"
#include "glm/gtc/matrix_transform.inl"
#include <chrono>
#include <ctime>

#define SCENE_TASK_POOL 0
#define FINAL_TASK_POOL 1
#define UPSAMPLE_FACTOR 2

float rand1() {
	return (float)rand() / RAND_MAX;
}

struct Vertex {
	vec4 position;
};

struct VertexUV {
	vec2 position;
	vec2 uv;
};

struct uSceneGlobals {
	mat4 perspective;
	mat4 view;
	vec4 viewPos;
	vec4 sunDirWorld;
	vec4 time;
};


int main()
{
	{
		glm::ivec2 windowSize(455, 455);


		auto window = VulkanWindow(0, 0, windowSize.x, windowSize.y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
		
		auto vctx = window.getContext();

		auto finalRenderer = vctx->makeRenderer();
		auto swapchain = vctx->makeSwapchain(window.getSurface());
		finalRenderer->targetSwapcahin(swapchain, false);

		auto sceneRenderer = vctx->makeRenderer();

		VulkanImageRef colorTarget, emissiveTarget;

		auto sceneSize = windowSize * UPSAMPLE_FACTOR;

		colorTarget = vctx->makeImage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, sceneSize, vk::Format::eR8G8B8A8Unorm);
		colorTarget->allocateDeviceMemory();
		colorTarget->createImageView();
		colorTarget->setSampler(vctx->getLinearSampler());

		emissiveTarget = vctx->makeImage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, sceneSize, vk::Format::eR8G8B8A8Unorm);
		emissiveTarget->allocateDeviceMemory();
		emissiveTarget->createImageView();
		emissiveTarget->setSampler(vctx->getLinearSampler());

		sceneRenderer->targetImages({
			colorTarget,
			emissiveTarget
			}, true);

		sceneRenderer->setClearColors({
			{0.0f, 0.0f, 0.0f, 0.0f},
			//{0.1f, 0.3f, 0.05f, 1.0f},
			{ 0.0f, 0.0f, 0.0f, 0.0f}
		});

	


		auto sceneUBO = vctx->makeUBO<uSceneGlobals>(1);
		{
			auto &usb = sceneUBO->at(0);

			usb.perspective = VULCRO_glProjFixYZ * perspective(
				radians(60.0f), 
				(float)windowSize.x / (float)windowSize.y, 
				1.0f, 100.0f);
		
		
			usb.sunDirWorld = vec4(normalize(vec3(1.0, 1.0, -1.0)), 1.0);

			sceneUBO->sync();
		}
		

		auto startMS = std::chrono::system_clock::now();

		auto rotateCam = [&sceneUBO, &startMS, &sceneSize]() {

			std::chrono::duration<double> nowS = std::chrono::system_clock::now() - startMS;
			double now = nowS.count();

			float rate = now * 0.05 + 1.0;
			

			float radius = 25.0;
			vec3 pos = vec3(sin(rate) * radius, 20.0, -cos(rate) * radius);

			sceneUBO->at(0).viewPos = vec4(pos, 1.0);
			sceneUBO->at(0).view = lookAt(pos, vec3(0.0, 12.0, 0.0), vec3(0.0, 1.0, 0.0));
			sceneUBO->at(0).time.x = now;
			sceneUBO->sync();
			sceneUBO->at(0).perspective = VULCRO_glProjFixYZ * perspective(
				radians(60.0f),
				(float)sceneSize.x / (float)sceneSize.y,
				1.0f, 100.0f);

		};
		
		rotateCam();

		//uboScene->at(0).perspective = fixer * glm::perspectiveFovRH(PI / 3.0f, );
		
		auto uSceneLayout = vctx->makeSetLayout({
			sceneUBO->getLayout()
			});

		auto uSceneSet = vctx->makeSet(uSceneLayout);

		uSceneSet->bindBuffer(0, sceneUBO->getDBI());
		uSceneSet->update();


		int grassLevels = 14;


		auto grassVBO = vctx->makeDynamicVBO<Vertex>({
			vk::Format::eR32G32B32A32Sfloat,
			}, grassLevels * 2);

		for (int i = 0; i < grassLevels; i++) {
			float h = i / (float) (grassLevels-1);
			//float taper = 1.0 - h;
			//taper = pow(taper, 0.5);

			grassVBO->set(i * 2,     { {-0.5, h, 0.0, 1.0} });
			grassVBO->set(i * 2 + 1, { {0.5, h, 0.0, 1.0} });
		}

		grassVBO->sync();

		int verticesPerBlade = grassLevels * 2;

		auto grassShader = vctx->makeShader(
			"shaders/grass_vert.spv",
			"shaders/grass_frag.spv",
			{
				grassVBO->getLayout()
			},
			{
				uSceneLayout
			}
			);



		auto grassPipeline = vctx->makePipeline(
			grassShader,
			sceneRenderer,
			{ vk::PrimitiveTopology::eTriangleStrip, 3, vk::CullModeFlagBits::eNone }
		);


		VertexUV vuvs[4] = {
			{ { -1.0, -1.0 },{ 0.0, 0.0 } },
			{ { -1.0, 1.0 }, { 0.0, 1.0 } },
			{ { 1.0, -1.0 }, { 1.0, 0.0 } },
			{ { 1.0, 1.0 },  { 1.0, 1.0 } }
		};


		auto blitVBO = vctx->makeVBO<VertexUV>({
			vk::Format::eR32G32Sfloat,
			vk::Format::eR32G32Sfloat
			}, 4, vuvs);


		auto blitIBO = vctx->makeIBO({
			0, 1, 2, 1, 3, 2
			});

		auto blitUniformLayout = vctx->makeSetLayout({
			ULB(1, vk::DescriptorType::eCombinedImageSampler, &colorTarget->getSampler()),
			ULB(1, vk::DescriptorType::eCombinedImageSampler, &emissiveTarget->getSampler())
			});

		auto blitUniformSet = vctx->makeSet(blitUniformLayout);

		blitUniformSet->bindImage(0, colorTarget);
		blitUniformSet->bindImage(1, emissiveTarget);

		auto blitShader = vctx->makeShader(
			"shaders/final_pass_vert.spv",
			"shaders/final_pass_frag.spv",
			{
				blitVBO->getLayout()
			},
			{
				blitUniformLayout
			}
			);

		auto blitPipeline = vctx->makePipeline(
			blitShader,
			finalRenderer
		);
	
		auto sceneTask = vctx->makeTask(SCENE_TASK_POOL);
		auto finalTask = vctx->makeTask(FINAL_TASK_POOL);

		auto recordTasks = [&]() {
			sceneTask->record([&](vk::CommandBuffer * cmd) {

				cmd->setViewport(0, 1, &sceneRenderer->getFullViewport());

				cmd->setScissor(0, 1, &sceneRenderer->getFullRect());

				sceneRenderer->record(cmd, [&]() {

					grassPipeline->bind(cmd);

					grassVBO->bind(cmd);

					grassPipeline->bindUniformSets(cmd, {
						uSceneSet
						});

					cmd->draw(verticesPerBlade, 160000, 0, 0);

				});
			});
		};

		recordTasks();

		vk::Semaphore sceneSemaphore = vctx->getDevice().createSemaphore(vk::SemaphoreCreateInfo());


		auto resize = [&]() {
			if (!swapchain->resize()) {
				SDL_Delay(100);
				return;
			}

			auto rect = swapchain->getRect();
			windowSize.x = rect.extent.width;
			windowSize.y = rect.extent.height;
			sceneSize = windowSize * UPSAMPLE_FACTOR;

			colorTarget->resize(sceneSize);
			emissiveTarget->resize(sceneSize);

			sceneRenderer->resize();
			finalRenderer->resize();

			blitUniformSet->bindImage(0, colorTarget);
			blitUniformSet->bindImage(1, emissiveTarget);

			vctx->resetTasks(SCENE_TASK_POOL);

			recordTasks();
		};

		//Main Loop
		window.run([&]() {

			//Wait for next available frame
			if (!swapchain->nextFrame()) {
				resize();
				return;
			}

			rotateCam();

			auto tStart = std::chrono::system_clock::now();

			sceneTask->execute(false, {}, { sceneSemaphore });

			//Record to command buffer
			finalTask->record([&](vk::CommandBuffer * cmd) {

				cmd->setViewport(0, 1, &finalRenderer->getFullViewport());
				cmd->setScissor(0, 1, &finalRenderer->getFullRect());

				finalRenderer->record(cmd, [&]() {

					blitPipeline->bind(cmd);

					blitVBO->bind(cmd);

					blitIBO->bind(cmd);

					blitPipeline->bindUniformSets(cmd, {
						blitUniformSet
						});

					cmd->drawIndexed(blitIBO->getCount(), 1, 0, 0, 0);

				});
			});

			//Submit command buffer
			finalTask->execute(true, { swapchain->getSemaphore(), sceneSemaphore });
			
			//Reset command buffers
			vctx->resetTasks(FINAL_TASK_POOL);

			//Present current frame to screen
			if (!swapchain->present()) {
				resize();
				return;
			}

			auto tStop = std::chrono::system_clock::now();
			std::chrono::duration<double> dt = tStop - tStart;
			std::cout << "FPS: " << 1.0 / dt.count() << std::endl;


			SDL_Delay(10);
		});

		vctx->getDevice().destroySemaphore(sceneSemaphore);
	}

	int i;
	std::cin >> i;
	return 0;
}