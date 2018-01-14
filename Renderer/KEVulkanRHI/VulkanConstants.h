#ifndef __VULKAN_CONSTANTS_H__
#define __VULKAN_CONSTANTS_H__
#include "vulkan\vulkan.h"

namespace KEVulkanRHI {


	const VkFormat DefaultDepthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;

	template<typename T>
	struct Rect{
		T x;
		T y;
		T width;
		T height;

		Rect():x(T()), y(T()), width(T()), height(T()) {
		
		}
	};

	using RectI = Rect<uint32_t>;
}



#endif