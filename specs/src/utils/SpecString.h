#ifndef SPECS2016__UTILS_SPECSTRING__H
#define SPECS2016__UTILS_SPECSTRING__H

#include <iostream>
#include <memory>
#include <string>

#define MAX_STR_LEN 65535
#define INVALID_LENGTH (MAX_STR_LEN+1)

enum outputAlignment {
	outputAlignmentLeft,
	outputAlignmentCenter,
	outputAlignmentRight,
	outputAlignmentComposed
};

enum ellipsisSpec {
	ellipsisSpecNone,
	ellipsisSpecLeft,
	ellipsisSpecThird,
	ellipsisSpecHalf,
	ellipsisSpecTwoThirds,
	ellipsisSpecRight
};

typedef std::shared_ptr<std::string> PSpecString;


void SpecString_Resize(PSpecString ps, size_t newSize, void* pPadChar, outputAlignment oa, ellipsisSpec es);

void SpecString_Resize(PSpecString ps, size_t newSize, char pPadChar, outputAlignment oa, ellipsisSpec es);

#endif
