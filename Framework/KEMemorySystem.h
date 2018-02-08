#ifndef _KEMEMORY_SYSTEM_H__
#define _KEMEMORY_SYSTEM_H__
#include "EngineConstans.h"

class KEMemorySystem
{
public:
	INLINE static KEMemorySystem& GetMemorySystem() {
		static KEMemorySystem l_system;
		return l_system;
	}
	~KEMemorySystem();
	KEMemorySystem(const KEMemorySystem&) = delete;
	KEMemorySystem& operator =(const KEMemorySystem&) = delete;
	uint64_t UploadVertexDataToVRAM(void* src, uint64_t size);
	uint64_t UploadIndexDataToVRAM(void* src,  uint64_t size);

	INLINE void* GetVertexDataPtr() { return m_vertex_data; }
	INLINE void* GetIndexDataPtr() { return m_index_data; }
	INLINE uint64_t GetVertexDataSize() { return m_vertex_data_size; }
	INLINE uint64_t GetIndexDataSize() { return m_index_data_size; }

private:
	uint8_t* m_vertex_data;
	uint8_t* m_index_data;
	KEMemorySystem();
	uint64_t m_vertex_data_size;
	uint64_t m_index_data_size;
	uint64_t m_vertex_offset;
	uint64_t m_index_offset;


};


#endif