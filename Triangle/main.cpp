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

		auto vkCtx = window.getContext();

		auto renderer = vkCtx->makeRenderer();
		auto swapchain = vkCtx->makeSwapchain(window.getSurface());
		renderer->targetSwapcahin(swapchain);

		//Shaders take an array of uniform layouts and vertex attribute layouts

		auto uniformLayout = vkCtx->makeUniformLayout({
			//Just one struct in our layout
			VULB(1, vk::DescriptorType::eUniformBuffer)
		});

		vector<VulkanUniformLayoutRef> ulrs = {
			uniformLayout
		};

		vector<VulkanVertexLayoutRef> vlrs = {
			vkCtx->makeVertexLayout({
				//Position
				vk::Format::eR32G32B32A32Sfloat,
				//Color
				vk::Format::eR32G32B32A32Sfloat
			})
		};

		auto shader = vkCtx->makeShader(
			"shaders/pos_color_vert.spv",
			"shaders/uniform_color_frag.spv",
			vlrs,
			ulrs
		);

		auto pipeline = vkCtx->makePipeline(
			shader,
			renderer
		);

		//A uniform set is like an instance of a layout. Uniform buffers can bind to it.
		auto uniformSet = vkCtx->makeUniformSet(uniformLayout);

		int numVerts = 3;

		VulkanBufferRef vbuffer = nullptr, ubuffer = nullptr;

		auto randomizeTriangle = [=, &vbuffer, &ubuffer]() {

			Vertex vs[3] = {
				Vertex({
				glm::vec4(0, -rand1(), 0.0, 1),
				glm::vec4(rand1(),rand1(), rand1(), 1)
					}),

				Vertex({
				glm::vec4(rand1(), rand1(), 0.0, 1),
				glm::vec4(rand1(), rand1(), rand1(), 1)
					}),

				Vertex({
				glm::vec4(-rand1(), rand1(), 0.0, 1),
				glm::vec4(1, rand1(), rand1(), rand1())
					}),
			};


			if (vbuffer == nullptr) {
				vbuffer = vkCtx->makeBuffer(
					vk::BufferUsageFlagBits::eVertexBuffer,
					sizeof(Vertex) * numVerts,
					&vs[0]
				);
			}
			else {
				vbuffer->upload(sizeof(Vertex) * numVerts, vs);
			}

			ExampleUniform us[1] = {
				{
					glm::vec4(rand1(), rand1(), rand1(), 1.0)
				}
			};

			if (ubuffer == nullptr) {
				ubuffer = vkCtx->makeBuffer(
					vk::BufferUsageFlagBits::eUniformBuffer,
					sizeof(ExampleUniform),
					us
				);
				
				//Bind ubo at binding point 0
				uniformSet->bindBuffer(0, ubuffer->getDBI());

				//Must call update after all binding points are set
				uniformSet->update();
			}
			else {
				ubuffer->upload(sizeof(ExampleUniform), us);
			}

		};

		randomizeTriangle();

		uint16_t iData[3] = {
			0, 1, 2
		};

		auto ibuffer = vkCtx->makeBuffer(
			vk::BufferUsageFlagBits::eIndexBuffer,
			sizeof(uint16_t) * numVerts,
			iData
		);

		const vk::Rect2D viewRect = swapchain->getRect();

		auto viewport = vk::Viewport(
			0.0f,
			0.0f,
			(float)viewRect.extent.width,
			(float)viewRect.extent.height,
			0.0,
			1.0
		);

		auto triangleTask = vkCtx->makeTask();

		//Main Loop
		window.run([=]() {

			//Randomize Triangle Buffers
			randomizeTriangle();

			//Wait for next available frame
			swapchain->nextFrame();

			//Record to command buffer
			triangleTask->record([=](vk::CommandBuffer cmd) {
			
				renderer->record(cmd, [=, &cmd]() {

					cmd.bindPipeline(
						vk::PipelineBindPoint::eGraphics,
						pipeline->getPipeline()
					);

					cmd.setViewport(0, 1, &viewport);

					cmd.setScissor(0, 1, &viewRect);

					vbuffer->bindVertex(cmd);

					ibuffer->bindIndex(cmd);

					uniformSet->bind(cmd, pipeline->getLayout());

					cmd.drawIndexed(3, 1, 0, 0, 0);
				
				});

			});


			//Submit command buffer
			triangleTask->execute(swapchain->getSemaphore());

			//Present current frame to screen
			swapchain->present();

			//Reset command buffers
			vkCtx->resetTasks();

			SDL_Delay(1000);
		});
	}

	int i;
	cin >> i;

    return 0;
}