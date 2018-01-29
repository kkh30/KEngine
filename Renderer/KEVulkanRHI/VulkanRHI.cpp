#include "KEVulkanRHI\VulkanRHI.h"
#include "KEWindow.h"
#include "KELog.h"
#include "VulkanDebug.h"
namespace KEVulkanRHI {

	VkDevice logic_device = VK_NULL_HANDLE;


	VulkanRHI::VulkanRHI():
		m_instance(VK_NULL_HANDLE),
		m_vk_device(VulkanDevice::GetVulkanDevice()),
		m_graphics_cmd_pool(VK_NULL_HANDLE), 
		m_surface(VK_NULL_HANDLE),
		m_swapChain(VulkanSwapChain()),
		m_frame_rect(VkRect2D()),
		m_currentBuffer(0),
		m_graphics_queue(VK_NULL_HANDLE),
		m_graphics_pipeline(VK_NULL_HANDLE),
		m_pipeline_layout(VK_NULL_HANDLE),
		m_pipeline_cache(VK_NULL_HANDLE),
		m_vertex_index_buffer({0}),
		indices({0}), 
		m_camera_uniform({0})
	{
		m_frame_rect.offset.x = 0;
		m_frame_rect.offset.y = 0;
		m_frame_rect.extent.width = KEWindow::GetWindow().GetWidth();
		m_frame_rect.extent.height = KEWindow::GetWindow().GetHeight();
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
		InitPipelineLayout();
		InitUniforms();
		InitPiplineState();
		InitFrameBuffer();
		InitDrawCmdBuffers();
		PrepareSynchronizationPrimitives();
		prepareVertices();
		RecordDrawCmdBuffers();

	}

	void VulkanRHI::InitUniforms() {
		InitCameraUniforms();

	}


	void VulkanRHI::InitCameraUniforms() {

		//1. alloc desc_set for camera uniform.
		VkDescriptorSetAllocateInfo l_desc_set_alloc_info = {};
		l_desc_set_alloc_info.descriptorPool = m_desc_pool;
		l_desc_set_alloc_info.descriptorSetCount = 1;
		l_desc_set_alloc_info.pSetLayouts = &m_desc_set_layout;
		l_desc_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vkAllocateDescriptorSets(m_vk_device.logicalDevice, &l_desc_set_alloc_info, &m_camera_uniform.desc_set);

		
		//2.alloc uniform buffer for camera uniform.
		m_camera_uniform.buffer = UniformBuffer(sizeof(glm::mat4), VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);


		//3. update descriptor set.
		VkWriteDescriptorSet l_write_desc_set = {};
		l_write_desc_set.descriptorCount = 1;
		l_write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		l_write_desc_set.dstArrayElement = 0;
		l_write_desc_set.dstBinding = 0;
		l_write_desc_set.dstSet = m_camera_uniform.desc_set;
		l_write_desc_set.pBufferInfo = &m_camera_uniform.buffer.buffer_info;
		l_write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkUpdateDescriptorSets(m_vk_device.logicalDevice, 1,&l_write_desc_set, 0, nullptr);

		//4. update camera data
		m_camera_uniform.buffer.UpdateUniformBuffer(&KECamera::GetCamera().m_mvp.mvp);

	}

	void VulkanRHI::prepareVertices()
	{

		uint64_t vertexBufferSize = KEMemorySystem::GetMemorySystem().GetVertexDataSize();
		uint64_t indexBufferSize = KEMemorySystem::GetMemorySystem().GetIndexDataSize();

		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;

		void *data;

		struct StagingBuffer {
			VkDeviceMemory memory;
			VkBuffer buffer;
		};

		struct {
			StagingBuffer vertices;
			StagingBuffer indices;
		} stagingBuffers;

		// Vertex buffer
		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.size = vertexBufferSize;
		// Buffer is used as the copy source
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		// Create a host-visible buffer to copy the vertex data to (staging buffer)
		VK_CHECK_RESULT(vkCreateBuffer(m_vk_device.logicalDevice, &vertexBufferInfo, nullptr, &stagingBuffers.vertices.buffer));
		vkGetBufferMemoryRequirements(m_vk_device.logicalDevice, stagingBuffers.vertices.buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		// Request a host visible memory type that can be used to copy our data do
		// Also request it to be coherent, so that writes are visible to the GPU right after unmapping the buffer
		memAlloc.memoryTypeIndex = m_vk_device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_vk_device.logicalDevice, &memAlloc, nullptr, &stagingBuffers.vertices.memory));
		// Map and copy
		VK_CHECK_RESULT(vkMapMemory(m_vk_device.logicalDevice, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data));
		memcpy(data, KEMemorySystem::GetMemorySystem().GetVertexDataPtr(), vertexBufferSize);
		vkUnmapMemory(m_vk_device.logicalDevice, stagingBuffers.vertices.memory);
		VK_CHECK_RESULT(vkBindBufferMemory(m_vk_device.logicalDevice, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0));

		// Create a m_vk_device.logicalDevice local buffer to which the (host local) vertex data will be copied and which will be used for rendering
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ;
		VK_CHECK_RESULT(vkCreateBuffer(m_vk_device.logicalDevice, &vertexBufferInfo, nullptr, &m_vertex_index_buffer.buffer));
		vkGetBufferMemoryRequirements(m_vk_device.logicalDevice, m_vertex_index_buffer.buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = m_vk_device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_vk_device.logicalDevice, &memAlloc, nullptr, &m_vertex_index_buffer.memory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_vk_device.logicalDevice, m_vertex_index_buffer.buffer, m_vertex_index_buffer.memory, 0));

		// Index buffer
		VkBufferCreateInfo indexbufferInfo = {};
		indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexbufferInfo.size = KEMemorySystem::GetMemorySystem().GetIndexDataSize();
		indexbufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		// Copy index data to a buffer visible to the host (staging buffer)
		VK_CHECK_RESULT(vkCreateBuffer(m_vk_device.logicalDevice, &indexbufferInfo, nullptr, &stagingBuffers.indices.buffer));
		vkGetBufferMemoryRequirements(m_vk_device.logicalDevice, stagingBuffers.indices.buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = m_vk_device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_vk_device.logicalDevice, &memAlloc, nullptr, &stagingBuffers.indices.memory));
		VK_CHECK_RESULT(vkMapMemory(m_vk_device.logicalDevice, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data));
		memcpy(data, KEMemorySystem::GetMemorySystem().GetIndexDataPtr(), indexBufferSize);
		vkUnmapMemory(m_vk_device.logicalDevice, stagingBuffers.indices.memory);
		VK_CHECK_RESULT(vkBindBufferMemory(m_vk_device.logicalDevice, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0));
		
		// Create destination buffer with m_vk_device.logicalDevice only visibility
		indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VK_CHECK_RESULT(vkCreateBuffer(m_vk_device.logicalDevice, &indexbufferInfo, nullptr, &indices.buffer));
		vkGetBufferMemoryRequirements(m_vk_device.logicalDevice, indices.buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = m_vk_device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_vk_device.logicalDevice, &memAlloc, nullptr, &indices.memory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_vk_device.logicalDevice, indices.buffer, indices.memory, 0));

		VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.pNext = nullptr;

		// Buffer copies have to be submitted to a queue, so we need a command buffer for them
		// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
		VkCommandBuffer copyCmd = m_vk_device.createCommandBuffer(m_graphics_cmd_pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);//getCommandBuffer(true);

		// Put buffer region copies into command buffer
		VkBufferCopy copyRegion = {};

		// Vertex buffer
		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(copyCmd, stagingBuffers.vertices.buffer, m_vertex_index_buffer.buffer, 1, &copyRegion);
		// Index buffer
		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, indices.buffer, 1, &copyRegion);

		// Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
		//flushCommandBuffer(copyCmd);
		m_vk_device.flushCommandBuffer(m_graphics_cmd_pool,copyCmd, m_graphics_queue, true);
		// Destroy staging buffers
		// Note: Staging buffer must not be deleted before the copies have been submitted and executed
		vkDestroyBuffer(m_vk_device.logicalDevice, stagingBuffers.vertices.buffer, nullptr);
		vkFreeMemory(m_vk_device.logicalDevice, stagingBuffers.vertices.memory, nullptr);
		vkDestroyBuffer(m_vk_device.logicalDevice, stagingBuffers.indices.buffer, nullptr);
		vkFreeMemory(m_vk_device.logicalDevice, stagingBuffers.indices.memory, nullptr);
		
	}




	void VulkanRHI::InitPipelineLayout() {

		VkDescriptorPoolSize l_desc_pool_size = {};
		l_desc_pool_size.descriptorCount = 1;
		l_desc_pool_size.type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolCreateInfo l_desc_pool_create_info = {};
		l_desc_pool_create_info.maxSets = 1;
		l_desc_pool_create_info.poolSizeCount = 1;
		l_desc_pool_create_info.pPoolSizes = &l_desc_pool_size;
		l_desc_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		vkCreateDescriptorPool(m_vk_device.logicalDevice, &l_desc_pool_create_info, nullptr, &m_desc_pool);


		//Camera Uniform Buffer Binding
		VkDescriptorSetLayoutBinding l_binding0 = {};
		l_binding0.binding = 0;
		l_binding0.descriptorCount = 1;
		l_binding0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		l_binding0.pImmutableSamplers = nullptr;
		l_binding0.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo l_desc_set_layout_create_info = {};
		l_desc_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		l_desc_set_layout_create_info.bindingCount = 1;
		l_desc_set_layout_create_info.pBindings = &l_binding0;
		vkCreateDescriptorSetLayout(m_vk_device.logicalDevice, &l_desc_set_layout_create_info, nullptr, &m_desc_set_layout);

		

		VkPipelineLayoutCreateInfo l_pipeline_layout_create_info = {};
		l_pipeline_layout_create_info.pPushConstantRanges = nullptr;
		l_pipeline_layout_create_info.pSetLayouts = &m_desc_set_layout;
		l_pipeline_layout_create_info.setLayoutCount = 1;
		l_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		vkCreatePipelineLayout(m_vk_device.logicalDevice, &l_pipeline_layout_create_info, nullptr, &m_pipeline_layout);

	}



	void VulkanRHI::RecordDrawCmdBuffers() {
		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.pNext = nullptr;
		//cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		// Set clear values for all framebuffer attachments with loadOp set to clear
		// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
		VkClearValue clearValues[2];
		clearValues[0].color = { { 0.0f, 0.5f, 0.0f, 1.0f } };
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
			viewport.width = (float)m_frame_rect.extent.width;
			viewport.height = (float)m_frame_rect.extent.height;
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


			// Bind descriptor sets describing shader binding points
			vkCmdBindDescriptorSets(m_draw_cmd_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1,&m_camera_uniform.desc_set, 0, nullptr);
			
			// Bind the rendering pipeline
			// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
			vkCmdBindPipeline(m_draw_cmd_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
		
			VkDeviceSize offsets[1] = { 0 };

			vkCmdBindVertexBuffers(m_draw_cmd_buffers[i], 0, 1, &m_vertex_index_buffer.buffer, offsets);
			vkCmdBindIndexBuffer(m_draw_cmd_buffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);


			DrawGameScene(m_draw_cmd_buffers[i]);

			vkCmdEndRenderPass(m_draw_cmd_buffers[i]);


			//Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to 
			//VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

			VK_CHECK_RESULT(vkEndCommandBuffer(m_draw_cmd_buffers[i]));
		}
	}


	void VulkanRHI::DrawGameScene(VkCommandBuffer p_draw_command_buffer) {
		auto& all_entites = EntityManager::GetEntityManager().GetAllEntities();
		System<KERenderComponent>& render_component_system = System<KERenderComponent>::GetSystem();
		for (auto& entity : all_entites) {
			auto& render_component = render_component_system.GetEntityComponent(entity);
			//vkCmdDraw(p_draw_command_buffer, render_component.vertex_count, 1, render_component.first_vertex, 0);
			vkCmdDrawIndexed(p_draw_command_buffer, render_component.index_count, 1, render_component.first_index, render_component.first_index, 0);
		}
	}

	void VulkanRHI::InitPiplineState() {
		VkGraphicsPipelineCreateInfo l_pipeline_create_info = {};
		l_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
		l_pipeline_create_info.layout = m_pipeline_layout;
		l_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		//ColorBlendState
		//{
			VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
			blendAttachmentState[0].colorWriteMask = 0xf;
			blendAttachmentState[0].blendEnable = VK_FALSE;
			VkPipelineColorBlendStateCreateInfo colorBlendState = {};
			colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendState.attachmentCount = 1;
			colorBlendState.pAttachments = blendAttachmentState;
			l_pipeline_create_info.pColorBlendState = &colorBlendState;
		//}

		//Viewport State
		//{
			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;
			l_pipeline_create_info.pViewportState = &viewportState;
		//}
		
		// Input assembly state describes how primitives are assembled
		// This pipeline will assemble vertex data as a triangle lists (though we only use one triangle)
		//{
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			l_pipeline_create_info.pInputAssemblyState = &inputAssemblyState;

		//}
		// Rasterization state
		//{
			VkPipelineRasterizationStateCreateInfo rasterizationState = {};
			rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationState.cullMode = VK_CULL_MODE_NONE;
			rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizationState.depthClampEnable = VK_FALSE;
			rasterizationState.rasterizerDiscardEnable = VK_FALSE;
			rasterizationState.depthBiasEnable = VK_FALSE;
			rasterizationState.lineWidth = 1.0f;
			l_pipeline_create_info.pRasterizationState = &rasterizationState;

		//}
		// Dynamic states
		//{
			std::vector<VkDynamicState> dynamicStateEnables;
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
			VkPipelineDynamicStateCreateInfo dynamicState = {};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.pDynamicStates = dynamicStateEnables.data();
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
			l_pipeline_create_info.pDynamicState = &dynamicState;
		//}

		

		// Depth and Stencil
		//{
			VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
			depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilState.depthTestEnable = VK_TRUE;
			depthStencilState.depthWriteEnable = VK_TRUE;
			depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilState.depthBoundsTestEnable = VK_FALSE;
			depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
			depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
			depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
			depthStencilState.stencilTestEnable = VK_FALSE;
			depthStencilState.front = depthStencilState.back;
			l_pipeline_create_info.pDepthStencilState = &depthStencilState;

		//}
		

		// Multi sampling state
		//{
			VkPipelineMultisampleStateCreateInfo multisampleState = {};
			multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleState.pSampleMask = nullptr;
			l_pipeline_create_info.pMultisampleState = &multisampleState;

		//}

			// Vertex input binding
			// This example uses a single vertex input binding at binding point 0 (see vkCmdBindVertexBuffers)
			VkVertexInputBindingDescription vertexInputBinding = {};
			vertexInputBinding.binding = 0;
			vertexInputBinding.stride = sizeof(KEVertex);
			vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			// Inpute attribute bindings describe shader attribute locations and memory layouts
			std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributs;
			// These match the following shader layout (see triangle.vert):
			//	layout (location = 0) in vec3 inPos;
			//	layout (location = 1) in vec3 inColor;
			// Attribute location 0: Position
			vertexInputAttributs[0].binding = 0;
			vertexInputAttributs[0].location = 0;
			// Position attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
			vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributs[0].offset = offsetof(KEVertex, position);
			// Attribute location 1: Color
			vertexInputAttributs[1].binding = 0;
			vertexInputAttributs[1].location = 1;
			// Color attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
			vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributs[1].offset = offsetof(KEVertex, color);

			// Vertex input state used for pipeline creation
			VkPipelineVertexInputStateCreateInfo vertexInputState = {};
			vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputState.vertexBindingDescriptionCount = 1;
			vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
			vertexInputState.vertexAttributeDescriptionCount = 2;
			vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();
			l_pipeline_create_info.pVertexInputState = &vertexInputState;


		//}		

		// Shaders
		//{
			std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

			// Vertex shader
			shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			// Set pipeline stage for this shader
			shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			// Load binary SPIR-V shader
			shaderStages[0].module = tools::loadShader("D:/Dev/KEngine/shaders/triangle.vert.spv", m_vk_device.logicalDevice);
			// Main entry point for the shader
			shaderStages[0].pName = "main";
			assert(shaderStages[0].module != VK_NULL_HANDLE);

			// Fragment shader
			shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			// Set pipeline stage for this shader
			shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			// Load binary SPIR-V shader
			shaderStages[1].module = tools::loadShader("D:/Dev/KEngine/shaders/triangle.frag.spv", m_vk_device.logicalDevice);
			// Main entry point for the shader
			shaderStages[1].pName = "main";
			assert(shaderStages[1].module != VK_NULL_HANDLE);

			// Set pipeline shader stage info
			l_pipeline_create_info.stageCount = static_cast<uint32_t>(shaderStages.size());
			l_pipeline_create_info.pStages = shaderStages.data();
		//}
		
		VkPipelineCacheCreateInfo l_pipeline_cache_create_info = {};
		l_pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vkCreatePipelineCache(m_vk_device.logicalDevice, &l_pipeline_cache_create_info, nullptr, &m_pipeline_cache);
		
		l_pipeline_create_info.renderPass = m_renderPass;




		// Create rendering pipeline using the specified states
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_vk_device.logicalDevice, m_pipeline_cache, 1, &l_pipeline_create_info, nullptr, &m_graphics_pipeline));

	}



	VulkanRHI::~VulkanRHI()
	{
	}



	void VulkanRHI::Update() {
		// Get next image in the swap chain (back/front buffer)
		VK_CHECK_RESULT(m_swapChain.acquireNextImage(m_semaphores.presentCompleteSemaphore, &m_currentBuffer));

		// Use a fence to wait until the command buffer has finished execution before using it again
		VK_CHECK_RESULT(vkWaitForFences(m_vk_device.logicalDevice, 1, &m_waitFences[m_currentBuffer], VK_TRUE, UINT64_MAX));
		VK_CHECK_RESULT(vkResetFences(m_vk_device.logicalDevice, 1, &m_waitFences[m_currentBuffer]));

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
		//VK_CHECK_RESULT(vkQueueSubmit(m_graphics_queue, 1, &submitInfo, nullptr));

		// Present the current buffer to the swap chain
		// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
		// This ensures that the image is not presented to the windowing system until all commands have been submitted
		VK_CHECK_RESULT(m_swapChain.queuePresent(m_graphics_queue, m_currentBuffer, m_semaphores.renderCompleteSemaphore));
	}


	void VulkanRHI::PrepareSynchronizationPrimitives()
	{
		// Semaphores (Used for correct command ordering)
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;

		// Semaphore used to ensures that image presentation is complete before starting to submit again
		VK_CHECK_RESULT(vkCreateSemaphore(m_vk_device.logicalDevice, &semaphoreCreateInfo, nullptr, &m_semaphores.presentCompleteSemaphore));

		// Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
		VK_CHECK_RESULT(vkCreateSemaphore(m_vk_device.logicalDevice, &semaphoreCreateInfo, nullptr, &m_semaphores.renderCompleteSemaphore));

		// Fences (Used to check draw command buffer completion)
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// Create in signaled state so we don't wait on first render of each command buffer
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		m_waitFences.resize(m_draw_cmd_buffers.size());
		for (auto& fence : m_waitFences)
		{
			VK_CHECK_RESULT(vkCreateFence(m_vk_device.logicalDevice, &fenceCreateInfo, nullptr, &fence));
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
		vkAllocateCommandBuffers(m_vk_device.logicalDevice, &l_draw_cmd_allc_info, m_draw_cmd_buffers.data());
	}

	void VulkanRHI::InitCmdQueue() {
		vkGetDeviceQueue(m_vk_device.logicalDevice, m_vk_device.getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT), 0, &m_graphics_queue);
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

		VK_CHECK_RESULT(vkCreateImage(m_vk_device.logicalDevice, &image, nullptr, &m_depth_stencil_buffer.image));
		vkGetImageMemoryRequirements(m_vk_device.logicalDevice, m_depth_stencil_buffer.image, &memReqs);
		mem_alloc.allocationSize = memReqs.size;
		mem_alloc.memoryTypeIndex = m_vk_device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_vk_device.logicalDevice, &mem_alloc, nullptr, &m_depth_stencil_buffer.mem));
		VK_CHECK_RESULT(vkBindImageMemory(m_vk_device.logicalDevice, m_depth_stencil_buffer.image, m_depth_stencil_buffer.mem, 0));

		depthStencilView.image = m_depth_stencil_buffer.image;
		VK_CHECK_RESULT(vkCreateImageView(m_vk_device.logicalDevice, &depthStencilView, nullptr, &m_depth_stencil_buffer.view));
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
			VK_CHECK_RESULT(vkCreateFramebuffer(m_vk_device.logicalDevice, &frameBufferCreateInfo, nullptr, &m_frameBuffers[i]));
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

		VK_CHECK_RESULT(vkCreateRenderPass(m_vk_device.logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
	}

	void VulkanRHI::InitSwapChain() {
		m_swapChain.connect(m_instance, m_physical_devices[0], m_vk_device.logicalDevice);
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
		l_cmd_pool_info.queueFamilyIndex = m_vk_device.queueFamilyIndices.graphics;
		l_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

		vkCreateCommandPool(m_vk_device.logicalDevice, &l_cmd_pool_info, nullptr, &m_graphics_cmd_pool);
	

	
	}

	void VulkanRHI::InitLogicDevice() {
		m_vk_device.CreateDevice(m_physical_devices[0]);
		std::vector<const char*> l_extensions;
		m_vk_device.createLogicalDevice(m_vk_device.features, l_extensions);

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



