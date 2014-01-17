#include "DecodeEncodedString.h"
#include <string.h> // for strlen
#include <assert.h>
#include <stdlib.h> // for malloc, realloc

int DecodeUtf8(cstr utf8, size_t in_len, wchar_t *out_ptr, size_t out_len, bool *invalid, size_t *consumed)
{
	wchar_t *dst = out_ptr; // over-estimate
	uint i = 0;
	cstr utf8_begin = utf8;
	cstr utf8_end = utf8 + in_len;

	*invalid = false;

	for(;;) {

		if (utf8 >= utf8_end) {
			if (consumed) *consumed = utf8 - utf8_begin;
			return i;
		}

		uint b = *utf8++;
		if (!(b & 0x80)) {
			// normal ascii
		} else if ((b & 0xE0) == 0xC0) {
			// two character unicode
			if (utf8 < utf8_end && (utf8[0] & 0xC0) == 0x80) {
				b = ((b & 0x1F) << 6) | (utf8[0] & 0x3F);
				utf8 += 1;
				if ( b < 0x80 ) {
					*invalid = true;
				}
			} else {
				*invalid = true;
			}
		} else if ((b & 0xF0) == 0xE0) {
			// three character unicode
			if (utf8 < utf8_end-1 && (((uint16*)utf8)[0] & 0xC0C0) == 0x8080) {
				b = ((b & 0xF) << 12) | ((utf8[0] & 0x3F) << 6) | (utf8[1] & 0x3F);
				utf8 += 2;
				if ( b < 0x800 ) {
					*invalid = true;
				}
			} else {
				*invalid = true;
			}
		} else if ((b & 0xF8) == 0xF0) {
			// four character unicode
			if (utf8 < utf8_end-2 && ( (((uint16*)utf8)[0] | (utf8[2]<<16)) & 0xC0C0C0) == 0x808080) {
				b = (b << 18) ^ (utf8[2] << 12) ^ (utf8[1] << 6) ^ utf8[2] ^ (0x80|(0x80<<6)|(0x80<<12)|(0xF0<<18));
				utf8 += 3;
				if (b < 0x10000) {
					*invalid = true;
				} else {
					// decode a surrogate pair
					if (i>= out_len) {
						if (consumed) *consumed = utf8 - utf8_begin - 1;
						return -1;
					}
					b -= 0x10000;
					dst[i++] = 0xD800 | (b >> 10);
					b = 0xDC00 | (b & 0x3FF);
				}
			} else {
				*invalid = true;
			}
		} else {
			*invalid = true;
		}

		if (i >= out_len) {
			if (consumed) *consumed = utf8 - utf8_begin - 1;
			return -1;
		}

		dst[i++] = b;
	}
}

wchar_t *DecodeEncodedString(int encoding, cstr encoded, size_t in_len, size_t *out_len)
{
	if (in_len == (size_t) -1) in_len = strlen(encoded);

	wchar_t *t = (wchar_t*)malloc(sizeof(wchar_t) * (in_len + 1));
	size_t len = in_len;

	if (in_len != 0) {
		// TODO: For POSIX, look at repercussions of setting CP_ACP in
		// wincompat.h to something other than 0.  Note that having
		// CP_ACP set to 0 is bad here because in non windows builds
		// passing in CP_ACP will trigger us to try to decode this as
		// CP_UTF8 and possibly assert even if we explicitly set
		// encoding to CP_ACP.

		// First try to decode as utf-8
		if (encoding == 0 || encoding == CP_UTF8) {
			bool invalid;
			len = DecodeUtf8(encoded, in_len, t, in_len, &invalid);
			assert(len >= 0);
			if (!invalid || encoding == CP_UTF8)
				goto OK;
			encoding = CP_ACP;
		}

		// Otherwise perform operating system-specific decode
#ifdef _WIN32
		len = MultiByteToWideChar(encoding, 0, encoded, in_len, t, in_len);
		if (0 == len && CP_ACP != encoding)
			len = MultiByteToWideChar(CP_ACP, 0, encoded, in_len, t, in_len);
#else
		// See man pages for mbstowcs mbstowcs_l multibyte xlocale mbsnrtowcs
		mbstate_t ps;
		memset(&ps, 0, sizeof(ps));
		cstr in_p = encoded;
#ifdef ANDROID
		// 20110302 - Android doesn't support the safer length-limited mbsnrtowcs
		len = mbsrtowcs(t, &in_p, in_len, &ps);
#else
		len = mbsnrtowcs(t, &in_p, in_len, (in_len + 1) * sizeof(wchar_t), &ps);
#endif // ANDROID
		if (len == (size_t)-1) // error
			len = 0;
		else if (in_p != 0) {
			// There was an error decoding this string.  in_p now points
			// to the last character that underwent successful conversion.
//			DbgLogf("DecodeEncodedString encountered an error in mbsnrtowcs and only partially decoded the string. Continuing...");
		}
#endif
	}
OK:
	t[len] = 0; // null terminate
	if (in_len >= len + 4)
		t = (wchar_t*)realloc(t, sizeof(wchar_t) * (len + 1));
	if (out_len)
		*out_len = len;
	return t;
}
