/*
Tests for the sha1_hash class
*/

#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "sha1_hash.h"

TEST(Sha1HashClassTest, TestOperators)
{
	static const byte rawhash1[SHA1_DIGESTSIZE] = {
		0xa8, 0xdb, 0xeb, 0x79, 0xac, 0x85, 0xd6, 0x3b,
		0xa7, 0x84, 0xeb, 0x09, 0x3a, 0x69, 0x91, 0x01,
		0xda, 0x8f, 0xea, 0x2f
	};
	static const byte rawhash2[SHA1_DIGESTSIZE] = {
		0xff, 0x07, 0xdc, 0xfc, 0x86, 0x05, 0xb6, 0x28,
		0x6e, 0xf4, 0x96, 0xdb, 0xda, 0x29, 0xbb, 0xa3,
		0xfd, 0x11, 0x7a, 0x73
	};
	static const byte zerohash[SHA1_DIGESTSIZE] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0
	};
	sha1_hash hash1;
	hash1 = rawhash1;
	EXPECT_TRUE(hash1 == rawhash1) << "FAILED to assign an initial value";

	sha1_hash hash1dup = hash1;
	EXPECT_TRUE(hash1 == rawhash1) << "FAILED to copy a value";
	EXPECT_TRUE(hash1 == hash1dup);

	sha1_hash hash2;
	hash2 = rawhash2;
	EXPECT_TRUE(hash2 == rawhash2) << "FAILED to assign an initial value";
	EXPECT_FALSE(hash2 != rawhash2);
	EXPECT_FALSE(hash2 == hash1dup);
	EXPECT_FALSE(hash2 == rawhash1);
	EXPECT_FALSE(hash1 == rawhash2);

	sha1_hash hashzero;
	hashzero = zerohash;
	EXPECT_TRUE(hashzero < hash1);
	EXPECT_TRUE(hashzero < hash2);

	EXPECT_TRUE(hash1 < hash2);

	for (size_t index = 0; index < SHA1_DIGESTSIZE; ++index) {
		EXPECT_EQ(rawhash1[index], hash1[index]);
		EXPECT_EQ(rawhash2[index], hash2[index]);
		EXPECT_EQ(hashzero[index], zerohash[index]);
	}
}

