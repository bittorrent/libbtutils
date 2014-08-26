#ifndef __SNPRINTF_H__
#define __SNPRINTF_H__

#ifndef _MSC_VER
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#include <inttypes.h> // for PRId64 et.al.
#endif

#ifndef PRId64
// MinGW uses microsofts runtime
#if defined _MSC_VER || defined __MINGW32__
#define PRId64 "I64d"
#define PRIu64 "I64u"
#elif !defined PRId64
#define PRId64 "lld"
#define PRIu64 "llu"
#endif
#endif

#ifdef _WIN32

#include <stdarg.h>

int snprintf(char* buf, int len, char const* fmt, ...);

#else

#include <stdio.h> // for snprintf

#endif

#endif

