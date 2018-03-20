#include "Vulcro.h"
#include "glm/gtc/matrix_transform.inl"

float rand1() {
	return (float)rand() / RAND_MAX;
}

struct Vertex {
	glm::vec4 position;
};

struct VertexUV {
	glm::vec2 position;
	glm::vec2 uv;
};

struct uSceneGlobals {
	glm::mat4 perspective;
	glm::mat4 view;
};


int main()
{
	{
		glm::ivec2 windowSize(500, 500);

		auto window = VulkanWindow(0, 0, windowSize.x, windowSize.y);

		auto vctx = window.getContext();

		auto finalRenderer = vctx->makeRenderer();
		auto swapchain = vctx->makeSwapchain(window.getSurface());
		finalRenderer->targetSwapcahin(swapchain, false);

		auto sceneRenderer = vctx->makeRenderer();

		VulkanImageRef colorTarget, emissiveTarget;


		auto resize = [&vctx, &colorTarget, &emissiveTarget, &sceneRenderer, &windowSize](glm::ivec2 size) {
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

			windowSize = size;
		};

		resize(ivec2(500, 500));

		auto sceneUBO = vctx->makeUBO<uSceneGlobals>(1);
		{
			auto &usb = sceneUBO->at(0);
			usb.perspective = vulcro::glProjFix * perspective(radians(60.0f), (float)windowSize.x / (float)windowSize.y, 0.1f, 100.0f);
			
			usb.view = lookAt(vec3(0.0, 3.0, -10.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));

			sceneUBO->sync();
		}
		
		
		//uboScene->at(0).perspective = fixer * glm::perspectiveFovRH(PI / 3.0f, );
		
		auto uSceneLayout = vctx->makeUniformSetLayout({
			sceneUBO->getLayout()
			});

		auto uSceneSet = vctx->makeUniformSet(uSceneLayout);

		uSceneSet->bindBuffer(0, sceneUBO->getDBI());
		uSceneSet->update();

		Vertex vs[3] = {
			{ { -1.0, 0.0,  0.0,  1.0 } },
			{ { 0.0,  1.0,  0.0,  1.0 } },
			{ { 1.0,  0.0,  0.0,  1.0 } }
		};

		auto grassVBO = vctx->makeVBO<Vertex>({
			vk::Format::eR32G32B32A32Sfloat,
			}, 3, vs);



		auto grassIBO = vctx->makeIBO({
			0, 1, 2
			});

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
			sceneRenderer
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

		auto blitUniformLayout = vctx->makeUniformSetLayout({
			ULB(1, vk::DescriptorType::eCombinedImageSampler, &colorTarget->getSampler()),
			ULB(1, vk::DescriptorType::eCombinedImageSampler, &emissiveTarget->getSampler())
			});

		auto blitUniformSet = vctx->makeUniformSet(blitUniformLayout);

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

		auto sceneTask = vctx->makeTask();



		//Main Loop
		window.run([=]() {

			//Wait for next available frame
			swapchain->nextFrame();

			const vk::Rect2D viewRect = swapchain->getRect();

			auto viewport = vk::Viewport(
				0.0f,
				0.0f,
				(float)viewRect.extent.width,
				(float)viewRect.extent.height,
				0.0,
				1.0
			);

			//Record to command buffer
			sceneTask->record([=](vk::CommandBuffer * cmd) {

				cmd->setViewport(0, 1, &viewport);

				cmd->setScissor(0, 1, &viewRect);

				sceneRenderer->record(cmd, [=]() {

					grassPipeline->bind(cmd);

					grassVBO->bind(cmd);

					grassIBO->bind(cmd);

					uSceneSet->bind(cmd, grassPipeline->getLayout());

					cmd->drawIndexed(grassIBO->getCount(), 1, 0, 0, 0);

				});


				finalRenderer->record(cmd, [=]() {

					blitPipeline->bind(cmd);

					blitVBO->bind(cmd);

					blitIBO->bind(cmd);

					blitUniformSet->bind(cmd, blitPipeline->getLayout());

					cmd->drawIndexed(blitIBO->getCount(), 1, 0, 0, 0);

				});

			});


			//Submit command buffer
			sceneTask->execute(swapchain->getSemaphore());

			//Present current frame to screen
			swapchain->present();

			//Reset command buffers
			vctx->resetTasks();

			SDL_Delay(100);
		});


	}

	int i;
	cin >> i;

	return 0;
}