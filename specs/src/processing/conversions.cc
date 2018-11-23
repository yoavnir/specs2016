#include "utils/ErrorReporting.h"
#include "conversions.h"
#include "utils/SpecString.h"
#include "utils/TimeUtils.h"

static std::string conv_identity(std::string& s) {
	return s;
}

static std::string conv_C2B(std::string& s) {
	std::string ret;
	ret.resize(s.length()*8);
	for (int i=0; i<s.length(); i++) {
		unsigned char c = (unsigned char)s[i];
		for (int j=0; j<8; j++) {
			ret[i*8+j] = (c>=128) ? '1' : '0';
			c <<= 1;
		}
	}

	return ret;
}

static std::string conv_C2X(std::string& s) {
	static char hexchars[]="0123456789abcdef";
	std::string ret;
	ret.resize(s.length()*2);
	for (int i=0; i<s.length(); i++) {
		unsigned char c = (unsigned char)s[i];
		ret[i*2] = hexchars[c>>4];
		ret[i*2+1] = hexchars[c%16];
	}

	return ret;
}

static std::string conv_B2C(std::string& s) {
	return s;
}

std::string conv_X2C(std::string& s) {
	std::string ret;
	if (1==s.length() % 2) {
		CONVERSION_EXCEPTION(s, "Hex", "Char");
	}

	const char* pc = s.c_str();
	for (int i=0; i < s.length(); i+=2) {
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

static std::string conv_d2x(std::string& s) {
	return s;
}

static std::string conv_x2d(std::string& s) {
	return s;
}

static std::string conv_ti2f(std::string& s, std::string& parm)
{
	if (s.length()!=8) return std::string();
	uint64_t internal = *((uint64_t*)(s.c_str()));
	PSpecString pRet = specTimeConvertToPrintable(internal, parm);
	std::string ret = std::string(pRet->data());
	delete pRet;
	return ret;
}

static std::string conv_tf2i(std::string& s, std::string& parm)
{
	uint64_t ret = specTimeConvertFromPrintable(s,parm);
	return std::string(((char*)(&ret)), sizeof(uint64_t));
}

#define X(c) if (s==#c) return StringConversion__##c;
#define Y(c) if (s==#c) return StringConversion__##c;
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
