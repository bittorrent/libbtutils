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
//
// Tests for the BitField class
// Originally derived from the utassert unit tests.
//

#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "bitfield.h"

#define ROUND_UP_TO_BYTES(x) (((x) + 7) >> 3)
enum { SOME_RANDOM_CONSTANT = 99 };

static const int hitlist[] =
{
	0, 1, 6, 7, 8, 14, 15, 16, 17, 63, 64, 65, 91, 92, 93, 97, 98
};
enum { HITLIST_LENGTH = sizeof(hitlist) / sizeof(int) };

TEST(BitFieldClassTest, TestBitOps)
{
	byte bit_vector[ROUND_UP_TO_BYTES(SOME_RANDOM_CONSTANT)];

	memset(bit_vector, 0, sizeof bit_vector);

	// Set all, then clear all
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		SETBIT(bit_vector, i);
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(HASBIT(bit_vector, i));
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		CLRBIT(bit_vector, i);
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(!HASBIT(bit_vector, i));

	// Set and clear hitlist one by one
	for (uint i = 0; i < HITLIST_LENGTH; ++i) {
		const int bitnum = hitlist[i];

		SETBIT(bit_vector, bitnum);
		EXPECT_TRUE(HASBIT(bit_vector, bitnum));
		CLRBIT(bit_vector, bitnum);
		EXPECT_TRUE(!HASBIT(bit_vector, bitnum));
	}

	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(!HASBIT(bit_vector, i));

	// Set all in list, then clear all in list
	for (uint i = 0; i < HITLIST_LENGTH; ++i)
		SETBIT(bit_vector, hitlist[i]);
	for (uint i = 0; i < HITLIST_LENGTH; ++i)
		EXPECT_TRUE(HASBIT(bit_vector, hitlist[i]));
	for (uint i = 0; i < HITLIST_LENGTH; ++i)
		CLRBIT(bit_vector, hitlist[i]);
	for (uint i = 0; i < HITLIST_LENGTH; ++i)
		EXPECT_TRUE(!HASBIT(bit_vector, hitlist[i]));

	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(!HASBIT(bit_vector, i));

	// Reverse order
	for (int i = HITLIST_LENGTH - 1; i >= 0; --i)
		SETBIT(bit_vector, hitlist[i]);
	for (int i = HITLIST_LENGTH - 1; i >= 0; --i)
		EXPECT_TRUE(HASBIT(bit_vector, hitlist[i]));
	for (int i = HITLIST_LENGTH - 1; i >= 0; --i)
		CLRBIT(bit_vector, hitlist[i]);
	for (int i = HITLIST_LENGTH - 1; i >= 0; --i)
		EXPECT_TRUE(!HASBIT(bit_vector, hitlist[i]));

	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(!HASBIT(bit_vector, i));

	// Set all, then check memory
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		SETBIT(bit_vector, i);
	for (uint i = 0; i < ROUND_UP_TO_BYTES(SOME_RANDOM_CONSTANT) - 1; ++i)
		EXPECT_TRUE(bit_vector[i] == 0xff);
	EXPECT_TRUE(bit_vector[ROUND_UP_TO_BYTES(SOME_RANDOM_CONSTANT) - 1] == 7);
}

TEST(BitFieldClassTest, TestBitField)
{
	byte bit_vector[ROUND_UP_TO_BYTES(SOME_RANDOM_CONSTANT)];

	memset(bit_vector, 0, sizeof bit_vector);
	BitField bf(bit_vector);

	// Set all, then clear all
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		bf.set(i);
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(bf.get(i));
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		bf.clr(i);
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_FALSE(bf.get(i));

	// Set and clear hitlist one by one
	for (uint i = 0; i < HITLIST_LENGTH; ++i) {
		const int bitnum = hitlist[i];

		bf.set(bitnum);
		EXPECT_TRUE(bf.get(bitnum));
		bf.clr(bitnum);
		EXPECT_TRUE(!bf.get(bitnum));
	}

	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(!bf.get(i));

	// Set all in list, then clear all in list
	for (uint i = 0; i < HITLIST_LENGTH; ++i)
		bf.set(hitlist[i]);
	for (uint i = 0; i < HITLIST_LENGTH; ++i)
		EXPECT_TRUE(bf.get(hitlist[i]));
	for (uint i = 0; i < HITLIST_LENGTH; ++i)
		bf.clr(hitlist[i]);
	for (uint i = 0; i < HITLIST_LENGTH; ++i)
		EXPECT_TRUE(!bf.get(hitlist[i]));

	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(!bf.get(i));

	// Reverse order
	for (int i = HITLIST_LENGTH - 1; i >= 0; --i)
		bf.set(hitlist[i]);
	for (int i = HITLIST_LENGTH - 1; i >= 0; --i)
		EXPECT_TRUE(bf.get(hitlist[i]));
	for (int i = HITLIST_LENGTH - 1; i >= 0; --i)
		bf.clr(hitlist[i]);
	for (int i = HITLIST_LENGTH - 1; i >= 0; --i)
		EXPECT_TRUE(!bf.get(hitlist[i]));

	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		EXPECT_TRUE(!bf.get(i));

	// Set all, then check memory
	for (uint i = 0; i < SOME_RANDOM_CONSTANT; ++i)
		bf.set(i);
	for (uint i = 0; i < ROUND_UP_TO_BYTES(SOME_RANDOM_CONSTANT) - 1; ++i)
		EXPECT_TRUE(bit_vector[i] == 0xff);
	EXPECT_TRUE(bit_vector[ROUND_UP_TO_BYTES(SOME_RANDOM_CONSTANT) - 1] == 7);
}
