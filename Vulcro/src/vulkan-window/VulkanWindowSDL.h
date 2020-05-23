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

#include <memory>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include "../vulkan-core/General.h"
#include "../vulkan-core/VulkanContext.h"

class VulkanInstance;
class vk::Instance;

class VulkanWindow
{

public:


    //GPU Safe input data
	struct Input {

		ivec2 mousePos;
		ivec2 mouseMove;
		int32_t mouseButtonDown[8] = { false, false, false, false, false, false, false, false };
		int32_t keyDown[8] = { false, false, false, false, false, false, false, false };
	};

	VulkanWindow();
	VulkanWindow(int x, int y, int width, int height, uint32_t flags = SDL_WINDOW_VULKAN);

	std::shared_ptr<VulkanContext> createContext(vk::PhysicalDevice& pDevice, const std::vector<const char*> & extensions = { "VK_KHR_swapchain", "VK_KHR_get_memory_requirements2"});


	void run(std::function<void()> update);

    void updateEvents();

	vk::SurfaceKHR &getSurface() {
		return mVKsurfaceKHR;
	}

	bool isKeyDown(SDL_Scancode keyCode) {
		return mKeyStates[keyCode] != 0;
	}

	glm::ivec2 getMousePos() {
		return mDeviceInput.mousePos;
	}

	void handleKeypress(std::function<void(SDL_Scancode code)> keyHandler) {
		_keyHandler = keyHandler;

	}

	glm::ivec2 getWindowSize() {
		SDL_GetWindowSize(mSDLWindow, &_width, &_height);
		return ivec2(_width, _height);
	}

	glm::ivec2 getMouseMove() {
		return mDeviceInput.mouseMove;
	}

	const Input &getInput() {
		return mDeviceInput;
	}

	void lockPointer(bool toggle);

    void togglePointerLock()
    {
        lockPointer(!mRelativeMouseMode);
    }

    bool isPointerLocked()
    {
        return mRelativeMouseMode;
    }

	bool getMouseDown() {
		return mIsMouseDown;
	}

	SDL_Window *getSDLWindow() {
		return mSDLWindow;
	}

    vk::Instance getInstance();

	void handleEvents(std::function<void(SDL_Event &event)> eventHandler) {
		_eventHandler = eventHandler;
	}

    bool isStillRunning();

	~VulkanWindow();

private:
	vk::SurfaceKHR mVKsurfaceKHR;
	std::unique_ptr<VulkanInstance> mVKInstance = nullptr;
	SDL_Window* mSDLWindow = nullptr;
	
	void handleEvent(SDL_Event &event);

	//shared_ptr<VulkanContext> _vulkanContext = nullptr;

	Input mDeviceInput;
	std::function<void(SDL_Scancode code)> _keyHandler = nullptr;
	int initWindow(uint32_t flags);

	std::function<void(SDL_Event &event)> _eventHandler = [](SDL_Event &event) {};

	glm::ivec2 _mousePos;
	glm::ivec2 _mouseMove;
	bool mRelativeMouseMode = false;
	bool mIsMouseDown = false;

	int _x = 100, _y = 100, _width = 300, _height = 300;
	const uint8_t * mKeyStates;

    bool mStillRunning = true;
};

