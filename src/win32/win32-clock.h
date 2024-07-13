#ifndef SRC_WIN32_WIN32_CLOCK_H_
#define SRC_WIN32_WIN32_CLOCK_H_

#include <windows.h>

#include <cstdint>

LARGE_INTEGER GetWallClock();

float GetCountersElapsed(LARGE_INTEGER start, LARGE_INTEGER end);
float GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end,
                        int64_t perf_count_frequency);

#endif  // SRC_WIN32_WIN32_CLOCK_H_
