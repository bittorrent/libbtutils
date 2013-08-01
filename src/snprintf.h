#ifndef __SNPRINTF_H__
#define __SNPRINTF_H__

#ifdef WIN32

#include <stdarg.h>

int snprintf(char* buf, int len, char const* fmt, ...);

#else

#include <stdio.h> // for snprintf

#endif

#endif

