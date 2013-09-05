#ifndef __SHA1_HASH_H__
#define __SHA1_HASH_H__

#include <string.h> // for memcmp, et. al.
#include <assert.h>

#include "utypes.h" // for byte

#define SHA1_DIGESTSIZE 20

struct sha1_hash {
	byte value[SHA1_DIGESTSIZE];

	sha1_hash() {}
	sha1_hash(const byte* b) { memcpy(value, b, 20); }

	sha1_hash & operator=(const sha1_hash & hash)
	{
		if (&value != &hash.value)
			memcpy(value, hash.value, sizeof(value));
		return *this;
	}

	bool is_all_zero() const
	{
		for (int i = 0; i < sizeof(value); ++i)
			if (value[i] != 0) return false;
		return true;
	}

	void clear()
	{
		for (int i = 0; i < sizeof(value); ++i)
			value[i] = 0;
	}

	sha1_hash &operator=(const byte *input)
	{
		if (input)
			memcpy(value, input, sizeof(value));
		else
			memset(&value, 0, sizeof(value));
		return *this;
	}

	bool operator==(const sha1_hash & hash) const
	{
		return (0 == memcmp(value, hash.value, sizeof(value)));
	}

	bool operator==(const byte *hash) const
	{
		assert(hash);
		if (!hash)
			return false;
		return (0 == memcmp(value, hash, sizeof(value)));
	}

	bool operator!=(const sha1_hash &hash) const { return !(*this == hash); }
	bool operator!=(const byte *hash) const { return !(*this == hash); }

	bool operator<(const sha1_hash & hash) const
	{
		// Used in peerconn.cpp
		return memcmp(value, hash.value, sizeof(value)) < 0;
	}

	unsigned char operator[](int i) const { return value[i]; }
};


#endif

