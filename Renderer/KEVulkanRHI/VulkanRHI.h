#ifndef __VULKAN_RHI_H__
#define __VULKAN_RHI_H__

#include "RHI.h"
#include "vulkan\vulkan.h"
#include "EngineConstans.h"

namespace KEVulkanRHI {

	
	

	class VulkanRHI : public RHI
	{
	public:
		VulkanRHI();
		~VulkanRHI();
		virtual void Update() final override;
		virtual void Init() final override;

	private:

	};



}













#endif