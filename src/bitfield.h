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
