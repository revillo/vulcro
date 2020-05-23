#include "VulkanWindowSDL.h"

#include "../vulkan-core/VulkanInstance.h"


VulkanWindow::VulkanWindow()
{
	initWindow(SDL_WINDOW_VULKAN);
}

VulkanWindow::VulkanWindow(int x, int y, int width, int height, uint32_t flags)
	:_x(x), _y(y), _width(width), _height(height)
{
	initWindow(flags | SDL_WINDOW_VULKAN);
}

void VulkanWindow::handleEvent(SDL_Event &event)
{
	switch (event.type) {

        case SDL_MOUSEBUTTONDOWN:
            mDeviceInput.mouseButtonDown[event.button.button] = true;
            break;
        case SDL_MOUSEBUTTONUP:
            mDeviceInput.mouseButtonDown[event.button.button] = false;
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

#define vLog_e printf

int VulkanWindow::initWindow(uint32_t flags)
{
    // Initialize sdl
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
		vLog_e("Could not initialize SDL.");
		return 1;
	}

    // Create an SDL window that supports Vulkan rendering.
	mSDLWindow = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width, _height, flags);
	if (mSDLWindow == NULL)
    {
        vLog_e("Could not create SDL window.");
		return 1;
	}

	// Get WSI extensions from SDL (we can add more if we like - we just can't remove these)
	unsigned extension_count;
	if (!SDL_Vulkan_GetInstanceExtensions(mSDLWindow, &extension_count, NULL))
    {
        vLog_e("Could not get the number of required instance extensions from SDL.");
		return 1;
	}

    // Request what vulkan extensions sdl requires to create a vulkan window
	std::vector<const char*> extensions(extension_count);
	if (!SDL_Vulkan_GetInstanceExtensions(mSDLWindow, &extension_count, extensions.data()))
    {
        vLog_e("Could not get the names of required instance extensions from SDL.");
		return 1;
	}

	extensions.push_back("VK_KHR_get_physical_device_properties2");

	mVKInstance = std::make_unique<VulkanInstance>(extensions);

	// Create a Vulkan surface for rendering
	VkSurfaceKHR c_surface;
	if (!SDL_Vulkan_CreateSurface(mSDLWindow, static_cast<VkInstance>(mVKInstance->getInstance()), &c_surface))
    {
        vLog_e("Could not create a Vulkan surface.");
		return 1;
	}

	mVKsurfaceKHR = vk::SurfaceKHR(c_surface);
	// This is where most initialization for a program should be performed	

	return 0;
}

std::shared_ptr<VulkanContext> VulkanWindow::createContext(vk::PhysicalDevice& pDevice, const std::vector<const char*> & extensions)
{
	return mVKInstance->createContext(pDevice, extensions);
}

void VulkanWindow::updateEvents()
{
    mKeyStates = SDL_GetKeyboardState(NULL);
    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            mStillRunning = false;
            return;
        }

        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            mIsMouseDown = true;
        }

        if (event.type == SDL_MOUSEBUTTONUP)
        {
            mIsMouseDown = false;
        }

        handleEvent(event);
    }

    mKeyStates = SDL_GetKeyboardState(NULL);

    if (!mRelativeMouseMode)
    {
        mDeviceInput.mouseMove = mDeviceInput.mousePos;
        SDL_GetMouseState(&mDeviceInput.mousePos.x, &mDeviceInput.mousePos.y);
        mDeviceInput.mouseMove = mDeviceInput.mousePos - mDeviceInput.mouseMove;
    }
    else
    {
        SDL_GetRelativeMouseState(&mDeviceInput.mouseMove.x, &mDeviceInput.mouseMove.y);
    }
}

bool VulkanWindow::isStillRunning()
{
    return mStillRunning;
}

vk::Instance VulkanWindow::getInstance()
{
    return mVKInstance->getInstance();
}


void VulkanWindow::run(std::function<void()> update) {

	while (mStillRunning)
    {
        updateEvents();

		update();

		if (mKeyStates[SDL_SCANCODE_ESCAPE])
        {
            mStillRunning = false;
		}
	}
}

void VulkanWindow::lockPointer(bool toggle)
{
	SDL_SetRelativeMouseMode((SDL_bool)toggle);


	if (mRelativeMouseMode != toggle) {
		SDL_GetRelativeMouseState(&mDeviceInput.mouseMove.x, &mDeviceInput.mouseMove.y);
		mDeviceInput.mouseMove = ivec2(0, 0);
	}
	mRelativeMouseMode = toggle;
}

VulkanWindow::~VulkanWindow()
{

	mVKInstance->getInstance().destroySurfaceKHR(mVKsurfaceKHR);
	SDL_DestroyWindow(mSDLWindow);
	SDL_Quit();
}
