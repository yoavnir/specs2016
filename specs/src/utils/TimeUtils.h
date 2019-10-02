#ifndef SPECS2016__UTILS__TIMEUTILS__H
#define SPECS2016__UTILS__TIMEUTILS__H

#include <cstdint>
#include <string>
#include <chrono>
#include "utils/SpecString.h"

#define MICROSECONDS_PER_SECOND 1000000
#define NANOSECONDS_PER_SECOND double(1000000000.0)
#define NANOSECONDS_PER_MILLISECOND double(1000000.0)
#define NANOSECONDS_PER_MICROSECOND double(1000.0)

using HClock = std::chrono::high_resolution_clock;
using HTimePoint = std::chrono::time_point<HClock>;
using SClock = std::chrono::system_clock;
using STimePoint = std::chrono::time_point<SClock>;

typedef int64_t clockValue;

clockValue specTimeGetTOD();

PSpecString specTimeConvertToPrintable(int64_t sinceEpoch, std::string format);

int64_t specTimeConvertFromPrintable(std::string printable, std::string format);

void specTimeSetTimeZone(const std::string& tzname);

void specTimeSetLocale(const std::string& _locale);

enum timeClasses{
	timeClassInitializing,
	timeClassProcessing,
	timeClassIO,
	timeClassInputQueue,
	timeClassOutputQueue,
	timeClassDraining,
	timeClassLast
};

class classifyingTimer {
public:
	classifyingTimer();
	void     changeClass(timeClasses _class);
	void     dump(std::string title);
private:
	std::chrono::time_point<HClock> m_lastTimePoint;
	timeClasses m_currentClass;
	uint64_t m_nanoseconds[timeClassLast];
	uint64_t getNanoSeconds(timeClasses _class) { return m_nanoseconds[_class]; }
	double   getSeconds(timeClasses _class) { return double(m_nanoseconds[_class]) / NANOSECONDS_PER_SECOND; }
	double   getMilliSeconds(timeClasses _class) { return double(m_nanoseconds[_class]) / NANOSECONDS_PER_MILLISECOND; }
	double   getMicroSeconds(timeClasses _class) { return double(m_nanoseconds[_class]) / NANOSECONDS_PER_MICROSECOND; }
};

#endif
