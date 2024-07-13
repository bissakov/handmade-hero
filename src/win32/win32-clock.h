#ifndef WIN32_CLOCK_H_

#include <windows.h>

#include <cstdint>

LARGE_INTEGER GetWallClock();

float GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end,
                        int64_t perf_count_frequency);

#define WIN32_CLOCK_H_
#endif  // WIN32_CLOCK_H_
