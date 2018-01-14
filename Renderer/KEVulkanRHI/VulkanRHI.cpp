#include "KEVulkanRHI\VulkanRHI.h"
#include "KEWindow.h"
#include "KELog.h"
#include "VulkanDebug.h"
namespace KEVulkanRHI {

	VkDevice logic_device = VK_NULL_HANDLE;


	VulkanRHI::VulkanRHI():
		m_instance(VK_NULL_HANDLE),
		m_vk_device(nullptr),
		m_graphics_cmd_pool(VK_NULL_HANDLE), 
		m_surface(VK_NULL_HANDLE),
		m_swapChain(VulkanSwapChain())
	{
	}

	VulkanRHI::~VulkanRHI()
	{
	}

	void VulkanRHI::Update() {

	
	}

	void  VulkanRHI::Init() {

		KELog::Log("Init VulkanRHI");
		InitInstance();
		SetupDebug();
		EnumeratePhaysicalDevices();
		InitLogicDevice();
		InitSwapChain();
		InitGraphicsCommandQueue();


	}

	void VulkanRHI::InitSwapChain() {
		m_swapChain.connect(m_instance, m_physical_devices[0], m_vk_device->logicalDevice);
		m_swapChain.initVKSurfaceWithSystemSurface(m_surface);
		uint32_t l_width = KEWindow::GetWindow().GetWidth();
		uint32_t l_heigth = KEWindow::GetWindow().GetHeight();
		m_swapChain.create(&l_width, &l_heigth);
	}

	void VulkanRHI::InitInstance() {
		// Use validation layers if this is a debug build, and use WSI extensions regardless
		m_vk_extensions = getAvailableWSIExtensions();
		m_vk_layers.push_back("VK_LAYER_LUNARG_standard_validation");
#if defined(_DEBUG)
		m_vk_extensions.push_back("VK_EXT_debug_report");

#endif

		// VkApplicationInfo allows the programmer to specifiy some basic information about the
		// program, which can be useful for layers and tools to provide more debug information.
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = NULL;
		appInfo.pApplicationName = "KEngine-VulkanRenderer";
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
		instInfo.enabledExtensionCount = static_cast<uint32_t>(m_vk_extensions.size());
		instInfo.ppEnabledExtensionNames = m_vk_extensions.data();
		instInfo.enabledLayerCount = static_cast<uint32_t>(m_vk_layers.size());
		instInfo.ppEnabledLayerNames = m_vk_layers.data();

		// Create the Vulkan instance.
		VkResult result = vkCreateInstance(&instInfo, NULL, &m_instance);
		assert(result == VK_SUCCESS);



		// Create a Vulkan surface for rendering
		createVulkanSurface(m_instance, KEWindow::GetWindow().GetSDLWindow(), &m_surface);

	}


	void VulkanRHI::SetupDebug() {
		// The report flags determine what type of messages for the layers will be displayed
		// For validating (debugging) an appplication the error and warning bits should suffice
		VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT| VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
		// Additional flags include performance info, loader and layer debug messages, etc.
		VkDebug::setupDebugging(m_instance, debugReportFlags, VK_NULL_HANDLE);
	}

	void VulkanRHI::InitGraphicsCommandQueue() {
		VkCommandPoolCreateInfo l_cmd_pool_info = {};
		l_cmd_pool_info.flags = 0;
		l_cmd_pool_info.queueFamilyIndex = m_vk_device->queueFamilyIndices.graphics;
		l_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

		vkCreateCommandPool(m_vk_device->logicalDevice, &l_cmd_pool_info, nullptr, &m_graphics_cmd_pool);
	}

	void VulkanRHI::InitLogicDevice() {
		m_vk_device = new VulkanDevice(m_physical_devices[0]);
		std::vector<const char*> l_extensions;
		m_vk_device->createLogicalDevice(m_vk_device->features, l_extensions);

	}


	void VulkanRHI::EnumeratePhaysicalDevices() {
		uint32_t gpuCount = 0;
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
		// Enumerate devices
		m_physical_devices.resize(gpuCount);
		KELog::Log("\nAvailable Vulkan devices\n");
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &gpuCount, m_physical_devices.data()));
		std::string l_window_title = KEWindow::GetWindow().GetWindowTitle();

		for (uint32_t i = 0; i < gpuCount; i++) {
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(m_physical_devices[i], &deviceProperties);
			KELog::Log("Device [ %s ] : \n",deviceProperties.deviceName);
			l_window_title += std::string(deviceProperties.deviceName);
		}

		SDL_SetWindowTitle(KEWindow::GetWindow().GetSDLWindow(), (l_window_title + std::string("-Vulkan")).c_str());

	}






	int VulkanRHI::createVulkanSurface(VkInstance instance, SDL_Window* window, VkSurfaceKHR* surface)
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
			assert(result == VK_SUCCESS);
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




	std::vector<const char*> VulkanRHI::getAvailableWSIExtensions()
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




}//namespace KEVulkanRHI



