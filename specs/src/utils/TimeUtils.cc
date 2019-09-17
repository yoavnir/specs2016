#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <locale>
#include <stdlib.h> // defines setenv
#include "utils/ErrorReporting.h"
#include "platform.h"  // For put_time and get_time vs strftime and strptime
#include "TimeUtils.h"

using TimeResolution = std::chrono::microseconds;

using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;

static std::locale g_locale;

clockValue specTimeGetTOD()
{
	auto currentTime = std::chrono::system_clock::now();
	auto sinceEpoch = currentTime.time_since_epoch();
	int64_t microseconds = std::chrono::duration_cast<TimeResolution>(sinceEpoch).count();
	return (clockValue(microseconds));
}


PSpecString specTimeConvertToPrintable(int64_t sinceEpoch, std::string format)
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

#ifdef PUT_TIME__SUPPORTED
	oss.imbue(g_locale);
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

int64_t specTimeConvertFromPrintable(std::string printable, std::string format)
{
	// initialize t in case of missing fields
	std::time_t now = std::time(nullptr);
	std::tm t = *(std::localtime(&now));

	unsigned int fractionalSeconds = 0;
	unsigned char fractionalSecondLength = 0;
	std::string fractionalPart;

	if (format.length() >= 3) {
		auto l = format.length();
		if (format[l-3]=='%' && format[l-1]=='f' &&
				format[l-2]>='0' && format[l-2]<='6') {
			fractionalSecondLength = format[l-2] - '0';
			format = format.substr(0,l-3);
		}
	}

#ifdef PUT_TIME__SUPPORTED
	std::istringstream ss(printable);
	ss.imbue(g_locale);
	ss >> std::get_time(&t, format.c_str());
	if (ss.fail()) {
		return 0;
	}
	if (fractionalSecondLength > 0) {
		std::getline(ss, fractionalPart);
		if (fractionalPart.length() != fractionalSecondLength) {
			fractionalPart.resize(fractionalSecondLength, '0');
		}
	}
#else
	char* fractionalPartPtr = strptime(printable.c_str(), format.c_str(), &t);
	if (!fractionalPartPtr) {
		return 0;
	}
	if (fractionalSecondLength > 0) {
		fractionalPart = fractionalPartPtr;
		if (fractionalPart.length() != fractionalSecondLength) {
			fractionalPart.resize(fractionalSecondLength, '0');
		}
	}
#endif

	// automatic detection of DST - Issue #2
	t.tm_isdst = -1;
	std::time_t secondsSinceEpoch = std::mktime(&t);

	// take care of microseconds
	if (fractionalSecondLength > 0) {
		try {
			int extraZeros = 6 - fractionalPart.length();
			fractionalSeconds = std::stoi(fractionalPart);
			while (extraZeros > 0) {
				fractionalSeconds *= 10;
				extraZeros--;
			}
		} catch (std::invalid_argument& e) {
			fractionalSeconds = 0;
		}
	}

	uint64_t ret = uint64_t(secondsSinceEpoch) * MICROSECONDS_PER_SECOND + fractionalSeconds;

	return ret;
}

void specTimeSetTimeZone(const std::string& tzname)
{
	static const char sTZ[] = "TZ";
	setenv(sTZ, tzname.data(), 1);
}

void specTimeSetLocale(const std::string& _locale)
{
	try {
		g_locale = std::locale(_locale.c_str());
	} catch(std::runtime_error& e) {
		std::string err = "Invalid locale <" + _locale + ">";
		MYTHROW(err);
	}
}

