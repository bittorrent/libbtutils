#ifndef __DECODEENCODEDSTRING_H__
#define __DECODEENCODEDSTRING_H__

#include "utypes.h"
#include <string.h> // for NULL
#include <wchar.h>

int DecodeUtf8(cstr utf8, size_t in_len, wchar_t *out_ptr, size_t out_len, bool *invalid, size_t *consumed = NULL);
wchar_t *DecodeEncodedString(int encoding, cstr encoded, size_t in_len = (size_t)-1, size_t *out_len=NULL);

#endif	// __DECODEENCODEDSTRING_H__
