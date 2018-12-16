#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "TimeUtils.h"

#define MICROSECONDS_PER_SECOND 1000000
using TimeResolution = std::chrono::microseconds;

using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;

uint64_t specTimeGetTOD()
{
	auto currentTime = std::chrono::system_clock::now();
	auto sinceEpoch = currentTime.time_since_epoch();
	return std::chrono::duration_cast<TimeResolution>(sinceEpoch).count();
}


PSpecString specTimeConvertToPrintable(uint64_t sinceEpoch, std::string format)
{
	Clock::duration dur = std::chrono::microseconds(sinceEpoch);
	TimePoint tp(dur);
	auto tmc = Clock::to_time_t(tp);
	std::tm bt = *std::localtime(&tmc);
	unsigned int fractionalSecond = (unsigned int)(sinceEpoch % MICROSECONDS_PER_SECOND);
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

#if __GNUC__ > 4
	oss << std::put_time(&bt, format.c_str());
#else
    char timeFormatterString[256];
    strftime(timeFormatterString, 255, format.c_str(), &bt);
    oss << timeFormatterString;
#endif
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
	// initialize t in case of missing fields
	std::time_t now = std::time(nullptr);
	std::tm t = *(std::localtime(&now));

	unsigned int fractionalSeconds = 0;
	unsigned char fractionalSecondLength = 0;

	if (format.length() >= 3) {
		auto l = format.length();
		if (format[l-3]=='%' && format[l-1]=='f' &&
				format[l-2]>='0' && format[l-2]<='6') {
			fractionalSecondLength = format[l-2] - '0';
			format = format.substr(0,l-3);
			if (printable.length() < fractionalSecondLength) {
				return 0;
			}
			try {
				fractionalSeconds = std::stoi(printable.substr(printable.length()-fractionalSecondLength));
			} catch (std::invalid_argument& e) {
				return 0; // perhaps throw instead?
			}
		}
	}

#if __GNUC__ > 4
	std::istringstream ss(printable);
	// ss.imbue(std::locale("de_DE.utf-8"));  TODO: handle locale as preference
	ss >> std::get_time(&t, format.c_str());
	if (ss.fail()) {
		return 0;
	}
#else
    if (!strptime(printable.c_str(), format.c_str(), &t)) {
        return 0;
    }
#endif

	std::time_t secondsSinceEpoch = std::mktime(&t);

	// take care of microseconds
	if (fractionalSecondLength > 0) {
		while (fractionalSecondLength < 6) {
			fractionalSeconds *= 10;
			fractionalSecondLength++;
		}
	}

	uint64_t ret = uint64_t(secondsSinceEpoch) * MICROSECONDS_PER_SECOND + fractionalSeconds;

	return ret;
}
