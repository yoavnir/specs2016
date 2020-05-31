#include "utils/ErrorReporting.h"
#include "utils/aluFunctions.h"
#include "utils/TimeUtils.h"
#include "utils/aluRand.h"
#include "utils/aluRegex.h"
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

#define ARG_FLOAT_WITH_DEFAULT(arg,def)       \
		((NULL == (arg)) ? def : (arg)->getFloat())

/*
 *
 * HELP command support
 * ====================
 */

size_t g_help_padding = 40;
static void aluFunc_help_builtin_func(std::string name, unsigned int argc, std::string sshort, std::string slong, bool bLong)
{
	size_t lengthSoFar;
	std::cout << "* " << name;
	lengthSoFar = name.length();
	if (sshort.length() > 0) {
		if (bLong) {
			std::cout << sshort << "\n";
		} else {
			size_t pos = sshort.find(")");
			lengthSoFar += pos;
			std::cout << sshort.substr(0,pos+2);
			static std::string dashes = "..........................................";
			std::cout << dashes.substr(0,g_help_padding-lengthSoFar)
						<< sshort .substr(pos+3) << "\n";
		}
	} else {
		static std::string default_args = "x,y,x,w,t";
		std::cout << "(" << default_args.substr(0,argc*2-1) << ")\n";
	}
	if (bLong) {
		if (slong.length() > 0) {
			std::cout << "\n" << slong << "\n\n";
		} else {
			std::cout << "\n";
		}
	}
}

static void aluFunc_help_builtin_header(const std::string& shdr)
{
	static std::string dashes("----------------------------------------");
	std::cout << "\n" << shdr << "\n" << dashes.substr(0,shdr.length()) << "\n";
}

#define X(fn,argc,flags,rl,shorthelp,longhelp) aluFunc_help_builtin_func(#fn,argc,shorthelp,longhelp,false);
#define H(hdr,len)  aluFunc_help_builtin_header(#hdr); g_help_padding = len;
void aluFunc_help_builtin()
{
	ALU_FUNCTION_LIST
}
#undef X
#undef H

#define X(fn,argc,flags,rl,shorthelp,longhelp) if (funcName==#fn) {              \
				std::cout << "\n";                                               \
				aluFunc_help_builtin_func(#fn, argc, shorthelp, longhelp, true); \
				return true;                                                     \
			}
#define H(hdr,len)
bool aluFunc_help_one_builtin(std::string& funcName)
{
	ALU_FUNCTION_LIST
	return false;
}
#undef X
#undef H

/*
 *
 *
 * ALU FUNCTIONS
 * =============
 *
 *
 */

PValue AluFunc_abs(PValue op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	if (op->getType()==counterType__Int) {
		ALUInt i = op->getInt();
		if (i<0) i = -i;
		return mkValue(i);
	} else {
		ALUFloat f = op->getFloat();
		if (f<0) f = -f;
		return mkValue(f);
	}
}

PValue AluFunc_pow(PValue op1, PValue op2)
{
	ASSERT_NOT_ELIDED(op1,1,base);
	ASSERT_NOT_ELIDED(op2,2,exponent);
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return mkValue(std::pow(op1->getFloat(), op2->getFloat()));
	}
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
		return mkValue(ALUInt(std::pow(op1->getInt(), op2->getInt())));
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return mkValue(ALUInt(std::pow(op1->getInt(), op2->getInt())));
	}
	return mkValue(std::pow(op1->getFloat(), op2->getFloat()));
}

PValue AluFunc_sqrt(PValue op)
{
	ASSERT_NOT_ELIDED(op,1,square);
	return mkValue(std::sqrt(op->getFloat()));
}

// Both of the following functions assume little-endian architecture
// The mainframe version and Solaris version will need some work...
static uint64_t binary2uint64(PValue op, unsigned char *pNumBits = NULL)
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

PValue AluFunc_c2u(PValue op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	uint64_t value = binary2uint64(op);
	MYASSERT_WITH_MSG(value <= MAX_ALUInt, "c2u: Binary value exceeds limit");
	return mkValue(ALUInt(value));
}

PValue AluFunc_c2d(PValue op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	unsigned char numBytes;
	uint64_t uvalue = binary2uint64(op, &numBytes);
	switch (numBytes) {
	case 1: {
		char *pvalue = (char*)(&uvalue);
		return mkValue(ALUInt(*pvalue));
	}
	case 2: {
		int16_t *pvalue = (int16_t*)(&uvalue);
		return mkValue(ALUInt(*pvalue));
	}
	case 4: {
		int32_t *pvalue = (int32_t*)(&uvalue);
		return mkValue(ALUInt(*pvalue));
	}
	case 8: {
		int64_t *pvalue = (int64_t*)(&uvalue);
		return mkValue(ALUInt(*pvalue));
	}
	default: {
		std::string err = "Invalid number of bytes: " + std::to_string(numBytes);
		MYTHROW(err);
	}
	}

	return NULL;
}

PValue AluFunc_c2f(PValue op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	std::string str = op->getStr();

	if (str.length() == sizeof(float)) {
		float* pf = (float*) str.c_str();
		return mkValue(ALUFloat(*pf));
	} else if (str.length() == sizeof(double)) {
		double *pd = (double*) str.c_str();
		return mkValue(ALUFloat(*pd));
	} else if (str.length() == sizeof(long double)) {
		long double *pld = (long double*) str.c_str();
		return mkValue(ALUFloat(*pld));
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



PValue AluFunc_frombin(PValue op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	uint64_t value = binary2uint64(op);
	return mkValue(ALUInt(value));
}

PValue AluFunc_tobine(PValue op, PValue _bits)
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
		return mkValue2((char*)&value, bits/8);
	default: {
		std::string err = "Invalid bit length " + _bits->getStr();
		MYTHROW(err);
	}
	}
}

PValue AluFunc_tobin(PValue op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	static PValue ppbit8 = std::make_shared<ALUValue>(ALUInt(8));
	static PValue ppbit16 = std::make_shared<ALUValue>(ALUInt(16));
	static PValue ppbit32 = std::make_shared<ALUValue>(ALUInt(32));
	static PValue ppbit64 = std::make_shared<ALUValue>(ALUInt(64));
	ALUInt value = op->getInt();

	if (0 == (value >> 8)) return AluFunc_tobine(op,ppbit8);
	if (0 == (value >> 16)) return AluFunc_tobine(op,ppbit16);
	if (0 == (value >> 32)) return AluFunc_tobine(op,ppbit32);
	return AluFunc_tobine(op,ppbit64);
}

PValue AluFunc_length(PValue op)
{
	ASSERT_NOT_ELIDED(op,1,op);
	return mkValue(ALUInt(op->getStr().length()));
}

PValue AluFunc_first()
{
	bool isFirst = g_pStateQueryAgent->isRunIn();
	return mkValue(ALUInt(isFirst ? 1 : 0));
}

PValue AluFunc_number()
{
	return mkValue(g_pStateQueryAgent->getIterationCount());
}

PValue AluFunc_recno()
{
	return mkValue(g_pStateQueryAgent->getRecordCount());
}

PValue AluFunc_eof()
{
	bool isRunOut = g_pStateQueryAgent->isRunOut();
	return mkValue(ALUInt(isRunOut ? 1 : 0));
}

PValue AluFunc_wordcount()
{
	return mkValue(ALUInt(g_pStateQueryAgent->getWordCount()));
}

PValue AluFunc_fieldcount()
{
	return mkValue(ALUInt(g_pStateQueryAgent->getFieldCount()));
}

// Helper function
static PValue AluFunc_range(ALUInt start, ALUInt end)
{
	PSpecString pRange = g_pStateQueryAgent->getFromTo(start, end);
	if (pRange) {
		PValue pRet = mkValue(pRange->data());
		return pRet;
	} else {
		return mkValue("");
	}
}

PValue AluFunc_record()
{
	return AluFunc_range(1,-1);
}

PValue AluFunc_range(PValue pStart, PValue pEnd)
{
	ALUInt start = ARG_INT_WITH_DEFAULT(pStart,1);
	ALUInt end = ARG_INT_WITH_DEFAULT(pEnd, -1);
	return AluFunc_range(start, end);
}

PValue AluFunc_word(PValue pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	ALUInt idx = pIdx->getInt();
	ALUInt start = g_pStateQueryAgent->getWordStart(idx);
	ALUInt end = g_pStateQueryAgent->getWordEnd(idx);
	return AluFunc_range(start, end);
}

PValue AluFunc_field(PValue pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	ALUInt idx = pIdx->getInt();
	ALUInt start = g_pStateQueryAgent->getFieldStart(idx);
	ALUInt end = g_pStateQueryAgent->getFieldEnd(idx);
	return AluFunc_range(start, end);
}

PValue AluFunc_wordrange(PValue pStart, PValue pEnd)
{
	ALUInt startIdx = ARG_INT_WITH_DEFAULT(pStart, 1);
	ALUInt endIdx = ARG_INT_WITH_DEFAULT(pEnd, -1);
	ALUInt start = g_pStateQueryAgent->getWordStart(startIdx);
	ALUInt end = g_pStateQueryAgent->getWordEnd(endIdx);
	return AluFunc_range(start, end);
}

PValue AluFunc_fieldrange(PValue pStart, PValue pEnd)
{
	ALUInt startIdx = ARG_INT_WITH_DEFAULT(pStart, 1);
	ALUInt endIdx = ARG_INT_WITH_DEFAULT(pEnd, -1);
	ALUInt start = g_pStateQueryAgent->getFieldStart(startIdx);
	ALUInt end = g_pStateQueryAgent->getFieldEnd(endIdx);
	return AluFunc_range(start, end);
}

PValue AluFunc_fieldindex(PValue pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	return mkValue(ALUInt(g_pStateQueryAgent->getFieldStart(pIdx->getInt())));
}

PValue AluFunc_fieldend(PValue pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	return mkValue(ALUInt(g_pStateQueryAgent->getFieldEnd(pIdx->getInt())));
}

PValue AluFunc_fieldlength(PValue pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	auto idx = pIdx->getInt();
	auto len = g_pStateQueryAgent->getFieldEnd(idx) - g_pStateQueryAgent->getFieldStart(idx) + 1;
	return mkValue(ALUInt(len));
}

PValue AluFunc_wordstart(PValue pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	return mkValue(ALUInt(g_pStateQueryAgent->getWordStart(pIdx->getInt())));
}

PValue AluFunc_wordend(PValue pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	return mkValue(ALUInt(g_pStateQueryAgent->getWordEnd(pIdx->getInt())));
}

PValue AluFunc_wordlen(PValue pIdx)
{
	ASSERT_NOT_ELIDED(pIdx,1,index);
	auto idx = pIdx->getInt();
	auto len = g_pStateQueryAgent->getWordEnd(idx) - g_pStateQueryAgent->getWordStart(idx) + 1;
	return mkValue(ALUInt(len));
}

PValue AluFunc_tf2mcs(PValue pTimeFormatted, PValue pFormat)
{
	ASSERT_NOT_ELIDED(pTimeFormatted,1,formatted_time);
	ASSERT_NOT_ELIDED(pFormat,2,format);
	int64_t tm = specTimeConvertFromPrintable(pTimeFormatted->getStr(), pFormat->getStr());
	return mkValue(ALUInt(tm));
}

PValue AluFunc_mcs2tf(PValue pValue, PValue pFormat)
{
	ASSERT_NOT_ELIDED(pValue,1,time_value);
	ASSERT_NOT_ELIDED(pFormat,2,format);
	int64_t microseconds = pValue->getInt();
	PSpecString printable = specTimeConvertToPrintable(microseconds, pFormat->getStr());
	PValue ret = mkValue2(printable->data(), printable->length());
	return ret;
}

PValue AluFunc_tf2s(PValue pTimeFormatted, PValue pFormat)
{
	ASSERT_NOT_ELIDED(pTimeFormatted,1,formatted_time);
	ASSERT_NOT_ELIDED(pFormat,2,format);
	int64_t tm = specTimeConvertFromPrintable(pTimeFormatted->getStr(), pFormat->getStr());
        ALUFloat seconds = (tm/MICROSECONDS_PER_SECOND);
        ALUFloat microseconds = (ALUFloat)(tm%MICROSECONDS_PER_SECOND)/MICROSECONDS_PER_SECOND;
        return mkValue(ALUFloat(seconds+microseconds));
}

PValue AluFunc_s2tf(PValue pValue, PValue pFormat)
{
	ASSERT_NOT_ELIDED(pValue,1,time_value);
	ASSERT_NOT_ELIDED(pFormat,2,format);
        ALUFloat seconds = pValue->getFloat();
        int64_t microseconds = seconds * MICROSECONDS_PER_SECOND;
	PSpecString printable = specTimeConvertToPrintable(microseconds, pFormat->getStr());
	PValue ret = mkValue2(printable->data(), printable->length());
	return ret;
}


// Substring functions

static PValue AluFunc_substring_do(std::string* pStr, ALUInt start, ALUInt length)
{
	// handle start
	if (start==0) {    // invalid string index in specs
		return mkValue();  // NaN
	}
	else if (start > ALUInt(pStr->length())) {
		return mkValue2("",0);
	} else if (start < 0) {
		start += pStr->length() + 1;
		if (start < 1) {
			return mkValue2("",0);
		}
	}

	// handle length
	if (length < 0) {
		length += pStr->length() + 1; // length=-1 means the length of the string
		if (length < 0) {
			return mkValue2("",0);
		}
	}
	if (size_t(start + length - 1) > pStr->length()) {
		length = pStr->length() - start + 1;
	}

	// Finally:
	return mkValue(pStr->substr(start-1,length));
}

PValue AluFunc_substr(PValue pBigString, PValue pStart, PValue pLength)
{
	std::string* pBigStr = (pBigString) ? pBigString->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();
	ALUInt start = ARG_INT_WITH_DEFAULT(pStart,1);
	ALUInt length = ARG_INT_WITH_DEFAULT(pLength,-1);
	return AluFunc_substring_do(pBigStr, start, length);
}

PValue AluFunc_left(PValue pBigString, PValue pLength)
{
	ASSERT_NOT_ELIDED(pLength,2,length);
	std::string* pBigStr = (pBigString) ? pBigString->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();
	auto bigLength = pBigStr->length();
	ALUInt len = pLength->getInt();
	if (len==0) return mkValue2("",0);
	if (len < 0) len = len + bigLength + 1;
	if (size_t(len) > bigLength) {
		return mkValue(*pBigStr
				+ std::string(len-bigLength, PAD_CHAR));
	}
	return AluFunc_substring_do(pBigStr, 1, len);
}

PValue AluFunc_right(PValue pBigString, PValue pLength)
{
	ASSERT_NOT_ELIDED(pLength,2,length);
	std::string* pBigStr = (pBigString) ? pBigString->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();
	auto bigLength = pBigStr->length();
	ALUInt len = pLength->getInt();
	if (len==0) return mkValue2("",0);
	if (len < 0) len = len + bigLength + 1;
	if (size_t(len) > bigLength) {
		return mkValue(std::string(len-bigLength, PAD_CHAR)
				+ *pBigString->getStrPtr());
	}
	return AluFunc_substring_do(pBigStr, bigLength-len+1, len);
}

PValue AluFunc_center(PValue pBigString, PValue pLength)
{
	ASSERT_NOT_ELIDED(pBigString,1,bigString);
	ASSERT_NOT_ELIDED(pLength,2,length);
	std::string* pBigStr = (pBigString) ? pBigString->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();
	auto bigLength = pBigStr->length();
	ALUInt len = pLength->getInt();
	if (len==0) return mkValue2("",0);
	if (len < 0) len = len + bigLength + 1;
	if (size_t(len) > bigLength) {
		size_t smallHalf = (len-bigLength) / 2;
		size_t bigHalf = (len-bigLength) - smallHalf;
		return mkValue(std::string(smallHalf, PAD_CHAR)
				+ *pBigStr
				+ std::string(bigHalf, PAD_CHAR));
	}
	return AluFunc_substring_do(pBigStr, (bigLength - len) / 2 + 1, len);
}

PValue AluFunc_centre(PValue pBigString, PValue pLength)
{
	return AluFunc_center(pBigString, pLength);
}

PValue AluFunc_pos(PValue _pNeedle, PValue _pHaystack)
{
	ASSERT_NOT_ELIDED(_pNeedle,1,needle);
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = (_pHaystack) ? _pHaystack->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();
	size_t pos = pHaystack->find(*pNeedle);
	if (std::string::npos == pos) {
		return mkValue(ALUInt(0));
	} else {
		return mkValue(ALUInt(pos+1));
	}
}

PValue AluFunc_lastpos(PValue _pNeedle, PValue _pHaystack)
{
	ASSERT_NOT_ELIDED(_pNeedle,1,needle);
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = (_pHaystack) ? _pHaystack->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();
	size_t pos = pHaystack->rfind(*pNeedle);
	if (std::string::npos == pos) {
		return mkValue(ALUInt(0));
	} else {
		return mkValue(ALUInt(pos+1));
	}
}

PValue AluFunc_includes(PValue _pHaystack, PValue _pNeedle1, PValue _pNeedle2, PValue _pNeedle3, PValue _pNeedle4)
{
	ASSERT_NOT_ELIDED(_pNeedle1,2,needle);

	std::string* pHaystack = (_pHaystack) ? _pHaystack->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();

	if (std::string::npos != pHaystack->find(*_pNeedle1->getStrPtr())) {
		return mkValue(ALUInt(1));
	} else if (_pNeedle2 && (std::string::npos != pHaystack->find(*_pNeedle2->getStrPtr()))) {
		return mkValue(ALUInt(1));
	} else if (_pNeedle3 && (std::string::npos != pHaystack->find(*_pNeedle3->getStrPtr()))) {
		return mkValue(ALUInt(1));
	} else if (_pNeedle4 && (std::string::npos != pHaystack->find(*_pNeedle4->getStrPtr()))) {
		return mkValue(ALUInt(1));
	}

	return mkValue(ALUInt(0));
}

PValue AluFunc_includesall(PValue _pHaystack, PValue _pNeedle1, PValue _pNeedle2, PValue _pNeedle3, PValue _pNeedle4)
{
	ASSERT_NOT_ELIDED(_pNeedle1,2,needle);

	std::string* pHaystack = (_pHaystack) ? _pHaystack->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();

	if (std::string::npos == pHaystack->find(*_pNeedle1->getStrPtr())) {
		return mkValue(ALUInt(0));
	} else if (_pNeedle2 && (std::string::npos == pHaystack->find(*_pNeedle2->getStrPtr()))) {
		return mkValue(ALUInt(0));
	} else if (_pNeedle3 && (std::string::npos == pHaystack->find(*_pNeedle3->getStrPtr()))) {
		return mkValue(ALUInt(0));
	} else if (_pNeedle4 && (std::string::npos == pHaystack->find(*_pNeedle4->getStrPtr()))) {
		return mkValue(ALUInt(0));
	}

	return mkValue(ALUInt(1));
}

PValue AluFunc_rmatch(PValue _pHaystack, PValue _pExp, PValue _pFlags)
{
	ASSERT_NOT_ELIDED(_pExp,2,regExp);
	std::string* pHaystack = (_pHaystack) ? _pHaystack->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();

	if (_pFlags) {
		return mkValue(ALUInt(regexMatch(pHaystack, _pExp, _pFlags->getStrPtr())));
	}
	return mkValue(ALUInt(regexMatch(pHaystack, _pExp)));
}

PValue AluFunc_rsearch(PValue _pHaystack, PValue _pExp, PValue _pFlags)
{
	ASSERT_NOT_ELIDED(_pExp,2,regExp);
	std::string* pHaystack = (_pHaystack) ? _pHaystack->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();

	if (_pFlags) {
		return mkValue(ALUInt(regexSearch(pHaystack, _pExp, _pFlags->getStrPtr())));
	}
	return mkValue(ALUInt(regexSearch(pHaystack, _pExp)));
}

PValue AluFunc_rreplace(PValue _pHaystack, PValue _pExp, PValue _pFmt, PValue _pFlags)
{
	ASSERT_NOT_ELIDED(_pExp,2,regExp);
	ASSERT_NOT_ELIDED(_pFmt,3,format);

	std::string* pHaystack = (_pHaystack) ? _pHaystack->getStrPtr() : g_pStateQueryAgent->currRecord()->sdata();
	std::string sFmt = _pFmt->getStr();

	if (_pFlags) {
		return mkValue(regexReplace(pHaystack, _pExp, sFmt, _pFlags->getStrPtr()));
	}
	return mkValue(regexReplace(pHaystack, _pExp, sFmt));
}

PValue AluFunc_conf(PValue _pKey, PValue _pDefault)
{
	ASSERT_NOT_ELIDED(_pKey,1,key);
	std::string key = _pKey->getStr();
	if (configSpecLiteralExists(key)) {
		return mkValue(configSpecLiteralGet(key));
	} else if (_pDefault) {
		return mkValue(*_pDefault);
	} else {
		return mkValue();
	}
}

extern std::string conv_D2X(std::string& s);
PValue AluFunc_d2x(PValue _pDecValue)
{
	ASSERT_NOT_ELIDED(_pDecValue,1,decValue);
	std::string dec = _pDecValue->getStr();
	return mkValue(conv_D2X(dec));
}

extern std::string conv_X2D(std::string& s);
static ALUInt SZLL = ALUInt(2 * sizeof(long long int));
PValue AluFunc_x2d(PValue _pHexValue, PValue pLength)
{
	static std::string zeropad = "0000000000000000";
	static std::string ffffpad = "FFFFFFFFFFFFFFFF";
	ASSERT_NOT_ELIDED(_pHexValue,1,hexValue);
	auto hex = _pHexValue->getStr();
	ALUInt len = ARG_INT_WITH_DEFAULT(pLength, 0);

	if (len < 1) {
		return mkValue(conv_X2D(hex));
	}

	if (len > SZLL) len = SZLL;
	while (size_t(len) > hex.length()) len--;

	if (hex.length()==0) return mkValue(ALUInt(0));

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

	return mkValue(ALUInt(value));
}

extern std::string conv_C2X(std::string& s);
PValue AluFunc_c2x(PValue _pCharValue)
{
	ASSERT_NOT_ELIDED(_pCharValue,1,charValue);
	std::string cv = _pCharValue->getStr();
	return mkValue(conv_C2X(cv));
}

std::string conv_X2CH(std::string& s);
PValue AluFunc_x2ch(PValue _pHexValue)
{
	ASSERT_NOT_ELIDED(_pHexValue,1,hexValue);
	std::string hex = _pHexValue->getStr();
	return mkValue(conv_X2CH(hex));
}

extern std::string conv_UCASE(std::string& s);
PValue AluFunc_ucase(PValue _pString)
{
	ASSERT_NOT_ELIDED(_pString,1,string);
	std::string st = _pString->getStr();
	return mkValue(conv_UCASE(st));
}

extern std::string conv_LCASE(std::string& s);
PValue AluFunc_lcase(PValue _pString)
{
	ASSERT_NOT_ELIDED(_pString,1,string);
	std::string st = _pString->getStr();
	return mkValue(conv_LCASE(st));
}

extern std::string conv_BSWAP(std::string& s);
PValue AluFunc_bswap(PValue _pString)
{
	ASSERT_NOT_ELIDED(_pString,1,string);
	std::string st = _pString->getStr();
	return mkValue(conv_BSWAP(st));
}

PValue AluFunc_break(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	bool bIsBreakEstablished = g_pStateQueryAgent->breakEstablished(fId);
	return mkValue(ALUInt(bIsBreakEstablished ? 1 : 0));
}

PValue AluFunc_present(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	bool bIsSet = g_pStateQueryAgent->fieldIdentifierIsSet(fId);
	return mkValue(ALUInt(bIsSet ? 1 : 0));
}

PValue AluFunc_sum(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "SUM requested for undefined field identifier")
	return pVStats->sum();
}

PValue AluFunc_min(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "MIN requested for undefined field identifier")
	return pVStats->_min();
}

PValue AluFunc_max(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "MAX requested for undefined field identifier")
	return pVStats->_max();
}

PValue AluFunc_average(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "AVERAGE requested for undefined field identifier")
	return pVStats->average();
}

PValue AluFunc_variance(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "VARIANCE requested for undefined field identifier")
	return pVStats->variance();
}

PValue AluFunc_stddev(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "STDDEV requested for undefined field identifier")
	return pVStats->stddev();
}

PValue AluFunc_stderrmean(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PAluValueStats pVStats = g_pStateQueryAgent->valueStatistics(fId);
	MYASSERT_WITH_MSG(pVStats!=NULL, "STDERRMEAN requested for undefined field identifier")
	return pVStats->stderrmean();
}

PValue AluFunc_rand(PValue pLimit)
{
	if (pLimit) {
		ALUInt res = AluRandGetIntUpTo(pLimit->getInt());
		return mkValue(res);
	} else {
		static ALUInt decimalLimit = 100000000000000000;
		ALUInt randomDecimal = AluRandGetIntUpTo(decimalLimit);
		std::ostringstream str;
		str << "0." << std::setw(17) << std::setfill('0') << randomDecimal;
		return mkValue(str.str());
	}
}

PValue AluFunc_floor(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(floor(pX->getFloat())));
}

PValue AluFunc_round(PValue pX, PValue pDecimals)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	if (pDecimals) {
		if (pDecimals->getInt() < 0) {
			MYTHROW("round: value for 'decimals' must not be negative");
		}
		ALUFloat scale = pow(((ALUFloat)(10.0)), pDecimals->getInt());
		return mkValue(ALUFloat((round(scale * pX->getFloat())) / scale));
	} else {
		return mkValue(ALUFloat(round(pX->getFloat())));
	}
}

PValue AluFunc_ceil(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(ceil(pX->getFloat())));
}

PValue AluFunc_sin(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(sin(pX->getFloat())));
}

PValue AluFunc_cos(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(cos(pX->getFloat())));
}

PValue AluFunc_tan(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(tan(pX->getFloat())));
}

PValue AluFunc_arcsin(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(asin(pX->getFloat())));
}

PValue AluFunc_arccos(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(acos(pX->getFloat())));
}

PValue AluFunc_arctan(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(atan(pX->getFloat())));
}

static ALUFloat degrees_to_radians = 0.0174532925199433;
static ALUFloat radians_to_degrees = 57.29577951308232;

PValue AluFunc_dsin(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(sin(degrees_to_radians*pX->getFloat())));
}

PValue AluFunc_dcos(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(cos(degrees_to_radians*pX->getFloat())));
}

PValue AluFunc_dtan(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(tan(degrees_to_radians*pX->getFloat())));
}

PValue AluFunc_arcdsin(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(radians_to_degrees*asin(pX->getFloat())));
}

PValue AluFunc_arcdcos(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(radians_to_degrees*acos(pX->getFloat())));
}

PValue AluFunc_arcdtan(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(ALUFloat(radians_to_degrees*atan(pX->getFloat())));
}

static ALUFloat e(2.71828182845904523536028747135266249775724709369995);
PValue AluFunc_exp(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(std::pow(e, pX->getFloat()));
}

PValue AluFunc_log(PValue pX, PValue pBase)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	ALUFloat res;
	if (pBase) {
		res = std::log(pX->getFloat()) / std::log(pBase->getFloat());
	} else {
		res = std::log(pX->getFloat());
	}
	return mkValue(res);
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


PValue AluFunc_fmap_nelem(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	return mkValue(pfMap->nelem());
}

PValue AluFunc_fmap_nsamples(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	return mkValue(pfMap->count());
}

PValue AluFunc_fmap_count(PValue _pFieldIdentifier, PValue pVal)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	ASSERT_NOT_ELIDED(pVal,2,s);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	std::string s = pVal->getStr();
	return mkValue((*pfMap)[s]);
}

PValue AluFunc_fmap_frac(PValue _pFieldIdentifier, PValue pVal)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	ASSERT_NOT_ELIDED(pVal,2,s);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	std::string s = pVal->getStr();
	ALUFloat frac = ALUFloat((*pfMap)[s]) / ALUFloat(pfMap->count());
	return mkValue(frac);
}

PValue AluFunc_fmap_pct(PValue _pFieldIdentifier, PValue pVal)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	ASSERT_NOT_ELIDED(pVal,2,s);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	std::string s = pVal->getStr();
	ALUFloat frac = ALUFloat((*pfMap)[s]) / ALUFloat(pfMap->count());
	return mkValue(PERCENTS * frac);
}

PValue AluFunc_fmap_common(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	return mkValue(pfMap->mostCommon());
}

PValue AluFunc_fmap_rare(PValue _pFieldIdentifier)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	return mkValue(pfMap->leastCommon());
}

PValue AluFunc_fmap_sample(PValue _pFieldIdentifier, PValue pVal)
{
	ASSERT_NOT_ELIDED(_pFieldIdentifier,1,fieldIdentifier);
	ASSERT_NOT_ELIDED(pVal,2,s);
	char fId = (char)(_pFieldIdentifier->getInt());
	PFrequencyMap pfMap = g_pStateQueryAgent->getFrequencyMap(fId);
	std::string s = pVal->getStr();
	pfMap->note(s);
	return mkValue((*pfMap)[s]);
}

PValue AluFunc_fmap_dump(PValue _pFieldIdentifier, PValue pFormat, PValue pOrder, PValue pPct)
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

	return mkValue(pfMap->dump(f, o, includePercentage));
}


PValue AluFunc_string(PValue pX)
{
	ASSERT_NOT_ELIDED(pX,1,x);
	return mkValue(pX->getStr());
}

PValue AluFunc_substitute(PValue pSrc, PValue pSearchString, PValue pSubstitute, PValue pMax)
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

	return mkValue(res);
}

PValue AluFunc_sfield(PValue pStr, PValue pCount, PValue pSep)
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
		return mkValue("");
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
			return mkValue("");
		} else {
			char *pEnd = pc;
			while ((*pEnd!='\0') && (*pEnd!=sep)) pEnd++;
			return mkValue2(pc, (pEnd-pc));
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
			return mkValue("");
		} else {
			char *pBegin = pc;
			while ((pBegin>pStart) && (*pBegin!=sep)) pBegin--;
			return mkValue2(pBegin+1, (pc-pBegin));
		}
	}
}

PValue AluFunc_sword(PValue pStr, PValue pCount, PValue pSep)
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
		return mkValue("");
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
			return mkValue("");
		} else {
			char *pEnd = pc;
			while ((*pEnd!='\0') && (*pEnd!=sep)) pEnd++;
			return mkValue2(pc, (pEnd-pc));
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
			return mkValue("");
		} else {
			char *pBegin = pc;
			while ((pBegin>pStart) && (*pBegin!=sep)) pBegin--;
			return mkValue2(pBegin+1, (pc-pBegin));
		}
	}
}

/* REXX Functions */

PValue AluFunc_abbrev_do(PValue pInformation, PValue pInfo, size_t len)
{
	std::string sBig = pInformation->getStr();
	std::string sLittle = pInfo->getStr().substr(0,len);

	if (sBig.length() >= sLittle.length()) {
		if (sLittle == sBig.substr(0,sLittle.length())) {
			return mkValue(ALUInt(1));
		}
	}

	return mkValue(ALUInt(0));
}

PValue AluFunc_abbrev(PValue pInformation, PValue pInfo, PValue pLen)
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

PValue AluFunc_bitand(PValue pS1, PValue pS2)
{
	ASSERT_NOT_ELIDED(pS1,1,s1);
	ASSERT_NOT_ELIDED(pS2,2,s2);
	std::string s1 = pS1->getStr();
	std::string s2 = pS2->getStr();

	size_t minlen = (s1.length() > s2.length()) ? s2.length() : s1.length();

	unsigned char pBuff[minlen];

	const unsigned char *pc1 = (const unsigned char*)(s1.c_str());
	const unsigned char *pc2 = (const unsigned char*)(s2.c_str());

	for (size_t i = 0 ; i < minlen ; i++) {
		pBuff[i] = pc1[i] & pc2[i];
	}

	PValue pRet = mkValue2((const char*)(pBuff), minlen);

	return pRet;
}

PValue AluFunc_bitor(PValue pS1, PValue pS2)
{
	ASSERT_NOT_ELIDED(pS1,1,s1);
	ASSERT_NOT_ELIDED(pS2,2,s2);
	std::string s1 = pS1->getStr();
	std::string s2 = pS2->getStr();

	size_t minlen = (s1.length() > s2.length()) ? s2.length() : s1.length();

	unsigned char pBuff[minlen];

	const unsigned char *pc1 = (const unsigned char*)(s1.c_str());
	const unsigned char *pc2 = (const unsigned char*)(s2.c_str());

	for (size_t i = 0 ; i < minlen ; i++) {
		pBuff[i] = pc1[i] | pc2[i];
	}

	PValue pRet = mkValue2((const char*)(pBuff), minlen);

	return pRet;
}

PValue AluFunc_bitxor(PValue pS1, PValue pS2)
{
	ASSERT_NOT_ELIDED(pS1,1,s1);
	ASSERT_NOT_ELIDED(pS2,2,s2);
	std::string s1 = pS1->getStr();
	std::string s2 = pS2->getStr();

	size_t minlen = (s1.length() > s2.length()) ? s2.length() : s1.length();

	unsigned char pBuff[minlen];

	const unsigned char *pc1 = (const unsigned char*)(s1.c_str());
	const unsigned char *pc2 = (const unsigned char*)(s2.c_str());

	for (size_t i = 0 ; i < minlen ; i++) {
		pBuff[i] = pc1[i] ^ pc2[i];
	}

	PValue pRet = mkValue2((const char*)(pBuff), minlen);

	return pRet;
}

PValue AluFunc_compare_do(PValue pS1, PValue pS2, char pad)
{
	auto s1 = pS1->getStr();
	auto s2 = pS2->getStr();
	if (s1 == s2) {
		return mkValue(ALUInt(0));
	}
	auto s1Len = s1.length();
	auto s2Len = s2.length();

	auto maxLen = std::max(s1Len, s2Len);

	for (size_t l = 0 ; l < maxLen ; l++) {
		char c1 = (l < s1Len) ? s1[l] : pad;
		char c2 = (l < s2Len) ? s2[l] : pad;

		if (c1 != c2) {
			return mkValue(ALUInt(l)+1);
		}
	}

	if (s1Len==s2Len) {
		std::string err = "compare/comparep: different character was not found; s1=<"
				+ s1 + ">; s2=>" + s2 + ">";
		MYTHROW(err);
	}

	// The padding made them equal
	return mkValue(ALUInt(0));
}

PValue AluFunc_compare(PValue pS1, PValue pS2, PValue pPad)
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

PValue AluFunc_copies(PValue pString, PValue pTimes)
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

	return mkValue(res);
}

PValue AluFunc_delstr(PValue pString, PValue pStart, PValue pLength)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pStart,2,start);
	auto theString = pString->getStr();
	auto start = pStart->getInt();

	if (start < 1) start = 1;

	// If start is after end of the string, we return the whole string
	if (size_t(start) > theString.length()) return mkValue(theString);

	std::string res = theString.substr(0,start-1);

	auto length = ARG_INT_WITH_DEFAULT(pLength,0);
	// zero is a special value meaning delete to the end. If the length is greater
	// than the remainder, also delete to the end.
	if (0>=length || size_t(length+start) > theString.length()) return mkValue(res);

	res += theString.substr(start+length-1);
	return mkValue(res);
}

PValue AluFunc_delword(PValue pString, PValue pStart, PValue pLength)
{
	ASSERT_NOT_ELIDED(pString,1,string);
	ASSERT_NOT_ELIDED(pStart,2,start);
	ASSERT_NOT_ELIDED(pLength,3,length);
	auto theString = pString->getStr();

	auto start = pStart->getInt();
	if (start < 1) start = 1;

	auto length = pLength->getInt();
	if (0>=length) length = MAX_ALUInt - start;

	if (0 == theString.length()) return mkValue(theString);

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

	return mkValue(res);
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

PValue AluFunc_find(PValue string, PValue phrase)
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

		if (!foundMismatch) return mkValue(ALUInt(i+1));
	}

	return mkValue(ALUInt(0));
}

PValue AluFunc_index(PValue _pHaystack, PValue _pNeedle, PValue _pStart)
{
	ASSERT_NOT_ELIDED(_pHaystack,1,haystack);
	ASSERT_NOT_ELIDED(_pNeedle,2,needle);
	std::string* pNeedle = _pNeedle->getStrPtr();
	std::string* pHaystack = _pHaystack->getStrPtr();
	ALUInt start = ARG_INT_WITH_DEFAULT(_pStart, 1);

	if (start < 1) start = 1;

	size_t pos = pHaystack->find(*pNeedle, (start-1));
	if (std::string::npos == pos) {
		return mkValue(ALUInt(0));
	} else {
		return mkValue(ALUInt(pos+1));
	}
}

static PValue insert_do(std::string& str, std::string& tgt, size_t pos, size_t len, char pad)
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

	return mkValue(ret);
}

PValue AluFunc_insert(PValue pString, PValue pTarget, PValue pPosition, PValue pLength, PValue pPad)
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

static PValue justify_do(std::string& str, size_t len, char pad)
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

	return mkValue(ret);
}

PValue AluFunc_justify(PValue pStr, PValue pLen, PValue pPad)
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

PValue AluFunc_overlay(PValue pString1, PValue pString2, PValue pStart, PValue pLength, PValue pPad)
{
	ASSERT_NOT_ELIDED(pString1,1,string1);
	ASSERT_NOT_ELIDED(pString2,2,string1);
	auto str1 = pString1->getStr();
	auto str2 = pString2->getStr();
	ALUInt start = ARG_INT_WITH_DEFAULT(pStart, 1);
	ALUInt length = ARG_INT_WITH_DEFAULT(pLength, 0);
	std::string padStr = ARG_STR_WITH_DEFAULT(pPad, " ");

	// sanity checks
	if (0 == str1.length()) return mkValue(str2);
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

	return mkValue(ret);
}

PValue AluFunc_reverse(PValue pStr)
{
	ASSERT_NOT_ELIDED(pStr,1,str);
	auto str = pStr->getStr();
	std::reverse(str.begin(), str.end());
	return mkValue(str);
}

PValue AluFunc_sign(PValue pNumber)
{
	ASSERT_NOT_ELIDED(pNumber,1,number);
	auto num = pNumber->getFloat();
	ALUInt ret = 0;
	if (num > 0) {
		ret = 1;
	} else if (num < 0) {
		ret = -1;
	}

	return mkValue(ret);
}

PValue AluFunc_space(PValue pStr, PValue pLength, PValue pPad)
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

	return mkValue(ret);
}

PValue AluFunc_strip(PValue pString, PValue pOption, PValue pPad)
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

	return mkValue(ret);
}

PValue AluFunc_subword(PValue pString, PValue pStart, PValue pLength)
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
			return mkValue("");
		}
		cstart = startVec[start-1];
	}

	if (0 == len) {
		return mkValue(str.substr(cstart));
	} else {
		auto endVec = breakIntoWords_end(str);
		if (size_t(start + len - 1) > endVec.size()) {
			return mkValue(str.substr(cstart));
		}
		cend = endVec[start+len-2];
		return mkValue(str.substr(cstart, cend-cstart+1));
	}
}

PValue AluFunc_translate(PValue pString, PValue pTableOut, PValue pTableIn, PValue pPad)
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

	return mkValue(str);
}

PValue AluFunc_verify(PValue pString, PValue pReference, PValue pOption, PValue pStart)
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

	return mkValue(ret);
}

PValue AluFunc_wordindex(PValue pString, PValue pIdx)
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

	return mkValue(ret);
}

PValue AluFunc_wordlength(PValue pString, PValue pIdx)
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

	return mkValue(ret);
}

PValue AluFunc_wordpos(PValue pPhrase, PValue pString, PValue pStart)
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
		if (sub==phrase) return mkValue(ALUInt(i+1));
	}

	return mkValue(ALUInt(0));
}

PValue AluFunc_words(PValue pStr)
{
	ASSERT_NOT_ELIDED(pStr,1,string);
	auto str = pStr->getStr();
	auto startVec = breakIntoWords(str);
	ALUInt ret = startVec.size();
	return mkValue(ret);
}

PValue AluFunc_xrange(PValue pStart, PValue pEnd)
{
	std::string startStr = ARG_STR_WITH_DEFAULT(pStart,"");
	std::string endStr = ARG_STR_WITH_DEFAULT(pEnd,"");

	int start = (startStr.length() > 0) ? startStr[0] : 0;
	int end = (endStr.length() > 0) ? endStr[0] : 0xff;

	while (start < 0) start += 256;
	while (end < 0) end += 256;

	if (end < start) return mkValue(std::string(""));

	std::string ret;
	for (int i = start ; i <= end ; i++) ret += char(i);

	return mkValue(ret);
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


PValue AluFunc_fmt(PValue pVal, PValue pFormat, PValue pDigits, PValue pDecimal, PValue pSep)
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
		return mkValue(oss.str());
	}

	oss << pVal->getFloat();

	return mkValue(oss.str());
}

PValue AluFunc_next()
{
	if (!g_PositionGetter) return mkValue(ALUInt(1));
	return mkValue(ALUInt(g_PositionGetter->pos()));
}

PValue AluFunc_rest()
{
	static std::string sName("cols");
	static std::string sCols = configSpecLiteralGet(sName);
	static ALUInt cols = std::stoul(sCols);
	return mkValue(ALUInt(cols - g_PositionGetter->pos() + 1));
}

PValue AluFunc_defined(PValue pName)
{
	ASSERT_NOT_ELIDED(pName,1,confString);
	auto name = pName->getStr();
	if (configSpecLiteralExists(name)) {
		return mkValue(ALUInt(1));
	} else {
		return mkValue(ALUInt(0));
	}
}


#ifdef DEBUG
PValue AluFunc_testfunc(PValue pArg1, PValue pArg2, PValue pArg3, PValue pArg4)
{
	std::string str1 = pArg1 ? pArg1->getStr() : "(nil)";
	std::string str2 = pArg2 ? pArg2->getStr() : "(nil)";
	std::string str3 = pArg3 ? pArg3->getStr() : "(nil)";
	std::string str4 = pArg4 ? pArg4->getStr() : "(nil)";

	std::string res = str1 + "," + str2 + "," + str3 + "," + str4;

	return mkValue(res);
}
#endif
