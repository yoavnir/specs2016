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

template<typename... Args>
PSpecString mkSpecString(Args&&... args)
{
	return std::make_shared<std::string>(std::forward<Args>(args)...);
}

// The pad is a single character, which in UTF-8 mode may be several bytes long.
// In UTF-8 mode newSize is measured in characters rather than bytes.
void SpecString_Resize(PSpecString ps, size_t newSize, const std::string& pad, outputAlignment oa, ellipsisSpec es);

// count copies of the pad character
std::string SpecString_Pad(const std::string& pad, size_t count);

#endif
