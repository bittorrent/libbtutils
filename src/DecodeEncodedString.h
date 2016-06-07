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
