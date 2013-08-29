#include "get_microseconds.h"

#include <assert.h>

#if defined(__APPLE__)

#include <mach/mach_time.h>

uint64 get_microseconds()
{
	// http://developer.apple.com/mac/library/qa/qa2004/qa1398.html
	// http://www.macresearch.org/tutorial_performance_and_time
	static mach_timebase_info_data_t sTimebaseInfo = { 0, 0 };
	static uint64_t start_tick = 0;
	uint64_t tick;
	// Returns a counter in some fraction of a nanoseconds
	tick = mach_absolute_time();
	if (sTimebaseInfo.denom == 0 || sTimebaseInfo.numer == 0) {
		// Get the timer ratio to convert mach_absolute_time to nanoseconds
		mach_timebase_info(&sTimebaseInfo);
		start_tick = tick;
	}
	// Calculate the elapsed time, convert it to microseconds and
	// return it.  Through trial and error I found that the
	// computation below consistently returns a result within 2000us
	// or 2ms of the result returned by gettimeofday.  Multiplying the
	// deominator with 1000 before using it to divide the numerator or
	// dividing the numerator by the denominator before multiplying it
	// with tick - start_tick caused the result to vary from
	// gettimeofday by over 20,000us or 20ms.
	return ((tick - start_tick) * sTimebaseInfo.numer / sTimebaseInfo.denom) / 1000;
}
#elif defined WIN32

#include <windows.h>

// MSVC 6 standard doesn't like division with uint64s
static double counterPerMicrosecond;

static uint64 startPerformanceCounter;
static int64 startGetTickCount;

struct static_initialization
{
	static_initialization()
	{
		uint64 frequency;
		QueryPerformanceCounter((LARGE_INTEGER*)&startPerformanceCounter);
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
		assert(frequency > 0);
		counterPerMicrosecond = (double)frequency / 1000000.0f;

		startGetTickCount = GetTickCount();
	}
} dummy_static_initializer;

namespace {
	int64 abs64(int64 x) { return x < 0 ? -x : x; }
}

uint64 get_microseconds()
{
	uint64 counter;

	QueryPerformanceCounter((LARGE_INTEGER*) &counter);
	int64 tick = GetTickCount();

	// handle the tick count wrapping
	if (tick < startGetTickCount) {
		startGetTickCount = tick;
		startPerformanceCounter = counter;
	}

	// unfortunately, QueryPerformanceCounter is not guaranteed
	// to be monotonic. Make it so.
	// NOTE: There are multiprocessor systems with buggy hardware
	// abstraction layers where the processors are not correctly
	// synchronized so each processor can return a different value
	// similar to rdtsc. See comments in Time_Initialize() above.
	int64 ret = (int64(counter) - int64(startPerformanceCounter)) / counterPerMicrosecond;
	// if the QPC clock leaps more than one second off GetTickCount64()
	// something is seriously fishy. Adjust QPC to stay monotonic
	int64 tick_diff = tick - startGetTickCount;
	if (abs64(ret / 100000 - tick_diff / 100) > 10) {
		startPerformanceCounter -= int64(tick_diff * 1000 - ret) * counterPerMicrosecond;
		ret = (counter - startPerformanceCounter) / counterPerMicrosecond;
	}

	return ret;
}

#else

// Non-OSX POSIX
#include <unistd.h>	// For the POSIX clock definitions (and, unfortunately, a lot else)
#include <time.h>	// For clock_gettime(), CLOCK_MONOTONIC

#if (_POSIX_TIMERS && defined(_POSIX_MONOTONIC_CLOCK) && (_POSIX_MONOTONIC_CLOCK >= 0) && defined(CLOCK_MONOTONIC)) || defined(ANDROID)
#include <errno.h>	// for errno
#include <string.h>	// for strerror()

uint64 get_microseconds()
{
	timespec t;
	int status = clock_gettime(CLOCK_MONOTONIC, &t);
#ifdef _DEBUG
	// TODO:  we need logging in utils
	//if (status)
		//DbgLogf("clock_gettime returned %d - error %d %s", status, errno, ::strerror(errno));
#endif
	assert(status == 0);
	uint64 tick = uint64(t.tv_sec) * 1000000 + uint64(t.tv_nsec) / 1000;
	return tick;
}
#else
// Fallback

// Some diagnostics, since we'd prefer that platforms provide a monotonic clock
#if _POSIX_TIMERS
#pragma message ("_POSIX_TIMERS defined")
#else
#pragma message ("_POSIX_TIMERS not defined or zero")
#endif
#if defined(_POSIX_MONOTONIC_CLOCK)
#pragma message ("_POSIX_MONOTONIC_CLOCK defined")
#else
#pragma message ("_POSIX_MONOTONIC_CLOCK not defined")
#endif
#if _POSIX_MONOTONIC_CLOCK >= 0
#pragma message ("_POSIX_MONOTONIC_CLOCK >= 0")
#else
#pragma message ("_POSIX_MONOTONIC_CLOCK not >= 0 or not defined")
#endif
#if defined(CLOCK_MONOTONIC)
#pragma message ("CLOCK_MONOTONIC defined")
#else
#pragma message ("CLOCK_MONOTONIC not defined")
#endif // defined(CLOCK_MONOTONIC)

#pragma message ("Using non-monotonic function gettimeofday() in get_microseconds()")

#include <sys/time.h>

uint64 get_microseconds()
{
	// No point in making these global.  They are only used here.
	static time_t start_time = 0;

	timeval t;
	::gettimeofday(&t, NULL);

	// avoid overflow by subtracting the seconds
	if (start_time == 0) start_time = t.tv_sec;

	return uint64(t.tv_sec - start_time) * 1000000 + (t.tv_usec);
}
#endif // Big ol' expression about _POSIX_MONOTONIC_CLOCK etc.

#endif

uint64 get_milliseconds()
{
	return get_microseconds() / 1000;
}

