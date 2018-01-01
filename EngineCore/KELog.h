#ifndef __KELOG_H__
#define __KELOG_H__
#include <fstream>

namespace KELog {
	

	static FILE* log_file = fopen("engine.log", "w");

	__forceinline void Log(char* p_msg) {
		fprintf(log_file, p_msg);
		fflush(log_file);
	}
}



#endif // !__KELOG_H__
