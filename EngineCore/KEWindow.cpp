#include "KEWindow.h"
#include <assert.h>
#include <vector>



#ifdef VULKAN_RENDERER

int createVulkanSurface(VkInstance instance, SDL_Window* window, VkSurfaceKHR* surface);
std::vector<const char*> getAvailableWSIExtensions();

#endif // VULKAN_RENDERER


void KEWindow::CreateKEWindow(int p_x , int p_y ,int p_width , int p_height, SDL_WindowFlags p_flags){

	m_width = p_width;
	m_height = p_height;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		assert(0);
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		assert(0);

	}

	if ((m_SDLWindow = SDL_CreateWindow(
		"KEngine", p_x, p_y,
		p_width, p_height, p_flags)
		) == NULL) {
		assert(0);

	}

#ifdef VULKAN_RENDERER
	// Use validation layers if this is a debug build, and use WSI extensions regardless
	std::vector<const char*> extensions = getAvailableWSIExtensions();
	std::vector<const char*> layers;
#if defined(_DEBUG)
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	// VkApplicationInfo allows the programmer to specifiy some basic information about the
	// program, which can be useful for layers and tools to provide more debug information.
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "Vulkan Program Template";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "LunarG SDK";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// VkInstanceCreateInfo is where the programmer specifies the layers and/or extensions that
	// are needed.
	VkInstanceCreateInfo instInfo = {};
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instInfo.pNext = NULL;
	instInfo.flags = 0;
	instInfo.pApplicationInfo = &appInfo;
	instInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instInfo.ppEnabledExtensionNames = extensions.data();
	instInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	instInfo.ppEnabledLayerNames = layers.data();

	// Create the Vulkan instance.
	VkInstance instance;
	VkResult result = vkCreateInstance(&instInfo, NULL, &instance);
	assert(result == VK_SUCCESS);

	// Create a Vulkan surface for rendering
	VkSurfaceKHR surface;
	createVulkanSurface(instance, m_SDLWindow, &surface);

#endif // VULKAN_RENDERER



}
void KEWindow::Show() {


	while (is_running) {

		//Call Renderer Func
		m_renderer_update_func();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

			case SDL_QUIT:
				is_running = false;
				break;

			default:
				// Do nothing.
				break;
			}
		}

	}

}



KEWindow::KEWindow():is_running(true)
{
	
}

KEWindow::~KEWindow()
{
}


#ifdef VULKAN_RENDERER



int createVulkanSurface(VkInstance instance, SDL_Window* window, VkSurfaceKHR* surface)
{
	SDL_SysWMinfo windowInfo;
	SDL_VERSION(&windowInfo.version);
	if (!SDL_GetWindowWMInfo(window, &windowInfo)) {
		return 1;
	}

	switch (windowInfo.subsystem) {

#if defined(SDL_VIDEO_DRIVER_ANDROID) && defined(VK_USE_PLATFORM_ANDROID_KHR)
	case SDL_SYSWM_ANDROID: {
		VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.window = windowInfo.info.android.window;

		VkResult result = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, NULL, surface);
		if (result != VK_SUCCESS) {
			std::cout << "Failed to create Android surface." << std::endl;
			return 1;
		}
		break;
	}
#endif

#if defined(SDL_VIDEO_DRIVER_MIR) && defined(VK_USE_PLATFORM_MIR_KHR)
	case SDL_SYSWM_MIR: {
		VkMirSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_MIR_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.connection = windowInfo.info.mir.connection;
		surfaceInfo.mirSurface = windowInfo.info.mir.surface;

		VkResult result = vkCreateMirSurfaceKHR(instance, &surfaceInfo, NULL, surface);
		if (result != VK_SUCCESS) {
			std::cout << "Failed to create Mir surface." << std::endl;
			return 1;
		}
		break;
	}
#endif

#if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
	case SDL_SYSWM_WAYLAND: {
		VkWaylandSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.display = windowInfo.info.wl.display;
		surfaceInfo.surface = windowInfo.info.wl.surface;

		VkResult result = vkCreateWaylandSurfaceKHR(instance, &surfaceInfo, NULL, surface);
		if (result != VK_SUCCESS) {
			std::cout << "Failed to create Wayland surface." << std::endl;
			return 1;
		}
		break;
	}
#endif

#if defined(SDL_VIDEO_DRIVER_WINDOWS) && defined(VK_USE_PLATFORM_WIN32_KHR)
	case SDL_SYSWM_WINDOWS: {
		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = GetModuleHandle(NULL);
		surfaceInfo.hwnd = windowInfo.info.win.window;

		VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, NULL, surface);
		if (result != VK_SUCCESS) {
			std::cout << "Failed to create Win32 surface." << std::endl;
			return 1;
		}
		break;
	}
#endif

#if defined(SDL_VIDEO_DRIVER_X11) && defined(VK_USE_PLATFORM_XLIB_KHR)
	case SDL_SYSWM_X11: {
		VkXlibSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.dpy = windowInfo.info.x11.display;
		surfaceInfo.window = windowInfo.info.x11.window;

		VkResult result = vkCreateXlibSurfaceKHR(instance, &surfaceInfo, NULL, surface);
		if (result != VK_SUCCESS) {
			std::cout << "Failed to create X11 surface." << std::endl;
			return 1;
		}
		break;
	}
#endif

	default:
		return 1;
	}

	return 0;
}




std::vector<const char*> getAvailableWSIExtensions()
{
	std::vector<const char*> extensions;
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
	extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

	return extensions;
}

#endif // VULKAN_RENDERER
