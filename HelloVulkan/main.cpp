#include "VulkanWindow.h"
#include "VulkanRenderer.h"

int main()
{
	{
		auto window = VulkanWindow(0, 0, 400, 400);

		auto vkCtx = window.getContext();

		auto renderer = make_shared<VulkanRenderer>(vkCtx);
		auto swapchain = make_shared<VulkanSwapchain>(vkCtx, window.getSurface());

		renderer->targetSwapcahin(swapchain);
		renderer->createGraphicsPipeline();

		auto triangleTask = make_shared<VulkanTask>(vkCtx);

		window.run([=]() {

			//Create a random triangle
			renderer->createTriangle();

			//Wait for next available frame
			swapchain->nextFrame();

			//Record to command buffer
			triangleTask->begin();

			renderer->renderTriangle(triangleTask);

			triangleTask->end();

			//Submit command buffer
			triangleTask->execute(swapchain->getSemaphore());

			//Present current frame to screen
			swapchain->present();

			//Reset command buffers
			vkCtx->resetTasks();

			SDL_Delay(30);
		});
	}

	int i;
	std::cin >> i;

    return 0;
}