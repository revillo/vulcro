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

		auto renderer = make_shared<VulkanRenderer>(vkCtx);
		auto swapchain = make_shared<VulkanSwapchain>(vkCtx, window.getSurface());

		renderer->targetSwapcahin(swapchain);

		auto uniformLayout = vkCtx->makeUniformLayout(
			{
				VULB()
			}
		);


		std::vector<VulkanUniformLayoutRef> ulrs = {
			uniformLayout
		};

		std::vector<VulkanVertexLayoutRef> vlrs = {
			make_shared<VulkanVertexLayout>(vector<vk::Format>({
				vk::Format::eR32G32B32A32Sfloat,
				vk::Format::eR32G32B32A32Sfloat
			}))
		};

		auto shader = make_shared<VulkanShader>(
			vkCtx,
			"shaders/pos_color_vert.spv",
			"shaders/uniform_color_frag.spv",
			vlrs,
			ulrs
		);

		auto pipeline = make_shared<VulkanPipeline>(
			vkCtx,
			shader,
			renderer
		);

		int numVerts = 3;

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


		auto vbuffer = make_shared<VulkanBuffer>(
			vkCtx,
			vk::BufferUsageFlagBits::eVertexBuffer,
			sizeof(Vertex) * numVerts,
			&vs[0]
		);

		uint16_t iData[3] = {
			0, 1, 2
		};

		auto ibuffer = make_shared<VulkanBuffer>(
			vkCtx,
			vk::BufferUsageFlagBits::eIndexBuffer,
			sizeof(uint16_t) * numVerts,
			iData
		);


		ExampleUniform us[1] = {
			{
				glm::vec4(0.0, 1.0, 0.0, 1.0)
			}
		};

		auto ubuffer = make_shared<VulkanBuffer>(
			vkCtx,
			vk::BufferUsageFlagBits::eUniformBuffer,
			sizeof(ExampleUniform),
			us
		);

		auto uniformSet = make_shared<VulkanUniformSet>(vkCtx, uniformLayout);

		uniformSet->bindBuffer(0, ubuffer->getDBI());

		uniformSet->update();




		auto triangleTask = make_shared<VulkanTask>(vkCtx);

		window.run([=]() {

			//Wait for next available frame
			swapchain->nextFrame();

			//Record to command buffer
			triangleTask->begin();


			renderer->begin(triangleTask);


			auto cmd = triangleTask->cmdb();


			cmd.bindPipeline(
				vk::PipelineBindPoint::eGraphics,
				pipeline->getPipeline()
			);

			vbuffer->bindVertex(cmd);

			ibuffer->bindIndex(cmd);

			uniformSet->bind(cmd, pipeline->getLayout());

			const vk::Rect2D swapRect = swapchain->getRect();


			auto viewport = vk::Viewport(
				0.0f,
				0.0f,
				(float)swapRect.extent.width,
				(float)swapRect.extent.height,
				0.0,
				1.0
			);

			cmd.setViewport(
				0,
				1,
				&viewport
			);


			cmd.setScissor(
				0, 1, &swapRect
			);

			cmd.drawIndexed(3, 1, 0, 0, 0);




			renderer->end(triangleTask);


			triangleTask->end();

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
	std::cin >> i;

    return 0;
}