#include "Vulcro.h"
#include "glm/gtc/matrix_transform.inl"

const float PI = 3.14159f;

float rand1() {
	return (float)rand() / RAND_MAX;
}

struct Vertex {
	glm::vec3 position;
};

struct ExampleUniform {
	glm::vec4 color;
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
		finalRenderer->targetSwapcahin(swapchain);

		auto sceneRenderer = vctx->makeRenderer();
		
		VulkanImageRef colorTarget, emissiveTarget;


		auto resize = [&vctx, &colorTarget, &emissiveTarget, &sceneRenderer, &windowSize](glm::ivec2 size) {
			colorTarget = vctx->makeImage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, size, vk::Format::eR8G8B8A8Unorm);
			colorTarget->createImageView();
			colorTarget->createSampler();
			
			emissiveTarget = vctx->makeImage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, size, vk::Format::eR8G8B8A8Unorm);
			emissiveTarget->createImageView();
			emissiveTarget->createSampler();




			sceneRenderer->targetImages({
				colorTarget,
				emissiveTarget 
			});

			windowSize = size;
		};

		resize(ivec2(500, 500));


		auto uboScene = vctx->makeUBO<uSceneGlobals>(1);

		uboScene->at(0).perspective = glm::perspectiveFovRH(PI / 3.0f, (float)windowSize.x, (float)windowSize.y, 0.1f, 100.0f);
		uboScene->at(1).view = glm::mat4(1.0);

		auto uSceneLayout = vctx->makeUniformSetLayout({
			uboScene->getLayout()
		});

		auto uSceneSet = vctx->makeUniformSet(uSceneLayout);

		uSceneSet->bindBuffer(0, uboScene->getDBI());
		uSceneSet->update();

		auto grassVBO = vctx->makeVBO<Vertex>({
			vk::Format::eR32G32B32Sfloat,
		}, 3);

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


		auto blitVBO = vctx->makeVBO<Vertex>({
			vk::Format::eR32G32B32Sfloat,
		}, 4);

		auto blitIBO = vctx->makeIBO({
			0, 1, 2, 0, 2, 3
		});
		
		auto blitUniformLayout = vctx->makeUniformSetLayout({
			ULB(1, vk::DescriptorType::eCombinedImageSampler, &colorTarget->getSampler()),
			ULB(1, vk::DescriptorType::eCombinedImageSampler, &emissiveTarget->getSampler())
		});

		auto blitUniformSet = vctx->makeUniformSet(blitUniformLayout);

		blitUniformSet->bindImage(0, colorTarget);
		blitUniformSet->bindImage(1, emissiveTarget);

		auto blitShader = vctx->makeShader(
			"shader/final_pass_vert.spv",
			"shader/final_pass_frag.spv",
			{
				blitVBO->getLayout()
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

					cmd->drawIndexed(grassVBO->getCount(), 1, 0, 0, 0);

				});


				finalRenderer->record(cmd, [=]() {

					blitPipeline->bind(cmd);

					blitVBO->bind(cmd);

					blitIBO->bind(cmd);

					cmd->drawIndexed(blitVBO->getCount(), 1, 0, 0, 0);

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