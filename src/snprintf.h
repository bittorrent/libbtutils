/*
Copyright 2016 BitTorrent Inc

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
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

