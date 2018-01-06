#ifndef __KELOG_H__
#define __KELOG_H__
#include <fstream>

namespace KELog {
	
#ifdef _DEBUG

	static FILE* log_file = fopen("engine.log", "w");
#endif // _DEBUG

	__forceinline void Log(char* p_msg) {
#ifdef _DEBUG
		fprintf(log_file, p_msg);
		fflush(log_file);
#endif // _DEBUG

		
	}
}



#endif // !__KELOG_H__
