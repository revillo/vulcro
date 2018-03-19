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
		auto window = VulkanWindow(0, 0, 400, 400);

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

		//Must call update after all binding points are set
		uniformSet->update();

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
	
		const vk::Rect2D viewRect = swapchain->getRect();

		auto viewport = vk::Viewport(
			0.0f,
			0.0f,
			(float)viewRect.extent.width,
			(float)viewRect.extent.height,
			0.0,
			1.0
		);

		auto triangleTask = vctx->makeTask();

		//Main Loop
		window.run([=]() {

			//Randomize Triangle Buffers
			randomizeTriangle();

			//Wait for next available frame
			swapchain->nextFrame();

			//Record to command buffer
			triangleTask->record([=](vk::CommandBuffer * cmd) {
			
				renderer->record(cmd, [=]() {

					pipeline->bind(cmd);

					cmd->setViewport(0, 1, &viewport);

					cmd->setScissor(0, 1, &viewRect);

					vbuf->bind(cmd);

					ibuf->bind(cmd);

					uniformSet->bind(cmd, pipeline->getLayout());

					cmd->drawIndexed(vbuf->getCount(), 1, 0, 0, 0);
				
				});

			});


			//Submit command buffer
			triangleTask->execute(swapchain->getSemaphore());

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