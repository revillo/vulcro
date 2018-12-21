#include "VulkanWindow.h"


VulkanWindow::VulkanWindow()
{
	initWindow(SDL_WINDOW_VULKAN);
}

VulkanWindow::VulkanWindow(int x, int y, int width, int height, uint32_t flags)
	:_x(x), _y(y), _width(width), _height(height)
{
	initWindow(flags);
}

void VulkanWindow::handleEvent(SDL_Event &event)
{
	switch (event.type) {

        case SDL_MOUSEBUTTONDOWN:
            _input.mouseButtonDown[event.button.button] = true;
            break;
        case SDL_MOUSEBUTTONUP:
            _input.mouseButtonDown[event.button.button] = false;
            break;
        case SDL_KEYDOWN:
            if (_keyHandler) {
                _keyHandler(event.key.keysym.scancode);
            }
            break;

        default:
            // Do nothing.
            break;
	}

	_eventHandler(event);
}

int VulkanWindow::initWindow(uint32_t flags) {
	// Create an SDL window that supports Vulkan rendering.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Could not initialize SDL." << std::endl;
		return 1;
	}
	_window = SDL_CreateWindow("Vulkan Window", _x,
		_y, _width, _height, flags);
	if (_window == NULL) {
		std::cout << "Could not create SDL window." << std::endl;
		return 1;
	}

	// Get WSI extensions from SDL (we can add more if we like - we just can't remove these)
	unsigned extension_count;
	if (!SDL_Vulkan_GetInstanceExtensions(_window, &extension_count, NULL)) {
		std::cout << "Could not get the number of required instance extensions from SDL." << std::endl;
		return 1;
	}
	std::vector<const char*> extensions(extension_count);
	if (!SDL_Vulkan_GetInstanceExtensions(_window, &extension_count, extensions.data())) {
		std::cout << "Could not get the names of required instance extensions from SDL." << std::endl;
		return 1;
	}

	// Use validation layers if this is a debug build
	std::vector<const char*> layers;
#if defined(_DEBUG)
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	// vk::ApplicationInfo allows the programmer to specifiy some basic information about the
	// program, which can be useful for layers and tools to provide more debug information.
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setPApplicationName("Vulkan C++ Windowed Program Template")
		.setApplicationVersion(1)
		.setPEngineName("LunarG SDK")
		.setEngineVersion(1)
		.setApiVersion(VK_API_VERSION_1_0);
	// vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
	// are needed.
	vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
		.setPpEnabledLayerNames(layers.data());

	// Create the Vulkan instance.
	try {
		_vkInstance = vk::createInstance(instInfo);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
		return 1;
	}

	// Create a Vulkan surface for rendering
	VkSurfaceKHR c_surface;
	if (!SDL_Vulkan_CreateSurface(_window, static_cast<VkInstance>(_vkInstance), &c_surface)) {
		std::cout << "Could not create a Vulkan surface." << std::endl;
		return 1;
	}

	_surfaceKHR = vk::SurfaceKHR(c_surface);
	// This is where most initialization for a program should be performed	

	_vulkanContext = make_shared<VulkanContext>(_vkInstance);
	
	//_vulkanContext->createDepthImage(glm::ivec2(_width, _height));


	return 0;
}

void VulkanWindow::run(std::function<void()> update) {
	// Poll for user input.
	bool stillRunning = true;
	SDL_GetMouseState(&_input.mousePos.x, &_input.mousePos.y);


	while (stillRunning) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                stillRunning = false;
                return;
            }	

			handleEvent(event);

		}
		
		_keyStates = SDL_GetKeyboardState(NULL);

		if (!_relativeMouseMode) {
			_input.mouseMove = _input.mousePos;
			SDL_GetMouseState(&_input.mousePos.x, &_input.mousePos.y);
			_input.mouseMove = _input.mousePos - _input.mouseMove;
		}
		else {
			SDL_GetRelativeMouseState(&_input.mouseMove.x, &_input.mouseMove.y);
		}

		update();

		if (_keyStates[SDL_SCANCODE_ESCAPE]) {
			stillRunning = false;
		}
	}
}

void VulkanWindow::lockPointer(bool toggle)
{
	SDL_SetRelativeMouseMode((SDL_bool)toggle);


	if (_relativeMouseMode != toggle) {
		SDL_GetRelativeMouseState(&_input.mouseMove.x, &_input.mouseMove.y);
		_input.mouseMove = ivec2(0, 0);
	}
	_relativeMouseMode = toggle;
}

VulkanWindow::~VulkanWindow()
{
	// Clean up.
	_vulkanContext = nullptr;

	_vkInstance.destroySurfaceKHR(_surfaceKHR);
	SDL_DestroyWindow(_window);
	SDL_Quit();
	_vkInstance.destroy();
}
