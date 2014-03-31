#include "bloom_filter.h"
#include "bitfield.h"
#include "endian_utils.h"

#include <stdlib.h> // for calloc
#include <assert.h>
#include <algorithm> // for min()
#include <math.h> // for log()

bloom_filter::bloom_filter(int size, int num_hashes)
{
	// round up to closest byte
	_set = (byte*)calloc((size + 7) / 8, 1);
	_size = size;
	assert(_size % 8 == 0);
	_num_hashes = num_hashes;
}

bloom_filter::bloom_filter(int size, byte* set, int num_hashes) {
	assert(size % 8 == 0);
	_set = (byte*)malloc((size + 7) / 8);
	memcpy(_set, set, (size + 7) / 8);
	_size = size;
	_num_hashes = num_hashes;
}

bloom_filter::bloom_filter(bloom_filter const& bf)
{
	int size = (bf._size + 7) / 8;
	_set = (byte*)malloc(size);
	memcpy(_set, bf._set, size);
	_size = bf._size;
	_num_hashes = bf._num_hashes;
}

bloom_filter const& bloom_filter::operator=(bloom_filter const& bf)
{
	free(_set);

	int size = (bf._size + 7) / 8;
	_set = (byte*)malloc(size);
	memcpy(_set, bf._set, size);
	_size = bf._size;
	_num_hashes = bf._num_hashes;
	return *this;
}

void bloom_filter::set_union(byte const* set)
{
	for (int i = 0; i < (_size + 7) / 8; ++i) {
		_set[i] |= set[i];
	}
}

bloom_filter::~bloom_filter()
{
	free(_set);
}

void bloom_filter::add(sha1_hash const& k)
{
	// instead of using multiple hash functions,
	// we use SHA1 and use different portions of
	// the digest as separate indices

	for (int i = 0; i < _num_hashes; ++i) {
		uint16 h = ReadBE16(k.value + i * 2);
		h %= _size;
		SETBIT(_set, h);
	}
}

int bloom_filter::count_zeroes() const
{
	// number of bits _not_ set in a nibble
	byte bitcount[16] =
	{
		// 0000, 0001, 0010, 0011, 0100, 0101, 0110, 0111,
		// 1000, 1001, 1010, 1011, 1100, 1101, 1110, 1111
		4, 3, 3, 2, 3, 2, 2, 1,
		3, 2, 2, 1, 2, 1, 1, 0
	};
	int ret = 0;
	for (int i = 0; i < (_size + 7) / 8; ++i)
	{
		ret += bitcount[_set[i] & 0xf];
		ret += bitcount[(_set[i] >> 4) & 0xf];
	}
	return ret;
}

int bloom_filter::estimate_count() const
{
	const int c = (std::min)(count_zeroes(), _size - 1);
	const int m = _size;
	return (int)(log(c / float(m)) / (2.f * log(1.f - 1.f/m)));
}

bool bloom_filter::test(sha1_hash const& k) const
{
	for (int i = 0; i < _num_hashes; ++i) {
		uint16 h = ReadBE16(k.value + i * 2);
		h %= _size;
		if (!HASBIT(_set, h)) return false;
	}
	return true;
}

void bloom_filter::clear()
{
	memset(_set, 0, (_size + 7) / 8);
}

int bloom_filter::get_size() const {
	return _size;
}

const byte* bloom_filter::get_set() const {
	return _set;
}
