#include "utils/ErrorReporting.h"
#include "utils/aluFunctions.h"
#include "utils/TimeUtils.h"
#include "utils/aluRand.h"
#include "processing/Config.h"
#include <string.h>
#include <cmath>
#include <functional>
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm> // for std::reverse

#define PAD_CHAR ' '

#define PERCENTS (ALUFloat(100.0))

stateQueryAgent* g_pStateQueryAgent = NULL;
positionGetter* g_PositionGetter = NULL;

void setStateQueryAgent(stateQueryAgent* qa)
{
	g_pStateQueryAgent = qa;
}

void setPositionGetter(positionGetter* pGetter)
{
	g_PositionGetter = pGetter;
}

static void throw_argument_issue(const char* _funcName, unsigned int argIdx, const char* argName, const char* message)
{
	std::string err(_funcName);

	// Remove the AluFunc_ prefix
	err = err.substr(8) + ": " + message;

	if (argIdx>0) {
		err += ": #" + std::to_string(argIdx);
		if (argName) {
			err = err + " (" + argName + ")";
		}
	}

	MYTHROW(err);
}

#define ASSERT_NOT_ELIDED(arg,idx,name)     \
	if (NULL == (arg)) { throw_argument_issue(__func__,idx,#name,"Argument must not be elided"); }

#define THROW_ARG_ISSUE(idx,name,msg)       \
		throw_argument_issue(__func__,idx,#name,msg.c_str());

#define ARG_INT_WITH_DEFAULT(arg,def)       \
		((NULL == (arg)) ? def : (arg)->getInt())

#define ARG_STR_WITH_DEFAULT(arg,def)       \
		((NULL == (arg)) ? def : (arg)->getStr())

/*
 *
 *
 * ALU FUNCTIONS
 * =============
 *
 *
 */

ALUValue* AluFunc_abs(ALUValue* op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	if (op->getType()==counterType__Int) {
		ALUInt i = op->getInt();
		if (i<0) i = -i;
		return new ALUValue(i);
	} else {
		ALUFloat f = op->getFloat();
		if (f<0) f = -f;
		return new ALUValue(f);
	}
}

ALUValue* AluFunc_pow(ALUValue* op1, ALUValue* op2)
{
	ASSERT_NOT_ELIDED(op1,1,base);
	ASSERT_NOT_ELIDED(op2,2,exponent);
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return new ALUValue(std::pow(op1->getFloat(), op2->getFloat()));
	}
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
		return new ALUValue(ALUInt(std::pow(op1->getInt(), op2->getInt())));
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return new ALUValue(ALUInt(std::pow(op1->getInt(), op2->getInt())));
	}
	return new ALUValue(std::pow(op1->getFloat(), op2->getFloat()));
}

ALUValue* AluFunc_sqrt(ALUValue* op)
{
	ASSERT_NOT_ELIDED(op,1,square);
	return new ALUValue(std::sqrt(op->getFloat()));
}

// Both of the following functions assume little-endian architecture
// The mainframe version and Solaris version will need some work...
static uint64_t binary2uint64(ALUValue* op, unsigned char *pNumBits = NULL)
{
	std::string str = op->getStr();
	uint64_t value = 0;

	switch (str.length()) {
	case 1: {
		value = ALUInt((unsigned char)(str[0]));
		if (pNumBits) *pNumBits = 1;
		break;
	}
	case 2: {
		uint16_t* pVal = (uint16_t*)str.c_str();
		value = *pVal;
		if (pNumBits) *pNumBits = 2;
		break;
	}
	case 3: {
		uint32_t tmp = 0;
		memcpy((char*)&tmp, str.c_str(), str.length());
		str = std::string((char*)&tmp, sizeof(tmp));
		/* intentional fall-through */
	}
	case 4: {
		uint32_t* pVal = (uint32_t*)str.c_str();
		value = *pVal;
		if (pNumBits) *pNumBits = 4;
		break;
	}
	case 5:
	case 6:
	case 7: {
		uint64_t tmp = 0;
		memcpy((char*)&tmp, str.c_str(), str.length());
		str = std::string((char*)&tmp, sizeof(tmp));
		/* intentional fall-through */
	}
	case 8: {
		uint64_t* pVal = (uint64_t*)str.c_str();
		value = *pVal;
		if (pNumBits) *pNumBits = 8;
		break;
	}
	default: {
		std::string err = "c2u/c2d: Invalid input length: " + std::to_string(str.length());
		MYTHROW(err);
	}
	}

	return value;
}

ALUValue* AluFunc_c2u(ALUValue* op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	uint64_t value = binary2uint64(op);
	MYASSERT_WITH_MSG(value <= MAX_ALUInt, "c2u: Binary value exceeds limit");
	return new ALUValue(ALUInt(value));
}

ALUValue* AluFunc_c2d(ALUValue* op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	unsigned char numBytes;
	uint64_t uvalue = binary2uint64(op, &numBytes);
	switch (numBytes) {
	case 1: {
		char *pvalue = (char*)(&uvalue);
		return new ALUValue(ALUInt(*pvalue));
	}
	case 2: {
		int16_t *pvalue = (int16_t*)(&uvalue);
		return new ALUValue(ALUInt(*pvalue));
	}
	case 4: {
		int32_t *pvalue = (int32_t*)(&uvalue);
		return new ALUValue(ALUInt(*pvalue));
	}
	case 8: {
		int64_t *pvalue = (int64_t*)(&uvalue);
		return new ALUValue(ALUInt(*pvalue));
	}
	default: {
		std::string err = "Invalid number of bytes: " + std::to_string(numBytes);
		MYTHROW(err);
	}
	}

	return NULL;
}

ALUValue* AluFunc_c2f(ALUValue* op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	std::string str = op->getStr();

	if (str.length() == sizeof(float)) {
		float* pf = (float*) str.c_str();
		return new ALUValue(ALUFloat(*pf));
	} else if (str.length() == sizeof(double)) {
		double *pd = (double*) str.c_str();
		return new ALUValue(ALUFloat(*pd));
	} else if (str.length() == sizeof(long double)) {
		long double *pld = (long double*) str.c_str();
		return new ALUValue(ALUFloat(*pld));
	} else {
		std::string err = "c2f: Invalid floating point length: " + std::to_string(str.length()) +
				". Supported lengths: " + std::to_string(sizeof(float));
		if (sizeof(float) < sizeof(double)) {
			err += ", " + std::to_string(sizeof(double));
		}
		if (sizeof(double) < sizeof(long double)) {
			err += ", " + std::to_string(sizeof(long double));
		}
		MYTHROW(err);
	}
}



ALUValue* AluFunc_frombin(ALUValue* op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	uint64_t value = binary2uint64(op);
	return new ALUValue(ALUInt(value));
}

ALUValue* AluFunc_tobine(ALUValue* op, ALUValue* _bits)
{
	ASSERT_NOT_ELIDED(op,1,op);
	ASSERT_NOT_ELIDED(_bits,2,bits);
	ALUInt value = op->getInt();
	ALUInt bits = _bits->getInt();
	switch (bits) {
	case 8:
	case 16:
	case 32:
	case 64:
		return new ALUValue((char*)&value, bits/8);
	default: {
		std::string err = "Invalid bit length " + _bits->getStr();
		MYTHROW(err);
	}
	}
}

ALUValue* AluFunc_tobin(ALUValue* op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	static ALUValue bit8(ALUInt(8));
	static ALUValue bit16(ALUInt(16));
	static ALUValue bit32(ALUInt(32));
	static ALUValue bit64(ALUInt(64));
	ALUInt value = op->getInt();

	if (0 == (value >> 8)) return AluFunc_tobine(op,&bit8);
	if (0 == (value >> 16)) return AluFunc_tobine(op,&bit16);
	if (0 == (value >> 32)) return AluFunc_tobine(op,&bit32);
	return AluFunc_tobine(op,&bit64);
}

ALUValue* AluFunc_length(ALUValue* op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	return new ALUValue(ALUInt(op->getStr().length()));
}

ALUValue* AluFunc_first()
{
	bool isFirst = g_pStateQueryAgent->isRunIn();
	return new ALUValue(ALUInt(isFirst ? 1 : 0));
}

ALUValue* AluFunc_number()
{
	return new ALUValue(g_pStateQueryAgent->getIterationCount());
}

ALUValue* AluFunc_recno()
{
	return new ALUValue(g_pStateQueryAgent->getRecordCount());
}

ALUValue* AluFunc_eof()
{
	bool isRunOut = g_pStateQueryAgent->isRunOut();
	return new ALUValue(ALUInt(isRunOut ? 1 : 0));
}

ALUValue* AluFunc_wordcount()
{
	return new ALUValue(ALUInt(g_pStateQueryAgent->getWordCount()));
}

ALUValue* AluFunc_fieldcount()
{
	return new ALUValue(ALUInt(g_pStateQueryAgent->getFieldCount()));
}

// Helper function
static ALUValue* AluFunc_range(ALUInt start, ALUInt end)
{
	PSpecString pRange = g_pStateQueryAgent->getFromTo(start, end);
	if (pRange) {
		ALUValue *pRet = new ALUValue(pRange->data());
		delete pRange;
		return pRet;
	} else {
		return new ALUValue("");
	}
}

ALUValue* AluFunc_record()
{
	return AluFunc_range(1,-1);
}

ALUValue* AluFunc_range(ALUValue* pStart, ALUValue* pEnd)
{
	ALUInt start = ARG_INT_WITH_DEFAULT(pStart,1);
	ALUInt end = ARG_INT_WITH_DEFAULT(pEnd, -1);
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_word(ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	ALUInt idx = pIdx->getInt();
	ALUInt start = g_pStateQueryAgent->getWordStart(idx);
	ALUInt end = g_pStateQueryAgent->getWordEnd(idx);
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_field(ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	ALUInt idx = pIdx->getInt();
	ALUInt start = g_pStateQueryAgent->getFieldStart(idx);
	ALUInt end = g_pStateQueryAgent->getFieldEnd(idx);
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_wordrange(ALUValue* pStart, ALUValue* pEnd)
{
	ALUInt startIdx = ARG_INT_WITH_DEFAULT(pStart, 1);
	ALUInt endIdx = ARG_INT_WITH_DEFAULT(pEnd, -1);
	ALUInt start = g_pStateQueryAgent->getWordStart(startIdx);
	ALUInt end = g_pStateQueryAgent->getWordEnd(endIdx);
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_fieldrange(ALUValue* pStart, ALUValue* pEnd)
{
	ALUInt startIdx = ARG_INT_WITH_DEFAULT(pStart, 1);
	ALUInt endIdx = ARG_INT_WITH_DEFAULT(pEnd, -1);
	ALUInt start = g_pStateQueryAgent->getFieldStart(startIdx);
	ALUInt end = g_pStateQueryAgent->getFieldEnd(endIdx);
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_fieldindex(ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	return new ALUValue(ALUInt(g_pStateQueryAgent->getFieldStart(pIdx->getInt())));
}

ALUValue* AluFunc_fieldend(ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	return new ALUValue(ALUInt(g_pStateQueryAgent->getFieldEnd(pIdx->getInt())));
}

ALUValue* AluFunc_fieldlength(ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	auto idx = pIdx->getInt();
	auto len = g_pStateQueryAgent->getFieldEnd(idx) - g_pStateQueryAgent->getFieldStart(idx) + 1;
	return new ALUValue(ALUInt(len));
}

ALUValue* AluFunc_wordstart(ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	return new ALUValue(ALUInt(g_pStateQueryAgent->getWordStart(pIdx->getInt())));
}

ALUValue* AluFunc_wordend(ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	return new ALUValue(ALUInt(g_pStateQueryAgent->getWordEnd(pIdx->getInt())));
}

ALUValue* AluFunc_wordlen(ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	auto idx = pIdx->getInt();
	auto len = g_pStateQueryAgent->getWordEnd(idx) - g_pStateQueryAgent->getWordStart(idx) + 1;
	return new ALUValue(ALUInt(len));
}

ALUValue* AluFunc_tf2d(ALUValue* pTimeFormatted, ALUValue* pFormat)
{
	ASSERT_NOT_ELIDED(pTimeFormatted,1,formatted_time);
	ASSERT_NOT_ELIDED(pFormat,2,format);
	int64_t tm = specTimeConvertFromPrintable(pTimeFormatted->getStr(), pFormat->getStr());
	return new ALUValue(ALUInt(tm));
}

ALUValue* AluFunc_d2tf(ALUValue* pValue, ALUValue* pFormat)
{
	ASSERT_NOT_ELIDED(pValue,1,time_value);
	ASSERT_NOT_ELIDED(pFormat,2,format);
	int64_t microseconds = pValue->getInt();
	PSpecString printable = specTimeConvertToPrintable(microseconds, pFormat->getStr());
	ALUValue* ret = new ALUValue(printable->data(), printable->length());
	delete printable;
	return ret;
}

ALUValue* AluFunc_tf2s(ALUValue* pTimeFormatted, ALUValue* pFormat)
{
	ASSERT_NOT_ELIDED(pTimeFormatted,1,formatted_time);
	ASSERT_NOT_ELIDED(pFormat,2,format);
	int64_t tm = specTimeConvertFromPrintable(pTimeFormatted->getStr(), pFormat->getStr());
        ALUFloat seconds = (tm/MICROSECONDS_PER_SECOND);
        ALUFloat microseconds = (ALUFloat)(tm%MICROSECONDS_PER_SECOND)/MICROSECONDS_PER_SECOND;
        return new ALUValue(ALUFloat(seconds+microseconds));
}

ALUValue* AluFunc_s2tf(ALUValue* pValue, ALUValue* pFormat)
{
	ASSERT_NOT_ELIDED(pValue,1,time_value);
	ASSERT_NOT_ELIDED(pFormat,2,format);
        ALUFloat seconds = pValue->getFloat();
        int64_t microseconds = seconds * MICROSECONDS_PER_SECOND;
	PSpecString printable = specTimeConvertToPrintable(microseconds, pFormat->getStr());
	ALUValue* ret = new ALUValue(printable->data(), printable->length());
	delete printable;
	return ret;
}


// Substring functions

static ALUValue* AluFunc_substring_do(ALUValue* pBigString, ALUInt start, ALUInt length)
{
	std::string* pStr = pBigString->getStrPtr();

	// handle start
	if (start==0) {    // invalid string index in specs
		return new ALUValue();  // NaN
	}
	else if (start > ALUInt(pStr->length())) {
		return new ALUValue("",0);
	} else if (start < 0) {
		start += pStr->length() + 1;
		if (start < 1) {
			return new ALUValue("",0);
		}
	}

	// handle length
	if (length < 0) {
		length += pStr->length() + 1; // length=-1 means the length of the string
		if (length < 0) {
			return new ALUValue("",0);
		}
	}
	if (size_t(start + length - 1) > pStr->length()) {
		length = pStr->length() - start + 1;
	}

	// Finally:
	return new ALUValue(pStr->substr(start-1,length));
}

ALUValue* AluFunc_substr(ALUValue* pBigString, ALUValue* pStart, ALUValue* pLength)
{
	ALUInt start = ARG_INT_WITH_DEFAULT(pStart,1);
	ALUInt length = ARG_INT_WITH_DEFAULT(pLength,-1);
	return AluFunc_substring_do(pBigString, start, length);
}

ALUValue* AluFunc_left(ALUValue* pBigString, ALUValue* pLength)
{
	ASSERT_NOT_ELIDED(pBigString,1,bigString);
	ASSERT_NOT_ELIDED(pLength,2,length);
	auto bigLength = pBigString->getStrPtr()->length();
	ALUInt len = pLength->getInt();
	if (len==0) return new ALUValue("",0);
	if (len < 0) len = len + bigLength + 1;
	if (size_t(len) > bigLength) {
		return new ALUValue(*pBigString->getStrPtr()
				+ std::string(len-bigLength, PAD_CHAR));
	}
	return AluFunc_substring_do(pBigString, 1, len);
}

ALUValue* AluFunc_right(ALUValue* pBigString, ALUValue* pLength)
{
	ASSERT_NOT_ELIDED(pBigString,1,bigString);
	ASSERT_NOT_ELIDED(pLength,2,length);
	auto bigLength = pBigString->getStrPtr()->length();
	ALUInt len = pLength->getInt();
	if (len==0) return new ALUValue("",0);
	if (len < 0) len = len + bigLength + 1;
	if (size_t(len) > bigLength) {
		return new ALUValue(std::string(len-bigLength, PAD_CHAR)
				+ *pBigString->getStrPtr());
	}
	return AluFunc_substring_do(pBigString, bigLength-len+1, len);
}

ALUValue* AluFunc_center(ALUValue* pBigString, ALUValue* pLength)
{
	ASSERT_NOT_ELIDED(pBigString,1,bigString);
	ASSERT_NOT_ELIDED(pLength,2,length);
	auto bigLength = pBigString->getStrPtr()->length();
	ALUInt len = pLength->getInt();
	if (len==0) return new ALUValue("",0);
	if (len < 0) len = len + bigLength + 1;
	if (size_t(len) > bigLength) {
		size_t smallHalf = (len-bigLength) / 2;
		size_t bigHalf = (len-bigLength) - smallHalf;
		return new ALUValue(std::string(smallHalf, PAD_CHAR)
				+ *pBigString->getStrPtr()
				+ std::string(bigHalf, PAD_CHAR));
	}
	return AluFunc_substring_do(pBigString, (bigLength - len) / 2 + 1, len);
}

ALUValue* AluFunc_centre(ALUValue* pBigString, ALUValue* pLength)
{
	return AluFunc_center(pBigString, pLength);
}

ALUValue* AluFunc_pos(ALUValue* _pNeedle, ALUValue* _pHaystack)
{
	ASSERT_NOT_ELIDED(_pNeedle,1,needle);
	ASSERT_NOT_ELIDED(_pHaystack,2,haystack);
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = _pHaystack->getStrPtr();
	size_t pos = pHaystack->find(*pNeedle);
	if (std::string::npos == pos) {
		return new ALUValue(ALUInt(0));
	} else {
		return new ALUValue(ALUInt(pos+1));
	}
}

ALUValue* AluFunc_lastpos(ALUValue* _pNeedle, ALUValue* _pHaystack)
{
	ASSERT_NOT_ELIDED(_pNeedle,1,needle);
	ASSERT_NOT_ELIDED(_pHaystack,2,haystack);
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = _pHaystack->getStrPtr();
	size_t pos = pHaystack->rfind(*pNeedle);
	if (std::string::npos == pos) {
		return new ALUValue(ALUInt(0));
	} else {
		return new ALUValue(ALUInt(pos+1));
	}
}

ALUValue* AluFunc_includes(ALUValue* _pHaystack, ALUValue* _pNeedle)
{
	ASSERT_NOT_ELIDED(_pNeedle,2,needle);
	ASSERT_NOT_ELIDED(_pHaystack,1,haystack);
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = _pHaystack->getStrPtr();
	bool bIsIncluded = (std::string::npos != pHaystack->find(*pNeedle));
	return new ALUValue(ALUInt(bIsIncluded ? 1 : 0));
}

ALUValue* AluFunc_conf(ALUValue* _pKey, ALUValue* _pDefault)
{
	ASSERT_NOT_ELIDED(_pKey,1,key);
	std::string key = _pKey->getStr();
	if (configSpecLiteralExists(key)) {
		return new ALUValue(configSpecLiteralGet(key));
	} else if (_pDefault) {
		return new ALUValue(*_pDefault);
	} else {
		return new ALUValue();
	}
}

extern std::string conv_D2X(std::string& s);
ALUValue* AluFunc_d2x(ALUValue* _pDecValue)
{
	ASSERT_NOT_ELIDED(_pDecValue,1,decValue);
	std::string dec = _pDecValue->getStr();
	return new ALUValue(conv_D2X(dec));
}

extern std::string conv_X2D(std::string& s);
static ALUInt SZLL = ALUInt(2 * sizeof(long long int));
ALUValue* AluFunc_x2d(ALUValue* _pHexValue, ALUValue* pLength)
{
	static std::string zeropad = "0000000000000000";
	static std::string ffffpad = "FFFFFFFFFFFFFFFF";
	ASSERT_NOT_ELIDED(_pHexValue,1,hexValue);
	auto hex = _pHexValue->getStr();
	ALUInt len = ARG_INT_WITH_DEFAULT(pLength, 0);

	if (len < 1) {
		return new ALUValue(conv_X2D(hex));
	}

	if (len > SZLL) len = SZLL;
	while (size_t(len) > hex.length()) len--;

	if (hex.length()==0) return new ALUValue(ALUInt(0));

	MYASSERT(zeropad.length() >= size_t(SZLL));

	auto firstDigit = hex[0];
	if (firstDigit >= '0' && firstDigit <= '7') {
		hex = zeropad.substr(0,(SZLL-len)) + hex.substr(0,len);
	} else {
		hex = ffffpad.substr(0,(SZLL-len)) + hex.substr(0,len);
	}

	long long int value;
	try {
		try {
			unsigned long long int uvalue = std::stoull(hex, NULL, 16);
			value = (long long int) uvalue;
		} catch (std::invalid_argument) {
			CONVERSION_EXCEPTION(hex, "Hex", "Decimal");
		}
	} catch (std::out_of_range) {
		CONVERSION_EXCEPTION_EX(hex, "Hex", "Decimal", "out of range")
	}

	return new ALUValue(ALUInt(value));
}

extern std::string conv_C2X(std::string& s);
ALUValue* AluFunc_c2x(ALUValue* _pCharValue)
{
	ASSERT_NOT_ELIDED(_pCharValue,1,charValue);
	std::string cv = _pCharValue->getStr();
	return new ALUValue(conv_C2X(cv));
}

std::string conv_X2CH(std::string& s);
ALUValue* AluFunc_x2ch(ALUValue* _pHexValue)
{
	ASSERT_NOT_ELIDED(_pHexValue,1,hexValue);
	std::string hex = _pHexValue->getStr();
	return new ALUValue(conv_X2CH(hex));
}

extern std::string conv_UCASE(std::string& s);
ALUValue* AluFunc_ucase(ALUValue* _pString)
{
	ASSERT_NOT_ELIDED(_pString,1,string);
	std::string st = _pString->getStr();
	return new ALUValue(conv_UCASE(st));
}

extern std::string conv_LCASE(std::string& s);
ALUValue* AluFunc_lcase(ALUValue* _pString)
{
	ASSERT_NOT_ELIDED(_pString,1,string);
	std::string st = _pString->getStr();
	return new ALUValue(conv_LCASE(st));
}

extern std::string conv_BSWAP(std::string& s);
ALUValue* AluFunc_bswap(ALUValue* _pString)
{
	ASSERT_NOT_ELIDED(_pString,1,string);
	std::string st = _pString->getStr();
	return new ALUValue(conv_BSWAP(st));
}

ALUValue* AluFunc_break(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	bool bIsBreakEstablished = g_pStateQueryAgent->breakEstablished(fId);
	return new ALUValue(ALUInt(bIsBreakEstablished ? 1 : 0));
}

ALUValue* AluFunc_present(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	bool bIsSet = g_pStateQueryAgent->fieldIdentifierIsSet(fId);
	return new ALUValue(ALUInt(bIsSet ? 1 : 0));
}

ALUValue* AluFunc_sum(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "SUM requested for undefined field identifier")
	return pVStats->sum();
}

ALUValue* AluFunc_min(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "MIN requested for undefined field identifier")
	return pVStats->_min();
}

ALUValue* AluFunc_max(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "MAX requested for undefined field identifier")
	return pVStats->_max();
}

ALUValue* AluFunc_average(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "AVERAGE requested for undefined field identifier")
	return pVStats->average();
}

ALUValue* AluFunc_variance(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "VARIANCE requested for undefined field identifier")
	return pVStats->variance();
}

ALUValue* AluFunc_stddev(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "STDDEV requested for undefined field identifier")
	return pVStats->stddev();
}

ALUValue* AluFunc_stderrmean(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "STDERRMEAN requested for undefined field identifier")
	return pVStats->stderrmean();
}

ALUValue* AluFunc_rand(ALUValue* pLimit)
{
	if (pLimit) {
		ALUInt res = AluRandGetIntUpTo(pLimit->getInt());
		return new ALUValue(res);
	} else {
		static ALUInt decimalLimit = 100000000000000000;
		ALUInt randomDecimal = AluRandGetIntUpTo(decimalLimit);
		std::ostringstream str;
		str << "0." << std::setw(17) << std::setfill('0') << randomDecimal;
		return new ALUValue(str.str());
	}
}

ALUValue* AluFunc_floor(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(floor(pX->getFloat())));
}

ALUValue* AluFunc_round(ALUValue* pX, ALUValue* pDecimals)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	if (pDecimals) {
		if (pDecimals->getInt() < 0) {
			MYTHROW("round: value for 'decimals' must not be negative");
		}
		ALUFloat scale = pow(((ALUFloat)(10.0)), pDecimals->getInt());
		return new ALUValue(ALUFloat((round(scale * pX->getFloat())) / scale));
	} else {
		return new ALUValue(ALUFloat(round(pX->getFloat())));
	}
}

ALUValue* AluFunc_ceil(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(ceil(pX->getFloat())));
}

ALUValue* AluFunc_sin(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(sin(pX->getFloat())));
}

ALUValue* AluFunc_cos(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(cos(pX->getFloat())));
}

ALUValue* AluFunc_tan(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(tan(pX->getFloat())));
}

ALUValue* AluFunc_arcsin(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(asin(pX->getFloat())));
}

ALUValue* AluFunc_arccos(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(acos(pX->getFloat())));
}

ALUValue* AluFunc_arctan(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(atan(pX->getFloat())));
}

static ALUFloat degrees_to_radians = 0.0174532925199433;
static ALUFloat radians_to_degrees = 57.29577951308232;

ALUValue* AluFunc_dsin(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(sin(degrees_to_radians*pX->getFloat())));
}

ALUValue* AluFunc_dcos(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(cos(degrees_to_radians*pX->getFloat())));
}

ALUValue* AluFunc_dtan(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(tan(degrees_to_radians*pX->getFloat())));
}

ALUValue* AluFunc_arcdsin(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(radians_to_degrees*asin(pX->getFloat())));
}

ALUValue* AluFunc_arcdcos(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(radians_to_degrees*acos(pX->getFloat())));
}

ALUValue* AluFunc_arcdtan(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(ALUFloat(radians_to_degrees*atan(pX->getFloat())));
}

/*
 * FREQUENCY MAP - CLASS AND ALU FUNCTIONS
 */

static std::string quotedString(const std::string& s)
{
	std::string ret = "\"";
	for (auto& c : s) {
		switch (c) {
		case '"': ret += '"';  // double the double-quote
		// intentional fall-through
		default:
			ret += c;
		}
	}
	ret += '"';

	return ret;
}

static std::string convertToCsv(const std::string& s)
{
	/* if s contains neither a comma nor a double-quote nor a non-printable, just return s */
	bool bNeedsQuoting = false;
	for (auto& c : s) {
		if (c==',' || c=='"' || (c<' ' && c>0)) {
			bNeedsQuoting = true;
			break;
		}
	}

	if (!bNeedsQuoting) return s;

	/* OK, we need to quote this... */
	return quotedString(s);
}

void frequencyMap::note(std::string& s)
{
	map[s]++;
	counter++;
}

std::string frequencyMap::mostCommon()
{
	ALUInt max = 0;
	std::string ret = "";
	for (freqMapPair& kv : map) {
		if (kv.second > max) {
			ret = kv.first;
			max = kv.second;
		}
	}
	return ret;
}

std::string frequencyMap::leastCommon()
{
	ALUInt min = 0;
	std::string ret = "";
	for (freqMapPair& kv : map) {
		if ((min == 0) || (kv.second < min)) {
			ret = kv.first;
			if (kv.second == 1) { // it doesn't get any rarer that this
				return ret;
			}
			min = kv.second;
		}
	}
	return ret;
}

std::string frequencyMap::dump(fmap_format f, fmap_sortOrder o, bool includePercentage)
{
	typedef std::function<bool(freqMapPair, freqMapPair)> Comparator;

	Comparator compFunctor;
	switch (o) {
	case fmap_sortOrder__byStringAscending:
		compFunctor = [](freqMapPair e1, freqMapPair e2) {
			return e1.first < e2.first;
		};
		break;
	case fmap_sortOrder__byStringDescending:
		compFunctor = [](freqMapPair e1, freqMapPair e2) {
			return e1.first > e2.first;
		};
		break;
	case fmap_sortOrder__byCountAscending:
		compFunctor = [](freqMapPair e1, freqMapPair e2) {
			return (e1.second == e2.second)
					? e1.first < e2.first
					: e1.second < e2.second;
		};
		break;
	case fmap_sortOrder__byCountDescending:
		compFunctor = [](freqMapPair e1, freqMapPair e2) {
			return (e1.second == e2.second)
					? e1.first > e2.first
					: e1.second > e2.second;
		};
		break;
	}

	std::set<freqMapPair, Comparator> setOfFreqs(map.begin(), map.end(), compFunctor);

	unsigned int width = 0;
	ALUInt maxFreq = 0;
	ALUInt sumFreq = 0;
	for (auto& kv : setOfFreqs) {
		if (kv.first.size() > width) {
			width = kv.first.size();
		}
		if (kv.second > maxFreq) {
			maxFreq = kv.second;
		}
		sumFreq += kv.second;
	}

	unsigned int freqWidth = std::to_string(maxFreq).size();

	if ((f > fmap_format__textualJustified) && (f < fmap_format__textualJustifiedLines)) {
		width = (unsigned int)(f);
	}

	std::ostringstream oss;

	// preamble

	if (fmap_format__textualJustifiedLines == f) {
		oss << "+" << std::setw(width+3) << std::setfill('-') << "+"
				<< std::setw(freqWidth+3) << std::setfill('-') << "+";
		if (includePercentage) oss << "--------+";
		oss << '\n';
	} else if (fmap_format__json == f) {
		oss << "\"frequencyMap\": {\n\t\"Entries\": [\n";
	}

	for (auto& kv : setOfFreqs) {
		if (fmap_format__textualJustifiedLines > f) {
			oss << std::setw(width) << std::setfill(' ') << std::left << kv.first;
			oss << std::setw(freqWidth+1) << std::setfill(' ') << std::right << kv.second;
			if (includePercentage) {
				oss << std::fixed << std::setw(7) << std::setprecision(2) << ALUFloat(kv.second) * 100.0 / ALUFloat(sumFreq) << "%";
			}
		}
		else if (fmap_format__textualJustifiedLines == f) {
			oss << "| " << std::setw(width) << std::setfill(' ') << std::left << kv.first;
			oss << " | " << std::setw(freqWidth) << std::setfill(' ') << std::right << kv.second << " |";
			if (includePercentage) {
				oss << std::fixed <<std::setw(6) << std::setprecision(2) <<
						ALUFloat(kv.second) * 100.0 / ALUFloat(sumFreq) << "% |";
			}
		} else if (fmap_format__csv == f) {
			oss << convertToCsv(kv.first) << "," << kv.second;
			if (includePercentage) oss << "." << std::fixed << std::setprecision(6) << ALUFloat(kv.second) / ALUFloat(sumFreq);
		} else if (fmap_format__json == f) {
			oss << "\t\t{ \"Key\":" << quotedString(kv.first) << ", \"Samples\":\"" << kv.second << "\"";
			if (includePercentage) oss << ", \"fraction\":\"" << std::fixed << std::setprecision(6) << ALUFloat(kv.second) / ALUFloat(sumFreq) << "\"";
			oss << " }";
		} else {
			MYASSERT ("Invalid format.");
		}

		oss << '\n';
	}

	// postamble

	if (fmap_format__textualJustifiedLines == f) {
		oss << "+" << std::setw(width+3) << std::setfill('-') << "+"
				<< std::setw(freqWidth+3) << std::setfill('-') << "+";
		if (includePercentage) oss << "--------+";
		oss << '\n';
	} else if (fmap_format__json == f) {
		oss << "\t]\n}\n";
	}

	return oss.str();
}


ALUValue* AluFunc_fmap_nelem(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	return new ALUValue(pfMap->nelem());
}

ALUValue* AluFunc_fmap_nsamples(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	return new ALUValue(pfMap->count());
}

ALUValue* AluFunc_fmap_count(ALUValue* _pFieldIdentifier, ALUValue* pVal)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	ASSERT_NOT_ELIDED(pVal,2,s);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	std::string s = pVal->getStr();
	return new ALUValue((*pfMap)[s]);
}

ALUValue* AluFunc_fmap_frac(ALUValue* _pFieldIdentifier, ALUValue* pVal)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	ASSERT_NOT_ELIDED(pVal,2,s);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	std::string s = pVal->getStr();
	ALUFloat frac = ALUFloat((*pfMap)[s]) / ALUFloat(pfMap->count());
	return new ALUValue(frac);
}

ALUValue* AluFunc_fmap_pct(ALUValue* _pFieldIdentifier, ALUValue* pVal)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	ASSERT_NOT_ELIDED(pVal,2,s);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	std::string s = pVal->getStr();
	ALUFloat frac = ALUFloat((*pfMap)[s]) / ALUFloat(pfMap->count());
	return new ALUValue(PERCENTS * frac);
}

ALUValue* AluFunc_fmap_common(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	return new ALUValue(pfMap->mostCommon());
}

ALUValue* AluFunc_fmap_rare(ALUValue* _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	return new ALUValue(pfMap->leastCommon());
}

ALUValue* AluFunc_fmap_sample(ALUValue* _pFieldIdentifier, ALUValue* pVal)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	ASSERT_NOT_ELIDED(pVal,2,s);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	std::string s = pVal->getStr();
	pfMap->note(s);
	return new ALUValue((*pfMap)[s]);
}

ALUValue* AluFunc_fmap_dump(ALUValue* _pFieldIdentifier, ALUValue* pFormat, ALUValue* pOrder, ALUValue* pPct)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	std::string s;
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);

	fmap_format f;
	s = ARG_STR_WITH_DEFAULT(pFormat, "txt");
	if (s=="" || s=="txt" || s=="0") f = fmap_format__textualJustified;
	else if (s=="lin") f = fmap_format__textualJustifiedLines;
	else if (s=="csv") f = fmap_format__csv;
	else if (s=="json") f = fmap_format__json;
	else if (counterType__Int == pFormat->getDivinedType()) {
		f = (fmap_format(pFormat->getInt()));
	}
	else {
		std::string err = "Invalid frequency map dump format: " + s;
		MYTHROW(err);
	}

	fmap_sortOrder o;
	s = ARG_STR_WITH_DEFAULT(pOrder,"sa");
	if (s=="" || s=="s" || s=="sa") o = fmap_sortOrder__byStringAscending;
	else if (s=="sd") o = fmap_sortOrder__byStringDescending;
	else if (s=="c" || s=="ca") o = fmap_sortOrder__byCountAscending;
	else if (s=="cd") o = fmap_sortOrder__byCountDescending;
	else {
		std::string err = "Invalid frequency map sort order: " + s;
		MYTHROW(err);
	}

	bool includePercentage = pPct ? pPct->getBool() : false;

	return new ALUValue(pfMap->dump(f, o, includePercentage));
}


ALUValue* AluFunc_string(ALUValue* pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return new ALUValue(pX->getStr());
}

ALUValue* AluFunc_substitute(ALUValue* pSrc, ALUValue* pSearchString, ALUValue* pSubstitute, ALUValue* pMax)
{
	ASSERT_NOT_ELIDED(pSrc,1,haystack);
	ASSERT_NOT_ELIDED(pSearchString,2,needle);
	ASSERT_NOT_ELIDED(pSubstitute,3,subst);
	std::string res = pSrc->getStr();
	ALUInt count = ARG_INT_WITH_DEFAULT(pMax,1);
	if (pMax->getStr()=="U") count = MAX_ALUInt;

	size_t findRet = 0;

	if (0 == pSearchString->getStr().length()) {
		MYTHROW("substitute: Search string must not be empty");
	}

	while (count > 0) {
		findRet = res.find(pSearchString->getStr(), findRet);
		if (findRet == std::string::npos) break;

		res = res.substr(0,findRet) + pSubstitute->getStr() +
				res.substr(findRet + pSearchString->getStr().length());

		count--;
	}

	return new ALUValue(res);
}

ALUValue* AluFunc_sfield(ALUValue* pStr, ALUValue* pCount, ALUValue* pSep)
{
	ASSERT_NOT_ELIDED(pStr,1,str);
	ASSERT_NOT_ELIDED(pCount,2,count);
	std::string str = pStr->getStr();
	ALUInt count = pCount->getInt();
	char sep;
	if (pSep && pSep->getStrPtr()->length() > 0) {
		sep = pSep->getStr()[0];
	} else {
		sep = '\t';
	}

	if (0 == count) {
		MYTHROW("sfield: Called with count equal to zero");
	}

	// The following ensures that the string is not zero-length for the rest of the code
	// which does make that assumption
	if (0 == str.length()) {
		return new ALUValue("");
	}


	if (count > 0) {
		char* pc = (char*)(str.c_str());
		while ((count > 1) && (*pc != '\0')) {
			while ((*pc!='\0') && (*pc!=sep)) pc++;
			if (*pc==sep) {
				count--;
				pc++;
			}
		}
		if (count > 1 || *pc=='\0') {
			return new ALUValue("");
		} else {
			char *pEnd = pc;
			while ((*pEnd!='\0') && (*pEnd!=sep)) pEnd++;
			return new ALUValue(pc, (pEnd-pc));
		}
	}
	else {
		char *pStart = (char*)(str.c_str());
		char* pc = pStart + str.length();
		pc--; // that's where the non-zero-length assumption comes in
		while ((count < -1) && (pc > pStart)) {
			while ((pc>pStart) && (*pc!=sep)) pc--;
			if (*pc==sep) {
				count++;
				pc--;
			}
		}
		if (count < -1 || pc==pStart) {
			return new ALUValue("");
		} else {
			char *pBegin = pc;
			while ((pBegin>pStart) && (*pBegin!=sep)) pBegin--;
			return new ALUValue(pBegin+1, (pc-pBegin));
		}
	}
}

ALUValue* AluFunc_sword(ALUValue* pStr, ALUValue* pCount, ALUValue* pSep)
{
	ASSERT_NOT_ELIDED(pStr,1,str);
	ASSERT_NOT_ELIDED(pCount,2,count);
	std::string str = pStr->getStr();
	ALUInt count = pCount->getInt();
	char sep;
	if (pSep && pSep->getStrPtr()->length() > 0) {
		sep = pSep->getStr()[0];
	} else {
		sep = ' ';
	}

	if (0 == count) {
		MYTHROW("sword: Called with count equal to zero");
	}

	// The following ensures that the string is not zero-length for the rest of the code
	// which does make that assumption
	if (0 == str.length()) {
		return new ALUValue("");
	}


	if (count > 0) {
		char* pc = (char*)(str.c_str());
		while (sep==*pc) pc++;  // The first word may start after 1 or more seps. Not so in sfield
		while ((count > 1) && (*pc != '\0')) {
			while ((*pc!='\0') && (*pc!=sep)) pc++;
			if (sep==*pc) {
				count--;
				while (sep==*pc) pc++;
			}
		}
		if (count > 1 || *pc=='\0') {
			return new ALUValue("");
		} else {
			char *pEnd = pc;
			while ((*pEnd!='\0') && (*pEnd!=sep)) pEnd++;
			return new ALUValue(pc, (pEnd-pc));
		}
	}
	else {
		char *pStart = (char*)(str.c_str());
		char* pc = pStart + str.length();
		pc--; // that's where the non-zero-length assumption comes in
		while (sep==*pc) pc--;  // The last word may be followed by word separators
		while ((count < -1) && (pc > pStart)) {
			while ((pc>pStart) && (*pc!=sep)) pc--;
			if (sep==*pc) {
				count++;
				while (sep==*pc) pc--;
			}
		}
		if (count < -1 || pc==pStart) {
			return new ALUValue("");
		} else {
			char *pBegin = pc;
			while ((pBegin>pStart) && (*pBegin!=sep)) pBegin--;
			return new ALUValue(pBegin+1, (pc-pBegin));
		}
	}
}

/* REXX Functions */

ALUValue* AluFunc_abbrev_do(ALUValue* pInformation, ALUValue* pInfo, size_t len)
{
	std::string sBig = pInformation->getStr();
	std::string sLittle = pInfo->getStr().substr(0,len);

	if (sBig.length() >= sLittle.length()) {
		if (sLittle == sBig.substr(0,sLittle.length())) {
			return new ALUValue(ALUInt(1));
		}
	}

	return new ALUValue(ALUInt(0));
}

ALUValue* AluFunc_abbrev(ALUValue* pInformation, ALUValue* pInfo, ALUValue* pLen)
{
	ASSERT_NOT_ELIDED(pInformation,1,information);
	ASSERT_NOT_ELIDED(pInfo,2,info);
	ALUInt len = ARG_INT_WITH_DEFAULT(pLen, uint32_t(std::string::npos));
	if (len <= 0) {
		std::string err = "abbrev: Got non-positive length: " + std::to_string(len);
		MYTHROW(err);
	}

	return AluFunc_abbrev_do(pInformation, pInfo, size_t(len));
}

ALUValue* AluFunc_bitand(ALUValue* pS1, ALUValue* pS2)
{
	ASSERT_NOT_ELIDED(pS1,1,s1);
	ASSERT_NOT_ELIDED(pS2,2,s2);
	std::string s1 = pS1->getStr();
	std::string s2 = pS2->getStr();

	size_t minlen = (s1.length() > s2.length()) ? s2.length() : s1.length();

	unsigned char *pBuff = new unsigned char[minlen];

	const unsigned char *pc1 = (const unsigned char*)(s1.c_str());
	const unsigned char *pc2 = (const unsigned char*)(s2.c_str());

	for (size_t i = 0 ; i < minlen ; i++) {
		pBuff[i] = pc1[i] & pc2[i];
	}

	ALUValue* pRet = new ALUValue((const char*)(pBuff), minlen);

	delete [] pBuff;

	return pRet;
}

ALUValue* AluFunc_bitor(ALUValue* pS1, ALUValue* pS2)
{
	ASSERT_NOT_ELIDED(pS1,1,s1);
	ASSERT_NOT_ELIDED(pS2,2,s2);
	std::string s1 = pS1->getStr();
	std::string s2 = pS2->getStr();

	size_t minlen = (s1.length() > s2.length()) ? s2.length() : s1.length();

	unsigned char *pBuff = new unsigned char[minlen];

	const unsigned char *pc1 = (const unsigned char*)(s1.c_str());
	const unsigned char *pc2 = (const unsigned char*)(s2.c_str());

	for (size_t i = 0 ; i < minlen ; i++) {
		pBuff[i] = pc1[i] | pc2[i];
	}

	ALUValue* pRet = new ALUValue((const char*)(pBuff), minlen);

	delete [] pBuff;

	return pRet;
}

ALUValue* AluFunc_bitxor(ALUValue* pS1, ALUValue* pS2)
{
	ASSERT_NOT_ELIDED(pS1,1,s1);
	ASSERT_NOT_ELIDED(pS2,2,s2);
	std::string s1 = pS1->getStr();
	std::string s2 = pS2->getStr();

	size_t minlen = (s1.length() > s2.length()) ? s2.length() : s1.length();

	unsigned char *pBuff = new unsigned char[minlen];

	const unsigned char *pc1 = (const unsigned char*)(s1.c_str());
	const unsigned char *pc2 = (const unsigned char*)(s2.c_str());

	for (size_t i = 0 ; i < minlen ; i++) {
		pBuff[i] = pc1[i] ^ pc2[i];
	}

	ALUValue* pRet = new ALUValue((const char*)(pBuff), minlen);

	delete [] pBuff;

	return pRet;
}

ALUValue* AluFunc_compare_do(ALUValue* pS1, ALUValue* pS2, char pad)
{
	auto s1 = pS1->getStr();
	auto s2 = pS2->getStr();
	if (s1 == s2) {
		return new ALUValue(ALUInt(0));
	}
	auto s1Len = s1.length();
	auto s2Len = s2.length();

	auto maxLen = std::max(s1Len, s2Len);

	for (size_t l = 0 ; l < maxLen ; l++) {
		char c1 = (l < s1Len) ? s1[l] : pad;
		char c2 = (l < s2Len) ? s2[l] : pad;

		if (c1 != c2) {
			return new ALUValue(ALUInt(l)+1);
		}
	}

	if (s1Len==s2Len) {
		std::string err = "compare/comparep: different character was not found; s1=<"
				+ s1 + ">; s2=>" + s2 + ">";
		MYTHROW(err);
	}

	// The padding made them equal
	return new ALUValue(ALUInt(0));
}

ALUValue* AluFunc_compare(ALUValue* pS1, ALUValue* pS2, ALUValue* pPad)
{
	ASSERT_NOT_ELIDED(pS1,1,s1);
	ASSERT_NOT_ELIDED(pS2,2,s2);
	std::string sPad = ARG_STR_WITH_DEFAULT(pPad, " ");
	if (sPad.length() != 1) {
		std::string err = "compare: Pad argument should be 1 char. Got <" + sPad + ">";
		MYTHROW(err);
	}
	return AluFunc_compare_do(pS1, pS2, sPad[0]);
}

ALUValue* AluFunc_copies(ALUValue* pString, ALUValue* pTimes)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pTimes,2,times);
	auto theString = pString->getStr();
	auto theCount = pTimes->getInt();

	if (theCount < 0) {
		std::string err = "copies: Second argument should be a non-negative integer. Got: "
				+ std::to_string(theCount);
		MYTHROW(err);
	}

	std::string res = "";
	for (int i=0 ; i < theCount ; i++) {
		res += theString;
	}

	return new ALUValue(res);
}

ALUValue* AluFunc_delstr(ALUValue* pString, ALUValue* pStart, ALUValue* pLength)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pStart,2,start);
	auto theString = pString->getStr();
	auto start = pStart->getInt();

	if (start < 1) start = 1;

	// If start is after end of the string, we return the whole string
	if (size_t(start) > theString.length()) return new ALUValue(theString);

	std::string res = theString.substr(0,start-1);

	auto length = ARG_INT_WITH_DEFAULT(pLength,0);
	// zero is a special value meaning delete to the end. If the length is greater
	// than the remainder, also delete to the end.
	if (0>=length || size_t(length+start) > theString.length()) return new ALUValue(res);

	res += theString.substr(start+length-1);
	return new ALUValue(res);
}

ALUValue* AluFunc_delword(ALUValue* pString, ALUValue* pStart, ALUValue* pLength)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pStart,2,start);
	ASSERT_NOT_ELIDED(pLength,3,length);
	auto theString = pString->getStr();

	auto start = pStart->getInt();
	if (start < 1) start = 1;

	auto length = pLength->getInt();
	if (0>=length) length = MAX_ALUInt - start;

	if (0 == theString.length()) return new ALUValue(theString);

	bool inWhitespace = isspace(theString[0]);
	unsigned int  wordIndex = (inWhitespace) ? 0 : 1; // using 1-based word index
	std::string res = "";
	for (size_t i=0; i<theString.length(); i++) {
		// check if we've moved from non-ws to ws or vice versa
		if (inWhitespace) {
			if (!isspace(theString[i])) {
				inWhitespace = false;
				wordIndex++;
			}
		} else {
			if (isspace(theString[i])) {
				inWhitespace = true;
			}
		}

		if ((wordIndex < (start-1)) ||
			((wordIndex == (start-1)) && !inWhitespace) || // remove the whitespace before the first word to be deleted
			(wordIndex > (start+length-1)) ||
			((wordIndex == (start+length-1)) && inWhitespace && (start>1))) {
			res += theString[i];
		}
	}

	return new ALUValue(res);
}

/*
 * Returns a vector of the words of s
 */
static std::vector<std::string> breakIntoWords(std::string s)
{
	std::vector<std::string> ret;

	if (0 == s.length()) return ret;

	bool inWhitespace = isspace(s[0]);
	unsigned int wordIndex = (inWhitespace) ? 0 : 1; // using 1-based word index
	int wordStart = (inWhitespace) ? -1 : 0;
	for (size_t i=0; i<s.length(); i++) {
		// check if we've moved from non-ws to ws or vice versa
		if (inWhitespace) {
			if (!isspace(s[i])) {
				inWhitespace = false;
				wordIndex++;
				wordStart = i;
			}
		} else {
			if (isspace(s[i])) {
				inWhitespace = true;
				ret.push_back(s.substr(wordStart, i-wordStart));
			} else if (i == s.length()-1) {
				ret.push_back(s.substr(wordStart));
			}
		}

	}

	return ret;
}

static std::vector<size_t> breakIntoWords_start(std::string s)
{
	std::vector<size_t> ret;

	if (0 == s.length()) return ret;

	bool inWhitespace = isspace(s[0]);
	if (!inWhitespace) ret.push_back(0);
	unsigned int wordIndex = (inWhitespace) ? 0 : 1; // using 1-based word index
	for (size_t i=0; i<s.length(); i++) {
		// check if we've moved from non-ws to ws or vice versa
		if (inWhitespace) {
			if (!isspace(s[i])) {
				inWhitespace = false;
				wordIndex++;
				ret.push_back(i);
			}
		} else {
			if (isspace(s[i])) {
				inWhitespace = true;
			}
		}

	}

	return ret;
}

static std::vector<size_t> breakIntoWords_end(std::string s)
{
	std::vector<size_t> ret;

	if (0 == s.length()) return ret;

	bool inWhitespace = isspace(s[0]);
	unsigned int wordIndex = (inWhitespace) ? 0 : 1; // using 1-based word index
	for (size_t i=0; i<s.length(); i++) {
		// check if we've moved from non-ws to ws or vice versa
		if (inWhitespace) {
			if (!isspace(s[i])) {
				inWhitespace = false;
				wordIndex++;
			}
		} else {
			if (isspace(s[i])) {
				inWhitespace = true;
				ret.push_back(i-1);
			} else if (i == s.length()-1) {
				ret.push_back(i);
			}
		}

	}
	return ret;
}

ALUValue* AluFunc_find(ALUValue* string, ALUValue* phrase)
{
	ASSERT_NOT_ELIDED(string,1,string);
	ASSERT_NOT_ELIDED(phrase,2,phrase);
	auto phraseWords = breakIntoWords(phrase->getStr());
	auto stringWords = breakIntoWords(string->getStr());

	int phraseWordCount = phraseWords.size();
	int stringWordCount = stringWords.size();

	for (int i=0 ; i < stringWordCount - phraseWordCount + 1 ; i++)
	{
 		bool foundMismatch = false;
		for (int j=0; !foundMismatch && j < phraseWordCount; j++)
		{
			if (phraseWords[j] != stringWords[i+j]) foundMismatch = true;
		}

		if (!foundMismatch) return new ALUValue(ALUInt(i+1));
	}

	return new ALUValue(ALUInt(0));
}

ALUValue* AluFunc_index(ALUValue* _pHaystack, ALUValue* _pNeedle, ALUValue* _pStart)
{
	ASSERT_NOT_ELIDED(_pHaystack,1,haystack);
	ASSERT_NOT_ELIDED(_pNeedle,2,needle);
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = _pHaystack->getStrPtr();
	ALUInt start = ARG_INT_WITH_DEFAULT(_pStart, 1);

	if (start < 1) start = 1;

	size_t pos = pHaystack->find(*pNeedle, (start-1));
	if (std::string::npos == pos) {
		return new ALUValue(ALUInt(0));
	} else {
		return new ALUValue(ALUInt(pos+1));
	}
}

static ALUValue* insert_do(std::string& str, std::string& tgt, size_t pos, size_t len, char pad)
{
	std::string paddedStr;

	if (0==len) {
		paddedStr = str;
	} else {
		if (len <= str.length()) {
			paddedStr = str.substr(0,len);
		} else {
			paddedStr = str;
			while (paddedStr.length() < len) paddedStr += pad;
		}
	}

	std::string ret;
	if (pos > 0) ret = tgt.substr(0,pos);

	ret += paddedStr;

	if (pos < tgt.length()) ret += tgt.substr(pos);

	return new ALUValue(ret);
}

ALUValue* AluFunc_insert(ALUValue* pString, ALUValue* pTarget, ALUValue* pPosition, ALUValue* pLength, ALUValue* pPad)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pTarget,2,target);
	auto theString = pString->getStr();
	auto theTarget = pTarget->getStr();
	auto position = ARG_INT_WITH_DEFAULT(pPosition,0);
	if (0 > position) {
		std::string err = "insert: Invalid negative position value: " + std::to_string(position);
		MYTHROW(err);
	}
	auto length = ARG_INT_WITH_DEFAULT(pLength,0);
	if (0 > length) {
		std::string err = "insert: Invalid negative length value: " + std::to_string(length);
		MYTHROW(err);
	}
	std::string sPad = ARG_STR_WITH_DEFAULT(pPad, " ");
	if (1 != sPad.length()) {
		std::string err = "insert: Invalid pad argument: <" + sPad + ">";
		MYTHROW(err);
	}

	return insert_do(theString, theTarget, position, length, sPad[0]);
}

static ALUValue* justify_do(std::string& str, size_t len, char pad)
{
	auto wordVector = breakIntoWords(str);
	int numOfSpaces = int(len);
	for (const auto & word : wordVector) {
		numOfSpaces -= int(word.length());
	}

	if (numOfSpaces < 0) numOfSpaces = 0;

	std::string ret = "";

	for (size_t i=0 ; i < wordVector.size() ; i++) {
		ret += wordVector[i];
		auto countGaps = wordVector.size() - i - 1;
		if (0 < countGaps) {
			auto spacesThisGap = numOfSpaces / countGaps;
			for (size_t j=0 ; j < spacesThisGap ; j++) ret += pad;
			numOfSpaces -= spacesThisGap;
		}
	}

	if (ret.size() > len) ret = ret.substr(0,len);
	MYASSERT(ret.size() == len);

	return new ALUValue(ret);
}

ALUValue* AluFunc_justify(ALUValue* pStr, ALUValue* pLen, ALUValue* pPad)
{
	ASSERT_NOT_ELIDED(pStr,1,string);
	ASSERT_NOT_ELIDED(pLen,2,length);
	auto str = pStr->getStr();
	auto len = pLen->getInt();
	if (len < 0) {
		std::string err = "justify: len argument should be non-negative. Got " + std::to_string(len);
		MYTHROW(err);
	}
	std::string sPad = ARG_STR_WITH_DEFAULT(pPad, " ");
	if (1 != sPad.length()) {
		std::string err = "justify: Invalid pad argument: <" + sPad + ">";
		MYTHROW(err);
	}

	return justify_do(str,size_t(len), sPad[0]);
}

ALUValue* AluFunc_overlay(ALUValue* pString1, ALUValue* pString2, ALUValue* pStart, ALUValue* pLength, ALUValue* pPad)
{
	ASSERT_NOT_ELIDED(pString1,1,string1);
	ASSERT_NOT_ELIDED(pString2,2,string1);
	auto str1 = pString1->getStr();
	auto str2 = pString2->getStr();
	ALUInt start = ARG_INT_WITH_DEFAULT(pStart, 1);
	ALUInt length = ARG_INT_WITH_DEFAULT(pLength, 0);
	std::string padStr = ARG_STR_WITH_DEFAULT(pPad, " ");

	// sanity checks
	if (0 == str1.length()) return new ALUValue(str2);
	if (start < 1) {
		std::string err = "overlay: start argument should be positive. Got: " + std::to_string(start);
		MYTHROW(err);
	}
	if (length < 0) {
		std::string err = "overlay: length argument should be positive. Got: " + std::to_string(length);
		MYTHROW(err);
	}
	if (length>0 && padStr.length()!=1) {
		std::string err = "overlay: Invalid pad argument: <" + padStr + ">";
		MYTHROW(err);
	}

	char pad = padStr[0];

	// str2 shall serve as return value
	// first pad if needed to reach start
	while (str2.length() < (size_t(start)-1)) {
		str2 += pad;
	}

	// now prepare the overwrite string -- re-use str1
	if (str1.length() > size_t(length) && length>0) {
		str1 = str1.substr(0,length);
	}
	while (str1.length() < size_t(length)) {
		str1 += pad;
	}

	std::string ret = str2.substr(0,start-1);
	ret += str1;
	if ((str1.length() + start - 1) < str2.length()) {
		ret += str2.substr(str1.length() + start - 1);
	}

	return new ALUValue(ret);
}

ALUValue* AluFunc_reverse(ALUValue* pStr)
{
	ASSERT_NOT_ELIDED(pStr,1,str);
	auto str = pStr->getStr();
	std::reverse(str.begin(), str.end());
	return new ALUValue(str);
}

ALUValue* AluFunc_sign(ALUValue* pNumber)
{
	ASSERT_NOT_ELIDED(pNumber,1,number);
	auto num = pNumber->getFloat();
	ALUInt ret = 0;
	if (num > 0) {
		ret = 1;
	} else if (num < 0) {
		ret = -1;
	}

	return new ALUValue(ret);
}

ALUValue* AluFunc_space(ALUValue* pStr, ALUValue* pLength, ALUValue* pPad)
{
	ASSERT_NOT_ELIDED(pStr,1,string);
	auto wordVector = breakIntoWords(pStr->getStr());
	std::string sPad = ARG_STR_WITH_DEFAULT(pPad, " ");
	if (sPad.length() != 1) {
		std::string err = "space: Pad argument should be 1 char. Got <" + sPad + ">";
		MYTHROW(err);
	}
	char pad = sPad[0];
	ALUInt len = ARG_INT_WITH_DEFAULT(pLength, 1);
	if (len < 0 || len > 1000) {
		std::string err = "space: Invalid length argument: <" + pLength->getStr() + ">";
		MYTHROW(err);
	}

	std::string ret;
	for (size_t i = 0; i < wordVector.size(); i++) {
		if (i > 0) {
			for (ALUInt j = 0 ; j < len ; j++) ret += pad;
		}
		ret += wordVector[i];
	}

	return new ALUValue(ret);
}

ALUValue* AluFunc_strip(ALUValue* pString, ALUValue* pOption, ALUValue* pPad)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	auto str = pString->getStr();

	std::string sOpt = ARG_STR_WITH_DEFAULT(pOption, "B");
	if (sOpt.length() != 1) {
		std::string err = "strip: Option argument should be 1 char. Got <" + sOpt + ">";
		MYTHROW(err);
	}
	char option = toupper(sOpt[0]);

	std::string sPad = ARG_STR_WITH_DEFAULT(pPad, " ");
	if (sPad.length() != 1) {
		std::string err = "strip: Pad argument should be 1 char. Got <" + sPad + ">";
		MYTHROW(err);
	}
	char pad = sPad[0];

	std::string ret;
	auto first = str.find_first_not_of(pad);
	auto last = str.find_last_not_of(pad);
	switch (option) {
	case 'B':
		ret = str.substr(first, last-first+1);
		break;
	case 'L':
		ret = str.substr(first);
		break;
	case 'T':
		ret = str.substr(0,last+1);
		break;
	default:
		std::string err = "strip: Invalid option argument: <"+pOption->getStr()+">";
		MYTHROW(err);
	}

	return new ALUValue(ret);
}

ALUValue* AluFunc_subword(ALUValue* pString, ALUValue* pStart, ALUValue* pLength)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pStart,2,start);
	auto str = pString->getStr();
	auto start = pStart->getInt();
	ALUInt len = ARG_INT_WITH_DEFAULT(pLength, 0);

	if (len < 0) {
		std::string err = "subword: length argument must be non-negative. Got: " + std::to_string(len);
		MYTHROW(err);
	}

	if (start < 0) {
		std::string err = "subword: start argument must be non-negative. Got: " + std::to_string(start);
		MYTHROW(err);
	}

	size_t cstart, cend;

	if (start==0) {
		cstart = 0;
		start = 1;
	}
	else {
		auto startVec = breakIntoWords_start(str);
		if (size_t(start) > startVec.size()) {
			return new ALUValue("");
		}
		cstart = startVec[start-1];
	}

	if (0 == len) {
		return new ALUValue(str.substr(cstart));
	} else {
		auto endVec = breakIntoWords_end(str);
		if (size_t(start + len - 1) > endVec.size()) {
			return new ALUValue(str.substr(cstart));
		}
		cend = endVec[start+len-2];
		return new ALUValue(str.substr(cstart, cend-cstart+1));
	}
}

ALUValue* AluFunc_translate(ALUValue* pString, ALUValue* pTableOut, ALUValue* pTableIn, ALUValue* pPad)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	auto str = pString->getStr();

	if ((pTableIn && !pTableOut) || (!pTableIn && pTableOut)) {
		static const std::string err = "Either both tableout and tablein should be specified, or neither.";
		THROW_ARG_ISSUE(0,"",err);
	}

	std::string tableout = pTableOut ? pTableOut->getStr() : "";
	std::string tablein = pTableIn ? pTableIn->getStr() : "";

	if (0 == tableout.length() && 0 == tablein.length()) {
		// just uppercase the whole thing
		for (auto & c : str) c = toupper(c);
	} else {
		std::string sPad = ARG_STR_WITH_DEFAULT(pPad, " ");
		if (sPad.length() != 1) {
			std::string err = "translate: Pad argument should be 1 char. Got <" + sPad + ">";
			MYTHROW(err);
		}
		char pad = sPad[0];

		for (size_t i=0; i<tablein.length(); i++) {
			char oldValue = tablein[i];
			char newValue = (tableout.length() <= i) ? pad : tableout[i];
			std::replace(str.begin(), str.end(), oldValue, newValue);
		}
	}

	return new ALUValue(str);
}

ALUValue* AluFunc_verify(ALUValue* pString, ALUValue* pReference, ALUValue* pOption, ALUValue* pStart)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pReference,1,reference);
	auto str = pString->getStr();
	auto reference = pReference->getStr();

	std::string sOpt = ARG_STR_WITH_DEFAULT(pOption, "N");
	if (sOpt.length() != 1) {
		std::string err = "verify: Option argument should be 'M' or 'N'. Got <" + sOpt + ">";
		MYTHROW(err);
	}
	char option = toupper(sOpt[0]);
	if ('N' != option && 'M' != option) {
		std::string err = "verify: bad option argument: <" + sOpt + ">";
		MYTHROW(err);
	}

	size_t start = ARG_INT_WITH_DEFAULT(pStart,1);
	if (start < 1) {
		std::string err = "verify: bad start argument: <" + pStart->getStr() + ">";
		MYTHROW(err);
	}

	ALUInt ret = 0;
	bool bFound = false;
	for (size_t i=start-1 ; !bFound && i < str.length() ; i++) {
		size_t cntMatch=0;
		for (size_t j=0 ; !cntMatch && j < reference.length() ; j++) {
			if (str[i] == reference[j]) ++cntMatch;
		}

		if ((option=='M' && cntMatch>0) || (option=='N' && cntMatch==0)) {
			bFound = true;
			ret = i + 1;
		}
	}

	return new ALUValue(ret);
}

ALUValue* AluFunc_wordindex(ALUValue* pString, ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pIdx,2,wordno);
	auto str = pString->getStr();
	ALUInt idx = pIdx->getInt();

	if (idx < 1) {
		std::string err = "wordindex: wordno argument must be positive. Got: " + std::to_string(idx);
		MYTHROW(err);
	}

	auto wordvec = breakIntoWords_start(str);

	ALUInt ret = 0;
	if (size_t(idx-1) < wordvec.size()) {
		ret = wordvec[idx-1] + 1;
	}

	return new ALUValue(ret);
}

ALUValue* AluFunc_wordlength(ALUValue* pString, ALUValue* pIdx)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pIdx,2,wordno);
	auto str = pString->getStr();
	ALUInt idx = pIdx->getInt();

	if (idx < 1) {
		std::string err = "wordlength: wordno argument must be positive. Got: " + std::to_string(idx);
		MYTHROW(err);
	}

	auto wordvec = breakIntoWords(str);

	ALUInt ret = 0;
	if (size_t(idx-1) < wordvec.size()) {
		ret = wordvec[idx-1].length();
	}

	return new ALUValue(ret);
}

ALUValue* AluFunc_wordpos(ALUValue* pPhrase, ALUValue* pString, ALUValue* pStart)
{
	ASSERT_NOT_ELIDED(pPhrase,1,phrase);
	ASSERT_NOT_ELIDED(pString,2,string);
	auto phrase = pPhrase->getStr();
	auto str = pString->getStr();
	ALUInt start = ARG_INT_WITH_DEFAULT(pStart,1);

	if (start < 1) {
		std::string err = "Argument must be positive, but got " + std::to_string(start);
		THROW_ARG_ISSUE(3,start,err);
	}

	auto startVec = breakIntoWords_start(str);

	for (size_t i = start-1 ; i < startVec.size() ; i++) {
		auto sub = str.substr(startVec[i], phrase.length());
		if (sub==phrase) return new ALUValue(ALUInt(i+1));
	}

	return new ALUValue(ALUInt(0));
}

ALUValue* AluFunc_words(ALUValue* pStr)
{
	ASSERT_NOT_ELIDED(pStr,1,string);
	auto str = pStr->getStr();
	auto startVec = breakIntoWords(str);
	ALUInt ret = startVec.size();
	return new ALUValue(ret);
}

ALUValue* AluFunc_xrange(ALUValue* pStart, ALUValue* pEnd)
{
	std::string startStr = ARG_STR_WITH_DEFAULT(pStart,"");
	std::string endStr = ARG_STR_WITH_DEFAULT(pEnd,"");

	int start = (startStr.length() > 0) ? startStr[0] : 0;
	int end = (endStr.length() > 0) ? endStr[0] : 0xff;

	while (start < 0) start += 256;
	while (end < 0) end += 256;

	if (end < start) return new ALUValue(std::string(""));

	std::string ret;
	for (int i = start ; i <= end ; i++) ret += char(i);

	return new ALUValue(ret);
}

class my_punct : public std::numpunct<char>
{
public:
	my_punct() : m_sep('\0'),m_dec('.') {}
	void set_sep(char c)	{m_sep = c;}
	void set_dec(char c)	{m_dec = c;}
protected:
	virtual char do_thousands_sep() const
	{
		return m_sep;
	}
	virtual char do_decimal_point() const
	{
		return m_dec;
	}
	virtual std::string do_grouping() const
	{
		return m_sep ? "\03" : "";
	}
private:
	char m_sep;
	char m_dec;
};


ALUValue* AluFunc_fmt(ALUValue* pVal, ALUValue* pFormat, ALUValue* pDigits, ALUValue* pDecimal, ALUValue* pSep)
{
	std::ostringstream oss;
	ASSERT_NOT_ELIDED(pVal,1,value);

	if (pDigits) {
		oss.precision(pDigits->getInt());
	}

	if (pFormat) {
		std::string format = pFormat->getStr();
		switch (format[0]) {
		case 'f':
		case 'F':
			oss.setf( std::ios::fixed, std:: ios::floatfield );
			break;
		case 's':
		case 'S':
		case 'e':
		case 'E':
			oss.setf( std::ios::scientific, std:: ios::floatfield );
			break;
		default: {
			std::string err = "fmt: #2 (format): Invalid format value: <" + format + ">";
			MYTHROW(err);
		}
		}
	}

	if (pDecimal || pSep) {
		static std::locale myStaticLocale;
		my_punct *pct = new my_punct;
		if (pDecimal) pct->set_dec(pDecimal->getStr().c_str()[0]);
		if (pSep) pct->set_sep(pSep->getStr().c_str()[0]);

		oss.imbue(std::locale(oss.getloc(), pct));
		oss << pVal->getFloat();
		return new ALUValue(oss.str());
	}

	oss << pVal->getFloat();

	return new ALUValue(oss.str());
}

ALUValue* AluFunc_next()
{
	if (!g_PositionGetter) return new ALUValue(ALUInt(1));
	return new ALUValue(ALUInt(g_PositionGetter->pos()));
}

ALUValue* AluFunc_rest()
{
	static std::string sName("cols");
	static std::string sCols = configSpecLiteralGet(sName);
	static ALUInt cols = std::stoul(sCols);
	return new ALUValue(ALUInt(cols - g_PositionGetter->pos() + 1));
}

ALUValue* AluFunc_defined(ALUValue* pName)
{
	ASSERT_NOT_ELIDED(pName,1,confString);
	auto name = pName->getStr();
	if (configSpecLiteralExists(name)) {
		return new ALUValue(ALUInt(1));
	} else {
		return new ALUValue(ALUInt(0));
	}
}


#ifdef DEBUG
ALUValue* AluFunc_testfunc(ALUValue* pArg1, ALUValue* pArg2, ALUValue* pArg3, ALUValue* pArg4)
{
	std::string str1 = pArg1 ? pArg1->getStr() : "(nil)";
	std::string str2 = pArg2 ? pArg2->getStr() : "(nil)";
	std::string str3 = pArg3 ? pArg3->getStr() : "(nil)";
	std::string str4 = pArg4 ? pArg4->getStr() : "(nil)";

	std::string res = str1 + "," + str2 + "," + str3 + "," + str4;

	return new ALUValue(res);
}
#endif
