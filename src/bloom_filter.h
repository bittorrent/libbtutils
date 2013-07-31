#pragma once

#ifndef _BLOOM_FILTER_H_
#define _BLOOM_FILTER_H_

#include "utypes.h"
#include "sha1_hash.h"

struct bloom_filter
{
	bloom_filter(int size = 16 * 32, int num_hashes = 4);
	bloom_filter(int size, byte* set, int num_hashes = 4);
	bloom_filter(bloom_filter const& bf);
	bloom_filter const& operator=(bloom_filter const& bf);
	~bloom_filter();
	void set_union(byte const* set);
	void add(sha1_hash const& k);
	bool test(sha1_hash const& k) const;
	void clear();
	int get_size() const;
	const byte* get_set() const;
	int count_zeroes() const;
	int estimate_count() const;
private:
	int _size; // size in bits
	byte* _set;
	int _num_hashes;
};

#endif // _BLOOM_FILTER_H_
