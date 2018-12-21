#ifndef SPECS2016__PROCESSING__CONVERSIONS__H
#define SPECS2016__PROCESSING__CONVERSIONS__H

#include <string>

#define STRING_CONVERSIONS_LIST  \
	X(identity) \
	X(ROT13)  \
	X(C2B)    /* bytes to binary string:  "AB" --> "0010000100100010" */  \
	X(C2X)    /* bytes to hex: "AB" --> "4142" */  \
	X(B2C)  \
	X(X2CH) \
	X(D2X)    /* convert decimal number to hex: "314159265" --> "12b9b0a1" */ \
	X(X2D)  \
	X(LCASE) \
	X(UCASE) \
	X(BSWAP) \

#define PARAMETRIZED_CONVERSIONS_LIST \
	Y(ti2f)   /* internal time representation (microseconds since epoch) to format */ \
	Y(tf2i)   /* internal time representation (microseconds since epoch) to format */ \

#define X(c) StringConversion__##c,
#define Y(c) StringConversion__##c,
enum StringConversions {
	STRING_CONVERSIONS_LIST
	PARAMETRIZED_CONVERSIONS_LIST
	StringConversion__COUNT_ITEMS,
	StringConversion__NONE
};
#undef X
#undef Y

#define X(c) #c,
#define Y(c) #c,
static std::string StringConversionStrArr[StringConversion__COUNT_ITEMS] = {\
	STRING_CONVERSIONS_LIST
	PARAMETRIZED_CONVERSIONS_LIST
};
#undef X
#undef Y

static inline std::string& StringConversion__2str(StringConversions tok) {
	return StringConversionStrArr[tok];
}

#define Y(c) case StringConversion__##c: return true;
static bool isParametrizedConversion(StringConversions conv)
{
	switch(conv) {
	PARAMETRIZED_CONVERSIONS_LIST
	default:
		return false;
	}
}
#undef Y

std::string stringConvert(std::string& source, StringConversions conv, std::string& param);
StringConversions getConversionByName(std::string& s);

#endif
