#include <chrono>
#include "TimeUtils.h"

uint64_t getCurrentTODClock()
{
	auto currentTime = std::chrono::system_clock::now();
	auto sinceEpoch = currentTime.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(sinceEpoch).count();
}
