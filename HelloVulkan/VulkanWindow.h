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

#include <iostream>
#include <vector>
#include <functional>

#include "VulkanContext.h"

class VulkanWindow
{
public:

	VulkanWindow();
	VulkanWindow(int x, int y, int width, int height);

	void run(std::function<void()> update);

	VulkanContextRef getContext() {
		return _vulkanContext;
	}

	vk::SurfaceKHR &getSurface() {
		return _surfaceKHR;
	}

	~VulkanWindow();

private:
	vk::SurfaceKHR _surfaceKHR;
	vk::Instance _vkInstance;
	SDL_Window* _window = nullptr;
	
	VulkanContextRef _vulkanContext = nullptr;

	int initWindow();

	int _x = 100, _y = 100, _width = 300, _height = 300;

};
