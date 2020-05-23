#include "Vulcro.h"
using namespace glm;

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
		VulkanWindow window(0, 0, 400, 400, SDL_WINDOW_VULKAN);

        vector<const char *> extensions = { "VK_KHR_swapchain", "VK_KHR_get_memory_requirements2" };
        auto vdm = std::make_unique<vke::VulkanDeviceManager>(window.getInstance());
        auto devices = vdm->findPhysicalDevicesWithCapabilities(extensions, vk::QueueFlagBits::eGraphics);
        auto vctx = window.createContext(vdm->getPhysicalDevice(devices[0]), extensions);


		auto renderer = vctx->makeRenderer();
		auto swapchain = vctx->makeSwapchain(window.getSurface());
		renderer->targetSwapcahin(swapchain);
		
		//Make a uniform buffer with one ExampleUniform to hold color
		auto ubuf = vctx->makeUBO<ExampleUniform>(1);

		//Make the tree triangle vertices, and specify the format of each field
		int numVerts = 3;
        
		auto vbuf = vctx->makeDynamicVBO<Vertex>(
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
		auto uniformSetLayout = vctx->makeSetLayout({
			ubuf->getLayout()
		});

		//Shaders take an array of vertex layouts and an array of uniform set layouts 
		auto shader = vctx->makeShader(
			"shaders/pos_color_vert.spv",
			"shaders/pos_color_frag.spv",
			{
				vbuf->getLayout()
			},
			{
				uniformSetLayout
			}
		);



		auto pipeline = vctx->makePipeline(
			shader,
			renderer,
			{
				vk::PrimitiveTopology::eTriangleList,
				3,
				vk::CullModeFlagBits::eBack,
				vk::FrontFace::eClockwise
			}
		);

		//A uniform set is like an instance of a layout. Uniform buffers can bind to it.
		auto uniformSet = vctx->makeSet(uniformSetLayout);

		//Bind ubo at binding point 0
		uniformSet->bindBuffer(0, ubuf);

		//Lambda to randomize our triangle buffers
		auto randomizeTriangle = [&]() {
			
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
			
			//vbuf->sync();
			

			ubuf->at().color = vec4(rand1(), rand1(), rand1(), 1.0);
			ubuf->sync();
		};

		randomizeTriangle();

		auto taskGroup = vctx->makeTaskGroup(swapchain->numImages());

		auto recordTasks = [&]() {
			taskGroup->record([&](vk::CommandBuffer * cmd, glm::uint32 taskNumber) {

				renderer->record(cmd, [&]() {

					cmd->setViewport(0, 1, &renderer->getFullViewport());

					cmd->setScissor(0, 1, &renderer->getFullRect());

					vbuf->bind(cmd);

					ibuf->bind(cmd);

					pipeline->bind(cmd);

					pipeline->bindSets(cmd, { uniformSet });

					cmd->drawIndexed(vbuf->getCount(), 1, 0, 0, 0);

				}, taskNumber);

			});
		};

		recordTasks();

		auto resize = [&]() {
			if (!swapchain->resize()) {
				SDL_Delay(100);
				return;
			}
			renderer->resize();
			//vctx->resetTasks();
			recordTasks();
		};

		//Main Loop
		window.run([&]() {

			//Randomize Triangle Buffers

            if (rand() % 10 == 1)
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

		});
	}


    return 0;
}