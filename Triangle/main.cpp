#include "Vulcro.h"

float rand1() {
	return (float)rand() / RAND_MAX;
}

struct Vertex {
	glm::vec4 position;
	glm::vec4 color;
};

struct ExampleUniform {
	glm::vec4 color;
};

int main()
{
	{
		auto window = VulkanWindow(0, 0, 400, 400, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

		auto vctx = window.getContext();

		auto renderer = vctx->makeRenderer();
		auto swapchain = vctx->makeSwapchain(window.getSurface());
		renderer->targetSwapcahin(swapchain);
		
		//Make a uniform buffer with one ExampleUniform to hold color
		auto ubuf = vctx->makeUBO<ExampleUniform>(1);

		//Make the tree triangle vertices, and specify the format of each field
		int numVerts = 3;

		auto vbuf = vctx->makeVBO<Vertex>(
			{
				//Position
				vk::Format::eR32G32B32A32Sfloat,
				//Color
				vk::Format::eR32G32B32A32Sfloat
			}
		, numVerts);

		//Index buffer is straightforward
		auto ibuf = vctx->makeIBO({
			0, 1, 2
		});

		//Multiple uniform buffers can be bound to a uniform set at different binding points,
		//so provide an array of layouts.
		auto uniformSetLayout = vctx->makeUniformSetLayout({
			ubuf->getLayout()
		});

		//Shaders take an array of vertex layouts and an array of uniform set layouts 
		auto shader = vctx->makeShader(
			"shaders/pos_color_vert.spv",
			"shaders/uniform_color_frag.spv",
			{
				vbuf->getLayout()
			},
			{
				uniformSetLayout
			}
		);

		auto pipeline = vctx->makePipeline(
			shader,
			renderer
		);

		//A uniform set is like an instance of a layout. Uniform buffers can bind to it.
		auto uniformSet = vctx->makeUniformSet(uniformSetLayout);

		//Bind ubo at binding point 0
		uniformSet->bindBuffer(0, ubuf->getDBI());

		//Lambda to randomize our triangle buffers
		auto randomizeTriangle = [=]() {

			vbuf->at(0) = Vertex({
				glm::vec4(0, -rand1(), 0.0, 1),
				glm::vec4(rand1(),rand1(), rand1(), 1)
			});

			vbuf->at(1) = Vertex({
				glm::vec4(rand1(), rand1(), 0.0, 1),
				glm::vec4(rand1(), rand1(), rand1(), 1)
			});

			vbuf->at(2) = Vertex({
				glm::vec4(-rand1(), rand1(), 0.0, 1),
				glm::vec4(1, rand1(), rand1(), rand1())
			});
			
			vbuf->sync();

			ubuf->at().color = vec4(rand1(), rand1(), rand1(), 1.0);
			ubuf->sync();
		};

		randomizeTriangle();

		auto triangleTask = vctx->makeTask();

		auto taskGroup = vctx->makeTaskGroup(swapchain->numImages());

		auto recordTasks = [=]() {
			taskGroup->record([=](vk::CommandBuffer * cmd, glm::uint32 taskNumber) {

				renderer->record(cmd, [=]() {

					cmd->setViewport(0, 1, &renderer->getFullViewport());

					cmd->setScissor(0, 1, &renderer->getFullRect());

					vbuf->bind(cmd);

					ibuf->bind(cmd);

					pipeline->bind(cmd);

					pipeline->bindUniformSets(cmd, { uniformSet });

					cmd->drawIndexed(vbuf->getCount(), 1, 0, 0, 0);

				}, taskNumber);

			});
		};

		recordTasks();

		auto resize = [=]() {
			if (!swapchain->resize()) {
				SDL_Delay(100);
				return;
			}
			renderer->resize();
			vctx->resetTasks();
			recordTasks();
		};

		//Main Loop
		window.run([=]() {

			//Randomize Triangle Buffers
			randomizeTriangle();

			//Wait for next available frame
			if (!swapchain->nextFrame()) {
				resize();
				return;
			}

			taskGroup->at(swapchain->getRenderingIndex())->execute(
				true, // Block on CPU until completed
				{ swapchain->getSemaphore() } //Wait for swapchain to be ready before rendering
			);

			//Present current frame to screen
			if (!swapchain->present()) {
				resize();
				return;
			}

			SDL_Delay(100);
		});
	}

	int i;
	std::cin >> i;

    return 0;
}