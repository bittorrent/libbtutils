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
	bloom_filter& operator=(bloom_filter const& bf);
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
