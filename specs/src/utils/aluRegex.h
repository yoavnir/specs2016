#ifndef SPECS2016__UTILS__ALU_REGEX_H
#define SPECS2016__UTILS__ALU_REGEX_H

#include <string>
#include "utils/aluValue.h"

void dumpRegexStats();

void disableRegexCache();

void setRegexType(std::string& s);

bool regexMatch(std::string* pStr, PValue pExp, std::string* pFlags = NULL);

bool regexSearch(std::string* pStr, PValue pExp, std::string* pFlags = NULL);

std::string regexReplace(std::string* pStr, PValue pExp, std::string& fmt, std::string* pFlags = NULL);

#endif
