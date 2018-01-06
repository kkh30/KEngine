#include "KEVulkanRHI\VulkanRHI.h"
#include "KEWindow.h"
#include "KELog.h"
namespace KEVulkanRHI {

	VkDevice logic_device = VK_NULL_HANDLE;


	VulkanRHI::VulkanRHI()
	{
	}

	VulkanRHI::~VulkanRHI()
	{
	}

	void VulkanRHI::Update() {

	}

	void  VulkanRHI::Init() {

		KELog::Log("Init VulkanRHI");

		VkDeviceCreateInfo l_device_create_info = {};

		l_device_create_info.




	}


}//namespace KEVulkanRHI



