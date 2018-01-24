#include "KEMemorySystem.h"



KEMemorySystem::KEMemorySystem():m_size(KEConstant::VRAM_SIZE),
m_data(new uint8_t[KEConstant::VRAM_SIZE]),
m_offset(0)
{
}

uint64_t KEMemorySystem::UploadToVRAM(void* src,uint64_t size) {
	assert(size + m_offset <= m_size && "Not Enough VRAM\n");
	const uint64_t l_current_offset = m_offset;
	memcpy((uint8_t*)m_data + m_offset, src, size);
	m_offset += size;
	return l_current_offset;
}


KEMemorySystem::~KEMemorySystem()
{
	if(m_data)
		delete[] m_data;
	m_data = nullptr;
}
