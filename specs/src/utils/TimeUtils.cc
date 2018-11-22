#include <chrono>
#include <iomanip>
// #include <time.h>
#include <iostream>
#include <sstream>
#include "TimeUtils.h"

using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;

uint64_t specTimeGetTOD()
{
	auto currentTime = std::chrono::system_clock::now();
	auto sinceEpoch = currentTime.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::microseconds>(sinceEpoch).count();
}

Clock::duration dur = std::chrono::microseconds(0x156981aa9fdecde0);
TimePoint tmns(dur);

PSpecString specTimeConvertToPrintable(uint64_t sinceEpoch, std::string format)
{
	Clock::duration dur = std::chrono::microseconds(sinceEpoch);
	TimePoint tp(dur);
	auto tmc = Clock::to_time_t(tp);
	std::tm bt = *std::localtime(&tmc);
	unsigned int fractionalSecond = (unsigned int)(sinceEpoch % 1000000);
	std::ostringstream oss;
	unsigned char fractionalSecondLength = 0;

	if (format.length() >= 3) {
		auto l = format.length();
		if (format[l-3]=='%' && format[l-1]=='f' &&
				format[l-2]>='0' && format[l-2]<='6') {
			fractionalSecondLength = format[l-2] - '0';
			format = format.substr(0,l-3);
		}
	}


	oss << std::put_time(&bt, format.c_str());
	if (fractionalSecondLength) {
		oss << std::setw(fractionalSecondLength) << std::setfill('0');
		while (fractionalSecondLength < 6) {
			fractionalSecondLength++;
			fractionalSecond /= 10;
		}
		oss << fractionalSecond;
	}
	std::string ret = oss.str();
	return SpecString::newString(ret);
}

uint64_t specTimeConvertFromPrintable(std::string printable, std::string format)
{
	std::tm t;

	return 0;
}
