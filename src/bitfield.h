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
#ifndef __BITFIELD_H__
#define __BITFIELD_H__

#include <stddef.h> // for NULL
#include "utypes.h"

#define SETBIT(arr, n) ((arr)[(n)>>3] |= (1 << ((n) & 7)))
#define CLRBIT(arr, n) ((arr)[(n)>>3] &= ~(1 << ((n) & 7)))
#define HASBIT(arr, n) ((arr)[(n)>>3] >> ((n)&7) & 1)

#define SETBIT2(arr, n, l) (assert(n < l), (arr)[(n)>>3] |= (1 << ((n) & 7)))
#define CLRBIT2(arr, n, l) (assert(n < l), (arr)[(n)>>3] &= ~(1 << ((n) & 7)))
#define HASBIT2(arr, n, l) (assert(n < l), (arr)[(n)>>3] >> ((n)&7) & 1)

struct reference;

struct BitField {

	BitField() : bytes(NULL) {}
	BitField(byte *s) : bytes(s) {}

	bool operator==(byte* val) const { return bytes == val; }
	bool operator!=(byte* val) const { return bytes != val; }

	byte* get_bytes() const { return bytes; }
	void set_bytes(byte* n) { bytes = n; }

	reference operator [](size_t i);
	bool operator [](size_t i) const { return get(i); }

	bool get(size_t i) const { return HASBIT(bytes, i); }
	void set(size_t i, bool n) { n?set(i):clr(i); }
	void set(size_t i) { SETBIT(bytes, i); }
	void clr(size_t i) { CLRBIT(bytes, i); }

	byte* bytes;
};

struct reference {
	reference(BitField& l, size_t n) : _l(l), i(n) {}

	reference& operator =(bool n) { _l.set(i, n); return *this; }
	operator bool() const { return _l.get(i); }

	BitField& _l;
	const size_t i;
};

#endif //__BITFIELD_H__
