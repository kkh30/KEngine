#ifndef _KERENDERER_COMPONENT_H__
#define _KERENDERER_COMPONENT_H__
#include <stdint.h>
#include "KEMemorySystem.h"

struct KEVertex
{
	float position[3];
	float color[3];
};

struct KRenderComponent
{
	uint64_t offset;
	uint64_t size;

	KRenderComponent(std::vector<KEVertex>&& p_vertices) :size(sizeof(KEVertex) * p_vertices.size()){
		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
		offset = l_memory_system.UploadToVRAM(p_vertices.data(), size);
	}
};












#endif