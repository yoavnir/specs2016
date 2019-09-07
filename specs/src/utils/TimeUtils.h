#ifndef SPECS2016__UTILS__TIMEUTILS__H
#define SPECS2016__UTILS__TIMEUTILS__H

#include <cstdint>
#include <string>
#include "utils/SpecString.h"

#define MICROSECONDS_PER_SECOND 1000000

typedef long double clockValue;

clockValue specTimeGetTOD();

PSpecString specTimeConvertToPrintable(int64_t sinceEpoch, std::string format);

int64_t specTimeConvertFromPrintable(std::string printable, std::string format);

void specTimeSetTimeZone(const std::string& tzname);

void specTimeSetLocale(const std::string& _locale);

#endif
