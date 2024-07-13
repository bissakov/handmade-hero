#include "../../src/win32/win32-clock.h"

#include <cstdint>

LARGE_INTEGER GetWallClock() {
  LARGE_INTEGER result;
  QueryPerformanceCounter(&result);
  return result;
}

float GetCountersElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
  float result = static_cast<float>(end.QuadPart - start.QuadPart);
  return result;
}

float GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end,
                        int64_t perf_count_frequency) {
  float counters_elapsed = GetCountersElapsed(start, end);
  float result = counters_elapsed / static_cast<float>(perf_count_frequency);
  return result;
}
