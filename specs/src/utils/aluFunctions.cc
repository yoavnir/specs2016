#include "utils/ErrorReporting.h"
#include "utils/aluFunctions.h"
#include "utils/TimeUtils.h"
#include "processing/Config.h"
#include <string.h>
#include <cmath>

#define PAD_CHAR ' '

stateQueryAgent* g_pStateQueryAgent = NULL;

void setStateQueryAgent(stateQueryAgent* qa)
{
	g_pStateQueryAgent = qa;
}

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
	return new ALUValue(std::sqrt(op->getFloat()));
}

// Both of the following functions assume little-endian architecture
// The mainframe version and Solaris version will need some work...

ALUValue* AluFunc_frombin(ALUValue* op)
{
	std::string str = op->getStr();
	uint64_t value = 0;

	switch (str.length()) {
	case 1: value = ALUInt((unsigned char)(str[0])); break;
	case 2: {
		uint16_t* pVal = (uint16_t*)str.c_str();
		value = *pVal;
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
		break;
	}
	default: {
		std::string err = "Invalid binary field length " + std::to_string(str.length());
		MYTHROW(err);
	}
	}

	return new ALUValue(ALUInt(value));
}

ALUValue* AluFunc_tobine(ALUValue* op, ALUValue* _bits)
{
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

ALUValue* AluFunc_len(ALUValue* op)
{
	return new ALUValue(ALUInt(op->getStr().length()));
}

ALUValue* AluFunc_first()
{
	bool isFirst = g_pStateQueryAgent->isRunIn();
	return new ALUValue(ALUInt(isFirst ? 1 : 0));
}

ALUValue* AluFunc_iterno()
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
	PSpecString pRet = g_pStateQueryAgent->getFromTo(start, end);
	if (pRet) {
		std::string st(pRet->data());
		delete pRet;
		return new ALUValue(st);
	} else {
		return new ALUValue();
	}
}

ALUValue* AluFunc_range(ALUValue* pStart, ALUValue* pEnd)
{
	return AluFunc_range(pStart->getInt(), pEnd->getInt());
}

ALUValue* AluFunc_word(ALUValue* pIdx)
{
	ALUInt idx = pIdx->getInt();
	ALUInt start = g_pStateQueryAgent->getWordStart(idx);
	ALUInt end = g_pStateQueryAgent->getWordEnd(idx);
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_field(ALUValue* pIdx)
{
	ALUInt idx = pIdx->getInt();
	ALUInt start = g_pStateQueryAgent->getFieldStart(idx);
	ALUInt end = g_pStateQueryAgent->getFieldEnd(idx);
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_words(ALUValue* pStart, ALUValue* pEnd)
{
	ALUInt start = g_pStateQueryAgent->getWordStart(pStart->getInt());
	ALUInt end = g_pStateQueryAgent->getWordEnd(pEnd->getInt());
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_fields(ALUValue* pStart, ALUValue* pEnd)
{
	ALUInt start = g_pStateQueryAgent->getFieldStart(pStart->getInt());
	ALUInt end = g_pStateQueryAgent->getFieldEnd(pEnd->getInt());
	return AluFunc_range(start, end);
}

ALUValue* AluFunc_fieldstart(ALUValue* pIdx)
{
	return new ALUValue(ALUInt(g_pStateQueryAgent->getFieldStart(pIdx->getInt())));
}

ALUValue* AluFunc_fieldend(ALUValue* pIdx)
{
	return new ALUValue(ALUInt(g_pStateQueryAgent->getFieldEnd(pIdx->getInt())));
}

ALUValue* AluFunc_wordstart(ALUValue* pIdx)
{
	return new ALUValue(ALUInt(g_pStateQueryAgent->getWordStart(pIdx->getInt())));
}

ALUValue* AluFunc_wordend(ALUValue* pIdx)
{
	return new ALUValue(ALUInt(g_pStateQueryAgent->getWordEnd(pIdx->getInt())));
}

ALUValue* AluFunc_tf2d(ALUValue* pTimeFormatted, ALUValue* pFormat)
{
	int64_t tm = specTimeConvertFromPrintable(pTimeFormatted->getStr(), pFormat->getStr());
	long double seconds;
	if (0 == (tm % MICROSECONDS_PER_SECOND)) {
		seconds = (long double)(tm / MICROSECONDS_PER_SECOND);
	} else {
		seconds = ((long double)tm) / MICROSECONDS_PER_SECOND;
	}
	return new ALUValue(seconds);
}

ALUValue* AluFunc_d2tf(ALUValue* pValue, ALUValue* pFormat)
{
	int64_t microseconds = ALUInt(((pValue->getFloat()) * MICROSECONDS_PER_SECOND) + 0.5);
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
	if ((start + length - 1) > pStr->length()) {
		length = pStr->length() - start + 1;
	}

	// Finally:
	return new ALUValue(pStr->substr(start-1,length));
}

ALUValue* AluFunc_substr(ALUValue* pBigString, ALUValue* pStart, ALUValue* pLength)
{
	return AluFunc_substring_do(pBigString, pStart->getInt(), pLength->getInt());
}

ALUValue* AluFunc_left(ALUValue* pBigString, ALUValue* pLength)
{
	auto bigLength = pBigString->getStrPtr()->length();
	ALUInt len = pLength->getInt();
	if (len==0) return new ALUValue("",0);
	if (len < 0) len = len + bigLength + 1;
	if (len > bigLength) {
		return new ALUValue(*pBigString->getStrPtr()
				+ std::string(len-bigLength, PAD_CHAR));
	}
	return AluFunc_substring_do(pBigString, 1, len);
}

ALUValue* AluFunc_right(ALUValue* pBigString, ALUValue* pLength)
{
	auto bigLength = pBigString->getStrPtr()->length();
	ALUInt len = pLength->getInt();
	if (len==0) return new ALUValue("",0);
	if (len < 0) len = len + bigLength + 1;
	if (len > bigLength) {
		return new ALUValue(std::string(len-bigLength, PAD_CHAR)
				+ *pBigString->getStrPtr());
	}
	return AluFunc_substring_do(pBigString, bigLength-len+1, len);
}

ALUValue* AluFunc_center(ALUValue* pBigString, ALUValue* pLength)
{
	auto bigLength = pBigString->getStrPtr()->length();
	ALUInt len = pLength->getInt();
	if (len==0) return new ALUValue("",0);
	if (len < 0) len = len + bigLength + 1;
	if (len > bigLength) {
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
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = _pHaystack->getStrPtr();
	size_t pos = pHaystack->find(*pNeedle);
	if (std::string::npos == pos) {
		return new ALUValue(ALUInt(0));
	} else {
		return new ALUValue(ALUInt(pos+1));
	}
}

ALUValue* AluFunc_rpos(ALUValue* _pNeedle, ALUValue* _pHaystack)
{
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
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = _pHaystack->getStrPtr();
	bool bIsIncluded = (std::string::npos != pHaystack->find(*pNeedle));
	return new ALUValue(ALUInt(bIsIncluded ? 1 : 0));
}

ALUValue* AluFunc_conf(ALUValue* _pKey)
{
	std::string key = _pKey->getStr();
	if (configSpecLiteralExists(key)) {
		return new ALUValue(configSpecLiteralGet(key));
	} else {
		return new ALUValue();
	}
}
