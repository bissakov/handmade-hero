#include "../../src/win32/win32-clock.h"

#include <cstdint>

LARGE_INTEGER GetWallClock() {
  LARGE_INTEGER result;
  QueryPerformanceCounter(&result);
  return result;
}

float GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end,
                        int64_t perf_count_frequency) {
  float result = static_cast<float>(end.QuadPart - start.QuadPart) /
                 static_cast<float>(perf_count_frequency);
  return result;
}
