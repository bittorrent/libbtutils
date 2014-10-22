#ifndef __UTYPES_H__
#define __UTYPES_H__

#include <ctype.h>

#if defined _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <tchar.h> // for _T
#endif

// standard types
typedef unsigned char byte;
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint;
typedef unsigned int uint32;
typedef signed int int32;

#ifdef _MSC_VER
typedef unsigned __int64 uint64;
typedef signed __int64 int64;
#else
typedef unsigned long long uint64;
typedef long long int64;
#endif

#ifndef _WIN32
typedef int32 LONG;
typedef uint32 ULONG;
typedef uint32 DWORD;
typedef uint64 ULONGLONG;
#endif

#ifdef ANDROID
typedef int64 LONGLONG;
#define INT64_MAX 0x7fffffffffffffffLL
#endif

#if __cplusplus > 199711L
static_assert (8 == sizeof(uint64), "sizeof uint64 is not 8");
static_assert (8 == sizeof(int64), "sizeof int64 is not 8");
static_assert (4 == sizeof(int32), "sizeof int32 is not 4");
static_assert (4 == sizeof(uint32), "sizeof uint32 is not 4");
#endif

/* _T("txt") evaluates to either ascii string in non-unicode builds or unicode
   string in unicode builds */
#if defined _WIN32 && defined _MSC_VER
  /* _T is defined in tchar.h */
  typedef TCHAR tchar;
#else
  #ifdef _UNICODE
  typedef wchar_t tchar;
    #ifndef WIN_RT
  #define _T(x) L##x
    #endif	// WIN_RT
  #else
  typedef char tchar;
  #define _T(x) x
  #endif
#endif /* _T */

#define _W(x) L##x

typedef const tchar * ctstr;
typedef tchar * tstr;
typedef const wchar_t * cwstr;
typedef wchar_t * wstr;

// always ANSI
typedef const char * cstr;
typedef char * str;

#endif //__UTYPES_H__
