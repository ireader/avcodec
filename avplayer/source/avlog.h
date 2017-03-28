#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

inline void avlog(const char* format, ...)
{
	static char msg[2 * 1024] = { 0 };

	va_list vl;
	va_start(vl, format);
	vsnprintf(msg, sizeof(msg) - 1, format, vl);
	va_end(vl);

#if defined(_WIN32) || defined(_WIN64)
	OutputDebugStringA(msg);
#else
	printf("%s", msg);
#endif
}

