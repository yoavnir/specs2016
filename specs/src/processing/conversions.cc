#include <string.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include "utils/platform.h"
#include "utils/ErrorReporting.h"
#include "conversions.h"
#include "utils/SpecString.h"
#include "utils/TimeUtils.h"

static std::string conv_identity(std::string& s) {
	return s;
}

static std::string conv_ROT13(std::string& s) {
	std::string ret;
	ret.resize(s.length());
	for (size_t i=0; i<s.length(); i++) {
		if ((s[i]>='A' && s[i]<='M') || (s[i]>='a' && s[i]<='m')) {
			ret[i] = s[i] + 13;
		} else if ((s[i]>='N' && s[i]<='Z') || (s[i]>='n' && s[i]<='z')) {
			ret[i] = s[i] - 13;
		} else {
			ret[i] = s[i];
		}
	}
	return ret;
}

static std::string conv_C2B(std::string& s) {
	std::string ret;
	ret.resize(s.length()*8);
	for (size_t i=0; i<s.length(); i++) {
		unsigned char c = (unsigned char)s[i];
		for (int j=0; j<8; j++) {
			ret[i*8+j] = (c>=128) ? '1' : '0';
			c <<= 1;
		}
	}

	return ret;
}

std::string conv_C2X(std::string& s) {
	static char hexchars[]="0123456789abcdef";
	std::string ret;
	ret.resize(s.length()*2);
	for (size_t i=0; i<s.length(); i++) {
		unsigned char c = (unsigned char)s[i];
		ret[i*2] = hexchars[c>>4];
		ret[i*2+1] = hexchars[c%16];
	}

	return ret;
}

static std::string conv_B2C(std::string& s) {
	std::string ret;
	ret.resize((s.length()+7)/8);
	unsigned char c = 0;
	for (size_t i=0; i<s.length(); i++) {
		c <<= 1;
		switch (s[i]) {
		case '1': c += 1;  break;
		case '0': break;
		default: CONVERSION_EXCEPTION(s, "Binary", "Char");
		}

		if (7==i%8 || i==(s.length() - 1)) {
			ret[i/8] = char(c);
			c = 0;
		}
	}
	return ret;
}

std::string conv_X2CH(std::string& s) {
	std::string ret;
	if (1==s.length() % 2) {
		CONVERSION_EXCEPTION(s, "Hex", "Char");
	}

	const char* pc = s.c_str();
	for (size_t i=0; i < s.length(); i+=2) {
		char c;
		if (*pc>='0' && *pc<='9') {
			c = *pc-'0';
		} else if (*pc>='A' && *pc<='F') {
			c = *pc - 'A' + 10;
		} else if (*pc>='a' && *pc<='f') {
			c = *pc - 'a' + 10;
		} else {
			CONVERSION_EXCEPTION(s, "Hex", "Char");
		}
		c <<= 4; pc++;
		if (*pc>='0' && *pc<='9') {
			c += (*pc-'0');
		} else if (*pc>='A' && *pc<='F') {
			c += (*pc - 'A' + 10);
		} else if (*pc>='a' && *pc<='f') {
			c += (*pc - 'a' + 10);
		} else {
			CONVERSION_EXCEPTION(s, "Hex", "Char");
		}

		ret += c;  pc++;
	}
	return ret;
}

std::string conv_D2X(std::string& s) {
	unsigned long long value;
	try {
		try {
			value = std::stoull(s);
		} catch (std::invalid_argument&) {
			CONVERSION_EXCEPTION(s, "Decimal", "Hex");
		}
	} catch (std::out_of_range&) {
		CONVERSION_EXCEPTION_EX(s, "Decimal", "Hex", "out of range")
	}

	std::ostringstream oss;
	oss << std::hex << value;
	return oss.str();
}

std::string conv_X2D(std::string& s) {
	unsigned long long value;
	try {
		try {
			value = std::stoull(s, nullptr, 16);
		} catch (std::invalid_argument&) {
			CONVERSION_EXCEPTION(s, "Hex", "Decimal");
		}
	} catch (std::out_of_range&) {
		CONVERSION_EXCEPTION_EX(s, "Hex", "Decimal", "out of range")
	}

	return std::to_string(value);
}

std::string conv_BSWAP(std::string& s) {
	std::string ret(s.size(), 0);
	const char* sStart = s.c_str();
	char* retPtr = (char*)(ret.c_str());
	char* sIt = (char*)(sStart + s.size());
	while (sIt >= sStart) {
		*retPtr++ = *(--sIt);
	}
	return ret;
}

std::string conv_LCASE(std::string& s) {
	std::string ret(s);
	for (auto & c : ret) c = tolower(c);
	return ret;
}

std::string conv_UCASE(std::string& s) {
	std::string ret(s);
	for (auto & c : ret) c = toupper(c);
	return ret;
}

static std::string conv_ti2f(std::string& s, std::string& parm)
{
	if (s.length()!=8) return std::string();
	int64_t internal = *((int64_t*)(s.c_str()));
	PSpecString pRet = specTimeConvertToPrintable(internal, parm);
	std::string ret = std::string(pRet->data());
	return ret;
}

static std::string conv_tf2i(std::string& s, std::string& parm)
{
	int64_t ret = specTimeConvertFromPrintable(s,parm);
	return std::string(((char*)(&ret)), sizeof(int64_t));
}

static std::string conv_s2tf(std::string& s, std::string& parm)
{
	int64_t internal = int64_t(std::stold(s) * MICROSECONDS_PER_SECOND + 0.5);
	PSpecString pRet = specTimeConvertToPrintable(internal, parm);
	std::string ret = std::string(pRet->data());
	return ret;
}

static std::string conv_tf2s(std::string& s, std::string& parm)
{
	int64_t tm = specTimeConvertFromPrintable(s,parm);
	long double seconds;
	if (0 == (tm % MICROSECONDS_PER_SECOND)) {
		seconds = (long double)(tm / MICROSECONDS_PER_SECOND);
	} else {
		seconds = ((long double)tm) / MICROSECONDS_PER_SECOND;
	}
	return std::to_string(seconds);
}

static std::string conv_mcs2tf(std::string& s, std::string& parm)
{
	int64_t internal = int64_t(std::stold(s) + 0.5);
	PSpecString pRet = specTimeConvertToPrintable(internal, parm);
	std::string ret = std::string(pRet->data());
	return ret;
}

static std::string conv_tf2mcs(std::string& s, std::string& parm)
{
	int64_t tm = specTimeConvertFromPrintable(s,parm);
	return std::to_string(tm);
}


#define X(c) if (0==strcasecmp(s.c_str(),#c)) return StringConversion__##c;
#define Y(c) if (0==strcasecmp(s.c_str(),#c)) return StringConversion__##c;
StringConversions getConversionByName(std::string& s)
{
	if (s=="identity") return StringConversion__NONE;
	STRING_CONVERSIONS_LIST
	PARAMETRIZED_CONVERSIONS_LIST
	return StringConversion__NONE;
}
#undef X
#undef Y

#define X(c) case StringConversion__##c: return conv_##c(source);
#define Y(c) case StringConversion__##c: return conv_##c(source, param);
std::string stringConvert(std::string& source, StringConversions conv, std::string& param)
{
	switch (conv) {
	STRING_CONVERSIONS_LIST
	PARAMETRIZED_CONVERSIONS_LIST
	default:
		std::string err = "Bad conversion: " + StringConversion__2str(conv);
		MYTHROW(err);
		return err;  // to appease the compiler
	}
}
