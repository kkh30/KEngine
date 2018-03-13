#ifndef __VULKAN_RHI_H__
#define __VULKAN_RHI_H__

#define VK_USE_PLATFORM_WIN32_KHR
#include "Camera\KECamera.h"
#include "VulkanBuffer.h"
#include "KEMemorySystem.h"
#include "ECS/KEEntity.h"
#include "ECS/KESystem.h"
#include "ECS/KERenderComponent.h"
#include "KEVulkanRHI\vulkan\vulkan.h"
#include "RHI.h"
#include "VulkanTools.h"
#include "EngineConstans.h"
#include "KEWindow.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanConstants.h"

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
		VulkanDevice& m_vk_device;
		std::vector<VkPhysicalDevice> m_physical_devices;
		VkCommandPool m_graphics_cmd_pool;
		std::vector<const char*> m_vk_extensions;
		std::vector<const char*> m_vk_layers;
		VkSurfaceKHR m_surface;
		VulkanSwapChain m_swapChain;
		std::vector<VkCommandBuffer> m_draw_cmd_buffers;
		VkCommandBuffer m_shadow_map_cmd;
		struct DepthStencilBuffer{
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		}m_depth_stencil_buffer;

		DepthStencilBuffer m_shadowmap_buffer;

		VkRenderPass m_renderPass;
		VkRenderPass m_shadowmapPass;

		std::vector<VkFramebuffer> m_frameBuffers;
		VkFramebuffer m_shadowmap_framebuffer;
		VkRect2D m_frame_rect;
		struct SynchronizationSemaphore {
			VkSemaphore presentCompleteSemaphore;
			VkSemaphore renderCompleteSemaphore;
			VkSemaphore shadowmapCompleteSemaphore;

		}m_semaphores;
		std::vector<VkFence> m_waitFences;
		uint32_t m_currentBuffer;
		VkQueue m_graphics_queue;

		std::array<VkPipeline,2> m_graphics_pipeline;
		VkDescriptorPool m_desc_pool;
		VkPipelineLayout m_pipeline_layout;
		VkPipelineCache m_pipeline_cache;
		VkDescriptorSetLayout m_desc_set_layout;
		// Vertex buffer and attributes
		struct {
			VkDeviceMemory memory;															// Handle to the device memory for this buffer
			VkBuffer buffer;																// Handle to the Vulkan buffer object that the memory is bound to
		} m_vertex_index_buffer;

		// Index buffer
		struct
		{
			VkDeviceMemory memory;
			VkBuffer buffer;
			uint32_t count;
		} indices;

		// Uniforms
		struct VKUniform {
			VkDescriptorSet desc_set;
			UniformBuffer buffer;
		};

		VKUniform m_camera_uniform;
		
	private:
		void ClearScreen();


	private:
		void InitInstance();
		void EnumeratePhaysicalDevices();
		void InitLogicDevice();
		void InitGraphicsCommandPool();
		void SetupDebug();
		void InitSwapChain();
		void InitDepthStencil();
		void InitFrameBuffer();
		void InitCmdQueue();
		void InitRenderPass();
		void InitPiplineState();
		void InitDrawCmdBuffers();
		void RecordDrawCmdBuffers();
		void RecodrShadowCmdBuffers();
		void PrepareSynchronizationPrimitives();
		void InitPipelineLayout();
		void prepareVertices();
		void DrawGameScene(VkCommandBuffer p_draw_command_buffer);
		void InitUniforms();
		void InitCameraUniforms();
		void InitShadowMapPass();

		int createVulkanSurface(VkInstance instance, SDL_Window* window, VkSurfaceKHR* surface);
		std::vector<const char*> getAvailableWSIExtensions();

	public:
		
	};



}













#endif