#ifndef SPECS2016__UTILS__TIMEUTILS__H
#define SPECS2016__UTILS__TIMEUTILS__H

#include <cstdint>
#include <string>
#include "utils/SpecString.h"

uint64_t specTimeGetTOD();

PSpecString specTimeConvertToPrintable(uint64_t sinceEpoch, std::string format);

uint64_t specTimeConvertFromPrintable(std::string printable, std::string format);

#endif
