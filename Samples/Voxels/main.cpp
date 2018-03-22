#include "Vulcro.h"
#include "cube.h"
#include "glm/gtc/matrix_transform.inl"
#include <chrono>
#include <ctime>

#define GRID_SIDE 20

struct BlockData {
	int32 type[GRID_SIDE * GRID_SIDE * GRID_SIDE];
};

struct Vertex {
	vec3 pos;
	vec2 uv;
};

float rand1() {
	return (float)rand() / RAND_MAX;
}

struct uSceneGlobals {
	mat4 perspective;
	mat4 view;
};

int main() {
	glm::ivec2 windowSize(500, 500);

	auto window = VulkanWindow(0, 0, windowSize.x, windowSize.y);
	uint32 gridSide = GRID_SIDE;
	uint32 gridCount = GRID_SIDE * GRID_SIDE * GRID_SIDE;

	auto vctx = window.getContext();
	auto renderer = vctx->makeRenderer();
	auto swapchain = vctx->makeSwapchain(window.getSurface());
	renderer->targetSwapcahin(swapchain);

	auto blockData = vctx->makeUBO<BlockData>(1);
	BlockData &dptr = blockData->at(0);

	for (int32 i = 0; i < gridCount; i++) {
		if (rand1() < 0.05) {
			dptr.type[i] = 1;
		}
		else {
			dptr.type[i] = 0;
		}
	}



	blockData->sync();

	auto sceneData = vctx->makeUBO<uSceneGlobals>(1);
	{
		auto &usb = sceneData->at(0);

		usb.perspective = vulcro::glProjFixYZ * perspective(
			radians(60.0f),
			(float)windowSize.x / (float)windowSize.y,
			1.0f, 100.0f
		);

		usb.view = lookAt(vec3(1.0, gridSide, gridSide * 5), vec3(gridSide, gridSide, 0.0), vec3(0.0, 1.0, 0.0));

		sceneData->sync();
	}


	auto blockVs = vctx->makeVBO<Vertex>(
		{
			//Position
			vk::Format::eR32G32B32Sfloat,
			//UV
			vk::Format::eR32G32Sfloat
		}
	, 36);

	for (int i = 0; i < 36; i++) {
		blockVs->set(i,
			{
				{CUBE_VERTICES[i * 3], CUBE_VERTICES[i * 3 + 1], CUBE_VERTICES[i * 3 + 2]},
				{ CUBE_UVS[i * 2], CUBE_UVS[i * 2 + 1]}
			}
		);
	}

	blockVs->sync();

	auto gridLayout = vctx->makeUniformSetLayout({
		blockData->getLayout()
		});

	auto sceneLayout = vctx->makeUniformSetLayout({
		sceneData->getLayout()
		});

	auto blockShader = vctx->makeShader(
		"shaders/block_vert.spv",
		"shaders/block_frag.spv",
		{
			blockVs->getLayout()
		},
		{
			sceneLayout,
			gridLayout
		}
		);

	auto blockPipeline = vctx->makePipeline(
		blockShader,
		renderer
	);


	auto blockSet = vctx->makeUniformSet(gridLayout);
	auto sceneSet = vctx->makeUniformSet(sceneLayout);

	blockSet->bindBuffer(0, blockData->getDBI());
	sceneSet->bindBuffer(0, sceneData->getDBI());

	auto task = vctx->makeTask();

	//Main Loop
	window.run([=]() {


		//Wait for next available frame
		swapchain->nextFrame();

		auto tStart = chrono::system_clock::now();

		//Record to command buffer
		task->record([=](vk::CommandBuffer * cmd) {


			renderer->record(cmd, [=]() {

				blockPipeline->bind(cmd);

				cmd->setViewport(0, 1, &renderer->getFullViewport());

				cmd->setScissor(0, 1, &renderer->getFullRect());

				blockVs->bind(cmd);

				//ibuf->bind(cmd);

				blockPipeline->bindUniformSets(cmd, {
					
					sceneSet,

					blockSet

				});

				cmd->draw(blockVs->getCount(), gridCount, 0, 0);

			});

		});


		//Submit command buffer. 
		task->execute(
			true, // Block on CPU until completed
			{ swapchain->getSemaphore() } //Wait for swapchain to be ready before rendering
		);

		auto tStop = chrono::system_clock::now();

		chrono::duration<double> dt = tStop - tStart;

		cout << "Time: " << dt.count() << std::endl <<  " FPS: " << 1.0 / dt.count() << endl;


		//Present current frame to screen
		swapchain->present();

		//Reset command buffers
		vctx->resetTasks();

		SDL_Delay(10);
	});
}