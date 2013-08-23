#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "get_microseconds.h"

//
// This test was converted from the old utassert style unit test
//

TEST(GetMicrosecondsClassTest, TestMonotonicClock)
{
	enum { REPEAT_COUNT = 10000000, NOTIFY_INTERVAL = REPEAT_COUNT / 20 };

#ifdef WIN32
	// TODO:  May need to initialize time system in test prep
	//Time_Initialize();
#endif

	unsigned long long previous_value = 0;
	unsigned long long first_value = 0;
	unsigned long long value = get_microseconds();

	// Ensure clock function not returning zero
	EXPECT_TRUE(value > 0)
		<< "Initial value of microsecond clock not greater than zero";
	first_value = value;
	for (unsigned long long tick_index = 0; tick_index < REPEAT_COUNT; ++tick_index) {
		value = get_microseconds();
		EXPECT_TRUE(value >= previous_value)
			<< "Monotonic clock went backward at index "
			<< tick_index
			<< " from " << previous_value << " to " << value;
		previous_value = value;
	}
	value = get_microseconds();
	// Ensure clock function not returning zero or the same value every time
	EXPECT_TRUE(value > 0)
		<< "Final value of microsecond clock not greater than zero";
	EXPECT_TRUE(value > first_value)
		<< "value " << value << " first_value " << first_value;
}

