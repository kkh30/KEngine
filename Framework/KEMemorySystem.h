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
	uint64_t UploadToVRAM(void* src, uint64_t size);
	INLINE void* GetDataPtr() { return m_data; }
	INLINE uint64_t GetMemorySize() { return m_size; }
private:
	void* m_data;
	
	KEMemorySystem();
	uint64_t m_size;
	uint64_t m_offset;


};


#endif