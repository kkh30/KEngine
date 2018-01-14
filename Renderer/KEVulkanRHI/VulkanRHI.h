#ifndef __VULKAN_RHI_H__
#define __VULKAN_RHI_H__

#define VK_USE_PLATFORM_WIN32_KHR
#include "KEVulkanRHI\vulkan\vulkan.h"
#include "RHI.h"
#include "VulkanTools.h"
#include "EngineConstans.h"
#include "KEWindow.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

namespace KEVulkanRHI {

	
	

	class VulkanRHI : public RHI
	{
	public:
		VulkanRHI();
		~VulkanRHI();
		virtual void Update() final override;
		virtual void Init() final override;


	private:
		VkInstance m_instance;
		VulkanDevice* m_vk_device;
		std::vector<VkPhysicalDevice> m_physical_devices;
		VkCommandPool m_graphics_cmd_pool;
		std::vector<const char*> m_vk_extensions;
		std::vector<const char*> m_vk_layers;
		VkSurfaceKHR m_surface;
		VulkanSwapChain m_swapChain;

	private:
		
		void InitInstance();
		void EnumeratePhaysicalDevices();
		void InitLogicDevice();
		void InitGraphicsCommandQueue();
		void SetupDebug();
		void InitSwapChain();
		int createVulkanSurface(VkInstance instance, SDL_Window* window, VkSurfaceKHR* surface);
		std::vector<const char*> getAvailableWSIExtensions();

	public:
		
	};



}













#endif