#include "KEMemorySystem.h"



KEMemorySystem::KEMemorySystem():
m_vertex_data_size(KEConstant::VRAM_SIZE),
m_vertex_data(new uint8_t[KEConstant::VRAM_SIZE]),
m_index_data_size(KEConstant::VRAM_SIZE),
m_index_data(new uint8_t[KEConstant::VRAM_SIZE]),
m_vertex_offset(0),
m_index_offset(0)
{
}

uint64_t KEMemorySystem::UploadVertexDataToVRAM(void* src,uint64_t size) {
	assert(size + m_vertex_offset <= m_vertex_data_size && "Not Enough VRAM\n");
	const uint64_t l_current_offset = m_vertex_offset;
	memcpy((uint8_t*)m_vertex_data + m_vertex_offset, src, size);
	m_vertex_offset += size;
	return l_current_offset;
}

uint64_t KEMemorySystem::UploadIndexDataToVRAM(void* src, uint64_t size) {
	assert(size + m_index_offset <= m_index_data_size && "Not Enough VRAM\n");
	const uint64_t l_current_offset = m_index_offset;
	memcpy((uint8_t*)m_index_data + m_index_offset, src, size);
	m_index_offset += size;
	return l_current_offset;
}


KEMemorySystem::~KEMemorySystem()
{
	if(m_vertex_data)
		delete[] m_vertex_data;
	m_vertex_data = nullptr;

	if (m_index_data)
		delete[] m_index_data;
	m_index_data = nullptr;
}
