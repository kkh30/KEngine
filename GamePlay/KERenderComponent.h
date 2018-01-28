#ifndef _KERENDERER_COMPONENT_H__
#define _KERENDERER_COMPONENT_H__
#include <stdint.h>
#include "KEMemorySystem.h"

struct KEVertex
{
	float position[3];
	float color[3];
};




struct KERenderComponent
{
	//uint64_t vertex_offset_in_byte;
	//uint64_t index_offset_in_byte;
	uint64_t vertex_size;
	uint64_t index_size;
	uint32_t vertex_count;
	uint32_t index_count;
	//Todo::Replace With GameScene Info
	static uint32_t total_vertex_num;
	static uint32_t total_index_num;
	uint32_t first_vertex;
	uint32_t first_index;

	KERenderComponent(std::vector<KEVertex>&& p_vertices,std::vector<uint32_t>&& p_indices) :
		vertex_size(sizeof(KEVertex) * p_vertices.size()),
		index_size(sizeof(uint32_t) * p_indices.size()),
		index_count(p_indices.size()),
		vertex_count(uint32_t(p_vertices.size())),
		first_index(0),
		first_vertex(0)
	{
		auto& l_memory_system = KEMemorySystem::GetMemorySystem();
		//vertex_offset_in_byte = l_memory_system.UploadToVRAM(p_vertices.data(), vertex_size);
		//index_offset_in_byte = l_memory_system.UploadToVRAM(p_indices.data(), index_size);
		l_memory_system.UploadVertexDataToVRAM(p_vertices.data(), vertex_size);
		l_memory_system.UploadIndexDataToVRAM(p_indices.data(), index_size);
		first_index = total_index_num;
		first_vertex = total_vertex_num;
		total_index_num += vertex_count;
		total_vertex_num += index_count;
	}


	KERenderComponent() {
	}
};












#endif