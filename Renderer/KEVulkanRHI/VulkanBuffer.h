/*
* Vulkan buffer class
*
* Encapsulates a Vulkan buffer
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanTools.h"
#include "VulkanDevice.h"



//GPUBuffer has to init after RHI since Logic device is created in the RHI
class GPUBuffer {

protected:
	VkDevice logicalDevice;

public:
	VkBuffer buffer;
	VkDeviceMemory memory;
	uint64_t size;
public:

	GPUBuffer():buffer(VK_NULL_HANDLE),
		memory(VK_NULL_HANDLE),
		size(0),
		logicalDevice(VK_NULL_HANDLE)
	{
	
	}

	GPUBuffer(uint64_t p_size, VkBufferUsageFlags usage, VkMemoryPropertyFlags p_flags):
		logicalDevice(VulkanDevice::GetVulkanDevice().logicalDevice),
		size(p_size)
	{

		//Align size to 0x100 bytes
		size = (size / 0x100 + 1) * 0x100;
		//Create A Buffer
		VkBufferCreateInfo l_buffer_create_info = {};
		l_buffer_create_info.pNext = nullptr;
		l_buffer_create_info.pQueueFamilyIndices = &(VulkanDevice::GetVulkanDevice().queueFamilyIndices.graphics);
		l_buffer_create_info.queueFamilyIndexCount = 1;
		l_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		l_buffer_create_info.size = size;
		l_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffer_create_info.usage = usage;
		vkCreateBuffer(logicalDevice, &l_buffer_create_info, nullptr, &buffer);

		//Alloc Memory for the buffer.
		VkMemoryRequirements l_reqs;
		vkGetBufferMemoryRequirements(logicalDevice, buffer, &l_reqs);
		VkMemoryAllocateInfo l_allc_memory_info = {};
		l_allc_memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		l_allc_memory_info.allocationSize = size;
		l_allc_memory_info.memoryTypeIndex = VulkanDevice::GetVulkanDevice().getMemoryType(l_reqs.memoryTypeBits, p_flags);
		vkAllocateMemory(logicalDevice, &l_allc_memory_info, nullptr, &memory);

		//Bind buffer to memory.

		vkBindBufferMemory(logicalDevice, buffer, memory, 0);

	}

	virtual ~GPUBuffer() {
	
	}

};


class UniformBuffer: public GPUBuffer {


private:
	void* m_data_host_ptr;
public:
	VkDescriptorBufferInfo buffer_info;

public:

	UniformBuffer(uint64_t p_size, VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags p_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT):
		GPUBuffer(p_size, usage, p_flags),
		m_data_host_ptr(nullptr)
	{
		
		vkMapMemory(logicalDevice, memory, 0, size, 0, &m_data_host_ptr);
		buffer_info.buffer = buffer;
		buffer_info.offset = 0;
		buffer_info.range = size;
	}

	INLINE void UpdateUniformBuffer(void* src,uint32_t offset = 0) {

		memmove(reinterpret_cast<uint8_t*>(m_data_host_ptr) + offset, src, size);
	}
	INLINE void UpdateUniformBuffer(void* src,uint32_t p_size, uint32_t offset = 0) {

		memmove(reinterpret_cast<uint8_t*>(m_data_host_ptr) + offset, src, p_size);
	}

	UniformBuffer():GPUBuffer()
	{

	}
	~UniformBuffer() {
		//vkUnmapMemory(logicalDevice, memory);
	}

};






struct Buffer
{
	VkDevice device;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo descriptor;
	VkDeviceSize size = 0;
	VkDeviceSize alignment = 0;
	void* mapped = nullptr;

	/** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
	VkBufferUsageFlags usageFlags;
	/** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
	VkMemoryPropertyFlags memoryPropertyFlags;

	/**
	* Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
	*
	* @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the buffer mapping call
	*/
	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
	{
		return vkMapMemory(device, memory, offset, size, 0, &mapped);
	}

	/**
	* Unmap a mapped memory range
	*
	* @note Does not return a result as vkUnmapMemory can't fail
	*/
	void unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(device, memory);
			mapped = nullptr;
		}
	}

	/**
	* Attach the allocated memory block to the buffer
	*
	* @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
	*
	* @return VkResult of the bindBufferMemory call
	*/
	VkResult bind(VkDeviceSize offset = 0)
	{
		return vkBindBufferMemory(device, buffer, memory, offset);
	}

	/**
	* Setup the default descriptor for this buffer
	*
	* @param size (Optional) Size of the memory range of the descriptor
	* @param offset (Optional) Byte offset from beginning
	*
	*/
	void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
	{
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range = size;
	}

	/**
	* Copies the specified data to the mapped buffer
	*
	* @param data Pointer to the data to copy
	* @param size Size of the data to copy in machine units
	*
	*/
	void copyTo(void* data, VkDeviceSize size)
	{
		assert(mapped);
		memcpy(mapped, data, size);
	}

	/**
	* Flush a memory range of the buffer to make it visible to the device
	*
	* @note Only required for non-coherent memory
	*
	* @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the flush call
	*/
	VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
	}

	/**
	* Invalidate a memory range of the buffer to make it visible to the host
	*
	* @note Only required for non-coherent memory
	*
	* @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the invalidate call
	*/
	VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
	}

	/**
	* Release all Vulkan resources held by this buffer
	*/
	void destroy()
	{
		if (buffer)
		{
			vkDestroyBuffer(device, buffer, nullptr);
		}
		if (memory)
		{
			vkFreeMemory(device, memory, nullptr);
		}
	}

};
