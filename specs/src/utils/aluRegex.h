#ifndef SPECS2016__UTILS__ALU_REGEX_H
#define SPECS2016__UTILS__ALU_REGEX_H

#include <string>
#include "utils/aluValue.h"

void dumpRegexStats();

void setRegexType(std::string& s);

bool regexMatch(ALUValue* pStr, ALUValue* pExp, std::string* pFlags = NULL);

bool regexSearch(ALUValue* pStr, ALUValue* pExp, std::string* pFlags = NULL);

std::string regexReplace(ALUValue* pStr, ALUValue* pExp, std::string& fmt, std::string* pFlags = NULL);

#endif
