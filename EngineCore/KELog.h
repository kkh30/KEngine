#ifndef __KELOG_H__
#define __KELOG_H__
#include <fstream>
#include <stdarg.h>

namespace KELog {
	
#ifdef _DEBUG

	static FILE* log_file = fopen("engine.log", "w");
#endif // _DEBUG

	__forceinline void Log(const char* const format,...) {
#ifdef _DEBUG
		char dest[1024 * 16];
		va_list argptr;
		va_start(argptr, format);
		vsprintf(dest, format, argptr);
		va_end(argptr);
		fprintf(log_file,dest);
		fflush(log_file);
#endif // _DEBUG

		
	}
}



#endif // !__KELOG_H__
