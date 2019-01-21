#pragma once

// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include "General.h"

#include "VulkanContext.h"

class VulkanWindow
{
public:

    //GPU Safe input data
	struct Input {

		ivec2 mousePos;
		ivec2 mouseMove;
		int32 mouseButtonDown[8] = { false, false, false, false, false, false, false, false };
		int32 keyDown[8] = { false, false, false, false, false, false, false, false };
	};

	VulkanWindow();
	VulkanWindow(int x, int y, int width, int height, uint32_t flags = SDL_WINDOW_VULKAN);

	void run(std::function<void()> update);

	VulkanContextRef getContext() {
		return _vulkanContext.get();
	}

	vk::SurfaceKHR &getSurface() {
		return _surfaceKHR;
	}

	bool isKeyDown(SDL_Scancode keyCode) {
		return _keyStates[keyCode] != 0;
	}

	glm::ivec2 getMousePos() {
		return _input.mousePos;
	}

	void handleKeypress(std::function<void(SDL_Scancode code)> keyHandler) {
		_keyHandler = keyHandler;

	}

	glm::ivec2 getWindowSize() {
		SDL_GetWindowSize(_window, &_width, &_height);
		return ivec2(_width, _height);
	}

	glm::ivec2 getMouseMove() {
		return _input.mouseMove;
	}

	const Input &getInput() {
		return _input;
	}

	void lockPointer(bool toggle);

	bool getMouseDown() {
		return _isMouseDown;
	}

	SDL_Window *getSDLWindow() {
		return _window;
	}


	void handleEvents(std::function<void(SDL_Event &event)> eventHandler) {
		_eventHandler = eventHandler;
	}


	~VulkanWindow();

private:
	vk::SurfaceKHR _surfaceKHR;
	vk::Instance _vkInstance;
	SDL_Window* _window = nullptr;
	
	void handleEvent(SDL_Event &event);

	shared_ptr<VulkanContext> _vulkanContext = nullptr;

	Input _input;
	std::function<void(SDL_Scancode code)> _keyHandler = nullptr;
	int initWindow(uint32_t flags);

	std::function<void(SDL_Event &event)> _eventHandler = [](SDL_Event &event) {};

	glm::ivec2 _mousePos;
	glm::ivec2 _mouseMove;
	bool _relativeMouseMode = false;
	bool _isMouseDown = false;

	int _x = 100, _y = 100, _width = 300, _height = 300;
	const uint8_t * _keyStates;
};

