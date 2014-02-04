#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "bloom_filter.h"
#include "sha1_hash.h"

//
// This test was converted from the old utassert style unit test.
// It no longer uses the SHA1 class to generate hashes, because that class
// is not contained within this unit collection.
//

TEST(BloomFilterClassTest, TestInsertions)
{
	enum
	{
		INCLUDED_VALUE_COUNT = 12
		, EXCLUDED_VALUE_COUNT = 100
		, DUP_SOURCE_INDEX = 3
		, DUP_DEST_INDEX = 7
		, INDEX_LIMIT = 256
	};
	ASSERT_TRUE(DUP_SOURCE_INDEX != DUP_DEST_INDEX);
	ASSERT_TRUE(DUP_SOURCE_INDEX < INCLUDED_VALUE_COUNT);
	ASSERT_TRUE(DUP_DEST_INDEX < INCLUDED_VALUE_COUNT);

	size_t index;
	byte hashValues[INCLUDED_VALUE_COUNT + EXCLUDED_VALUE_COUNT][SHA1_DIGESTSIZE];

	for (index = 0; index < INDEX_LIMIT; ++index) {
		size_t hashIndex;
		bloom_filter f;

		// Generate a set of hashes to be inserted
		// Rules:
		// 1) values are unique within this set, except for one case
		// 2) entries DUP_SOURCE_INDEX and DUP_DEST_INDEX are duplicates
		// 3) entries differ from each other by one value

		// Rules 1 and 3
		for (hashIndex = 0; hashIndex < INCLUDED_VALUE_COUNT; ++hashIndex) {
			memset(hashValues[hashIndex], index, SHA1_DIGESTSIZE);
			hashValues[hashIndex][SHA1_DIGESTSIZE - 1] = (byte) hashIndex;
		}
		// Rule 2
		hashValues[DUP_DEST_INDEX][SHA1_DIGESTSIZE - 1] =
			hashValues[DUP_SOURCE_INDEX][SHA1_DIGESTSIZE - 1];

		// Generate a set of hashes to not be inserted
		// Rules:
		// 1) the values must be different than any of the inserted values
		// 2) values are unique within this set
		// 3) due to the finite probability of false positives, the values
		//    must be different enough from the included values to not trigger
		//    false positives
		for (hashIndex = INCLUDED_VALUE_COUNT;
			hashIndex < INCLUDED_VALUE_COUNT + EXCLUDED_VALUE_COUNT;
			++hashIndex) {
			memset(hashValues[hashIndex], index, SHA1_DIGESTSIZE);
			hashValues[hashIndex][SHA1_DIGESTSIZE - 1] = (byte) hashIndex;
			// Muck with early part of values to avoid false positives.
			// Commenting out this line results in lots of false positives.
			hashValues[hashIndex][0] = (byte) ((index + 1) % INDEX_LIMIT);
		}

		// Load the bloom filter with the included hashes
		for (hashIndex = 0; hashIndex < INCLUDED_VALUE_COUNT; ++hashIndex) {
			sha1_hash key;
			key = hashValues[hashIndex];
			f.add(key);
		}

		// Verify the bloom filter includes the included hashes
		for (hashIndex = 0; hashIndex < INCLUDED_VALUE_COUNT; ++hashIndex) {
			sha1_hash key;
			key = hashValues[hashIndex];
			EXPECT_TRUE(f.test(key))
				<< " index " << index << " hashIndex " << hashIndex;
		}

		// Verify the bloom filter does not include the excluded hashes
		int false_positives = 0;
		for (hashIndex = INCLUDED_VALUE_COUNT;
			hashIndex < INCLUDED_VALUE_COUNT + EXCLUDED_VALUE_COUNT;
			++hashIndex) {
			sha1_hash key;
			key = hashValues[hashIndex];
			EXPECT_FALSE(f.test(key))
				<< " index " << index << " hashIndex " << hashIndex;
			if (f.test(key))
				++false_positives;
		}
		EXPECT_TRUE(false_positives <= 1) << "false positives " << false_positives << " index " << index;
	}
}

