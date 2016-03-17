#ifndef __DECODEENCODEDSTRING_H__
#define __DECODEENCODEDSTRING_H__

#include "utypes.h"
#include <string>

#ifndef WIN32

enum { CP_UTF8 = 1,
       CP_ACP = 0 };

#endif

int DecodeUtf8(cstr utf8, size_t in_len, wchar_t *out_ptr, size_t out_len, bool *invalid, size_t *consumed = NULL);
std::wstring DecodeEncodedString(int encoding, cstr encoded, size_t in_len = (size_t)-1);

#endif	// __DECODEENCODEDSTRING_H__
