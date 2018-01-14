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
		m_swapChain(VulkanSwapChain()),
		m_frame_rect(VkRect2D()),
		m_currentBuffer(0),
		m_graphics_queue(VK_NULL_HANDLE)
	{
		m_frame_rect.offset.x = 0;
		m_frame_rect.offset.y = 0;
		m_frame_rect.extent.width = KEWindow::GetWindow().GetWidth();
		m_frame_rect.extent.height = KEWindow::GetWindow().GetHeight();
	}

	void VulkanRHI::RecordDrawCmdBuffers() {
		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.pNext = nullptr;

		// Set clear values for all framebuffer attachments with loadOp set to clear
		// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
		VkClearValue clearValues[2];
		clearValues[0].color = { { 0.3f, 0.5f, 0.6f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.renderArea = m_frame_rect;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;

		for (int32_t i = 0; i < m_draw_cmd_buffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = m_frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(m_draw_cmd_buffers[i], &cmdBufInfo));

			// Start the first sub pass specified in our default render pass setup by the base class
			// This will clear the color and depth attachment
			vkCmdBeginRenderPass(m_draw_cmd_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Update dynamic viewport state
			VkViewport viewport = {};
			viewport.height = (float)m_frame_rect.extent.width;
			viewport.width  = (float)m_frame_rect.extent.height;
			viewport.minDepth = (float) 0.0f;
			viewport.maxDepth = (float) 1.0f;
			vkCmdSetViewport(m_draw_cmd_buffers[i], 0, 1, &viewport);

			// Update dynamic scissor state
			VkRect2D scissor = {};
			scissor.extent.width = m_frame_rect.extent.width;
			scissor.extent.height = m_frame_rect.extent.height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(m_draw_cmd_buffers[i], 0, 1, &scissor);

			//// Bind descriptor sets describing shader binding points
			//vkCmdBindDescriptorSets(m_draw_cmd_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
			//
			//// Bind the rendering pipeline
			//// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
			//vkCmdBindPipeline(m_draw_cmd_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			//
			//// Bind triangle vertex buffer (contains position and colors)
			//VkDeviceSize offsets[1] = { 0 };
			//vkCmdBindVertexBuffers(m_draw_cmd_buffers[i], 0, 1, &vertices.buffer, offsets);
			//
			//// Bind triangle index buffer
			//vkCmdBindIndexBuffer(m_draw_cmd_buffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			//
			//// Draw indexed triangle
			//vkCmdDrawIndexed(m_draw_cmd_buffers[i], indices.count, 1, 0, 0, 1);

			vkCmdEndRenderPass(m_draw_cmd_buffers[i]);

			// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to 
			// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

			VK_CHECK_RESULT(vkEndCommandBuffer(m_draw_cmd_buffers[i]));
		}
	}


	VulkanRHI::~VulkanRHI()
	{
	}



	void VulkanRHI::Update() {
		// Get next image in the swap chain (back/front buffer)
		VK_CHECK_RESULT(m_swapChain.acquireNextImage(m_semaphores.presentCompleteSemaphore, &m_currentBuffer));

		// Use a fence to wait until the command buffer has finished execution before using it again
		VK_CHECK_RESULT(vkWaitForFences(m_vk_device->logicalDevice, 1, &m_waitFences[m_currentBuffer], VK_TRUE, UINT64_MAX));
		VK_CHECK_RESULT(vkResetFences(m_vk_device->logicalDevice, 1, &m_waitFences[m_currentBuffer]));

		// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// The submit info structure specifices a command buffer queue submission batch
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitStageMask;									// Pointer to the list of pipeline stages that the semaphore waits will occur at
		submitInfo.pWaitSemaphores = &m_semaphores.presentCompleteSemaphore;							// Semaphore(s) to wait upon before the submitted command buffer starts executing
		submitInfo.waitSemaphoreCount = 1;												// One wait semaphore																				
		submitInfo.pSignalSemaphores = &m_semaphores.renderCompleteSemaphore;						// Semaphore(s) to be signaled when command buffers have completed
		submitInfo.signalSemaphoreCount = 1;											// One signal semaphore
		submitInfo.pCommandBuffers = &m_draw_cmd_buffers[m_currentBuffer];					// Command buffers(s) to execute in this batch (submission)
		submitInfo.commandBufferCount = 1;												// One command buffer

																						// Submit to the graphics queue passing a wait fence
		VK_CHECK_RESULT(vkQueueSubmit(m_graphics_queue, 1, &submitInfo, m_waitFences[m_currentBuffer]));

		// Present the current buffer to the swap chain
		// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
		// This ensures that the image is not presented to the windowing system until all commands have been submitted
		VK_CHECK_RESULT(m_swapChain.queuePresent(m_graphics_queue, m_currentBuffer, m_semaphores.renderCompleteSemaphore));
	}

	void  VulkanRHI::Init() {

		KELog::Log("Init VulkanRHI");
		InitInstance();
		SetupDebug();
		EnumeratePhaysicalDevices();
		InitLogicDevice();
		InitCmdQueue();
		InitSwapChain();
		InitGraphicsCommandPool();
		InitDepthStencil();
		InitRenderPass();
		InitFrameBuffer();
		InitDrawCmdBuffers();
		PrepareSynchronizationPrimitives();
		RecordDrawCmdBuffers();

	}

	void VulkanRHI::PrepareSynchronizationPrimitives()
	{
		// Semaphores (Used for correct command ordering)
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;

		// Semaphore used to ensures that image presentation is complete before starting to submit again
		VK_CHECK_RESULT(vkCreateSemaphore(m_vk_device->logicalDevice, &semaphoreCreateInfo, nullptr, &m_semaphores.presentCompleteSemaphore));

		// Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
		VK_CHECK_RESULT(vkCreateSemaphore(m_vk_device->logicalDevice, &semaphoreCreateInfo, nullptr, &m_semaphores.renderCompleteSemaphore));

		// Fences (Used to check draw command buffer completion)
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// Create in signaled state so we don't wait on first render of each command buffer
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		m_waitFences.resize(m_draw_cmd_buffers.size());
		for (auto& fence : m_waitFences)
		{
			VK_CHECK_RESULT(vkCreateFence(m_vk_device->logicalDevice, &fenceCreateInfo, nullptr, &fence));
		}
	}


	void VulkanRHI::InitDrawCmdBuffers() {
		m_draw_cmd_buffers.resize(m_swapChain.imageCount);
		VkCommandBufferAllocateInfo l_draw_cmd_allc_info;
		l_draw_cmd_allc_info.pNext = nullptr;
		l_draw_cmd_allc_info.commandBufferCount = m_swapChain.imageCount;
		l_draw_cmd_allc_info.commandPool = m_graphics_cmd_pool;
		l_draw_cmd_allc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		l_draw_cmd_allc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vkAllocateCommandBuffers(m_vk_device->logicalDevice, &l_draw_cmd_allc_info, m_draw_cmd_buffers.data());
	}

	void VulkanRHI::InitCmdQueue() {
		vkGetDeviceQueue(m_vk_device->logicalDevice, m_vk_device->getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT), 0, &m_graphics_queue);
	}



	void VulkanRHI::ClearScreen() {
	
	}

	void VulkanRHI::InitDepthStencil(){
		VkImageCreateInfo image = {};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.pNext = NULL;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = DefaultDepthStencilFormat;
		image.extent = { KEWindow::GetWindow().GetWidth(), KEWindow::GetWindow().GetHeight(), 1 };
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		image.flags = 0;

		VkMemoryAllocateInfo mem_alloc = {};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkImageViewCreateInfo depthStencilView = {};
		depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthStencilView.pNext = NULL;
		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilView.format = DefaultDepthStencilFormat;
		depthStencilView.flags = 0;
		depthStencilView.subresourceRange = {};
		depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		depthStencilView.subresourceRange.baseMipLevel = 0;
		depthStencilView.subresourceRange.levelCount = 1;
		depthStencilView.subresourceRange.baseArrayLayer = 0;
		depthStencilView.subresourceRange.layerCount = 1;

		VkMemoryRequirements memReqs;

		VK_CHECK_RESULT(vkCreateImage(m_vk_device->logicalDevice, &image, nullptr, &m_depth_stencil_buffer.image));
		vkGetImageMemoryRequirements(m_vk_device->logicalDevice, m_depth_stencil_buffer.image, &memReqs);
		mem_alloc.allocationSize = memReqs.size;
		mem_alloc.memoryTypeIndex = m_vk_device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_vk_device->logicalDevice, &mem_alloc, nullptr, &m_depth_stencil_buffer.mem));
		VK_CHECK_RESULT(vkBindImageMemory(m_vk_device->logicalDevice, m_depth_stencil_buffer.image, m_depth_stencil_buffer.mem, 0));

		depthStencilView.image = m_depth_stencil_buffer.image;
		VK_CHECK_RESULT(vkCreateImageView(m_vk_device->logicalDevice, &depthStencilView, nullptr, &m_depth_stencil_buffer.view));
	}
	void VulkanRHI::InitFrameBuffer() {
		VkImageView attachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		attachments[1] = m_depth_stencil_buffer.view;

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = NULL;
		frameBufferCreateInfo.renderPass = m_renderPass;
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = KEWindow::GetWindow().GetWidth();
		frameBufferCreateInfo.height = KEWindow::GetWindow().GetHeight();
		frameBufferCreateInfo.layers = 1;

		// Create frame buffers for every swap chain image
		m_frameBuffers.resize(m_swapChain.imageCount);
		for (uint32_t i = 0; i < m_frameBuffers.size(); i++)
		{
			attachments[0] = m_swapChain.buffers[i].view;
			VK_CHECK_RESULT(vkCreateFramebuffer(m_vk_device->logicalDevice, &frameBufferCreateInfo, nullptr, &m_frameBuffers[i]));
		}
	}
	void VulkanRHI::InitRenderPass()  {
		std::array<VkAttachmentDescription, 2> attachments = {};
		// Color attachment
		attachments[0].format = m_swapChain.colorFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		attachments[1].format = DefaultDepthStencilFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(m_vk_device->logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
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

	void VulkanRHI::InitGraphicsCommandPool() {


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



