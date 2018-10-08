#include "conversions.h"

static std::string conv_identity(std::string& s) {
	return s;
}

static std::string conv_C2B(std::string& s) {
	std::string ret;
	for (int i=0; i<s.length(); i++) {
		unsigned char c = (unsigned char)s[i];
		for (int j=0; j<8; j++) {
			ret += (c>=128) ? '1' : '0';
			c <<= 1;
		}
	}

	return ret;
}

static std::string conv_C2X(std::string& s) {
	static char hexchars[]="0123456789abcdef";
	std::string ret;
	for (int i=0; i<s.length(); i++) {
		unsigned char c = (unsigned char)s[i];
		ret += hexchars[c>>4];
		ret += hexchars[c%16];
	}

	return ret;
}

static std::string conv_B2C(std::string& s) {
	return s;
}

static std::string conv_X2C(std::string& s) {
	return s;
}

static std::string conv_d2x(std::string& s) {
	return s;
}

static std::string conv_x2d(std::string& s) {
	return s;
}

#define X(c) if (s==#c) return StringConversion__##c;
StringConversions getConversionByName(std::string& s)
{
	if (s=="identity") return StringConversion__NONE;
	STRING_CONVERSIONS_LIST
	return StringConversion__NONE;
}
#undef X

#define X(c) case StringConversion__##c: return conv_##c(source);
std::string stringConvert(std::string& source, StringConversions conv)
{
	switch (conv) {
	STRING_CONVERSIONS_LIST
	default:
		throw std::invalid_argument("Bad conversion "+StringConversion__2str(conv));
	}
}
