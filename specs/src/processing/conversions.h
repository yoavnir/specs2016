#ifndef SPECS2016__PROCESSING__CONVERSIONS__H
#define SPECS2016__PROCESSING__CONVERSIONS__H

#include <string>

#define STRING_CONVERSIONS_LIST  \
	X(identity) \
	X(C2B)    /* bytes to binary string:  "AB" --> "0010000100100010" */  \
	X(C2X)    /* bytes to hex: "AB" --> "4142" */  \
	X(B2C)  \
	X(X2C)  \
	X(d2x)    /* convert decimal number to hex: "314159265" --> "12b9b0a1" */ \
	X(x2d)

#define X(c) StringConversion__##c,
enum StringConversions {
	STRING_CONVERSIONS_LIST
	StringConversion__COUNT_ITEMS,
	StringConversion__NONE
};
#undef X

#define X(c) #c,
static std::string StringConversionStrArr[StringConversion__COUNT_ITEMS] = {\
	STRING_CONVERSIONS_LIST
};
#undef X

static inline std::string& StringConversion__2str(StringConversions tok) {
	return StringConversionStrArr[tok];
}

std::string stringConvert(std::string& source, StringConversions conv);
StringConversions getConversionByName(std::string& s);

#endif
