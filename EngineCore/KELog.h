#ifndef __KELOG_H__
#define __KELOG_H__
#include <fstream>
#include <stdarg.h>

namespace KELog {
	
#ifdef _DEBUG
	static FILE* log_file_open = nullptr;
	static errno_t file_open = fopen_s(&log_file_open,"engine.log", "w");
	static char dest[1024 * 16];

#endif // _DEBUG

	__forceinline void Log(const char* const format,...) {
#ifdef _DEBUG
		va_list argptr;
		va_start(argptr, format);
		vsprintf_s(dest, format, argptr);
		va_end(argptr);
		fprintf_s(log_file_open,dest);
		printf(dest);
		fflush(log_file_open);
#endif // _DEBUG

		
	}
}



#endif // !__KELOG_H__
