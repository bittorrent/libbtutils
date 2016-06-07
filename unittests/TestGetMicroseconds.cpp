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

