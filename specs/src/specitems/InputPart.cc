#include <arpa/inet.h>
#include "processing/Config.h"
#include "utils/TimeUtils.h"
#include "utils/ErrorReporting.h"
#include "item.h"

std::string LiteralPart::Debug()
{
	return "/" + m_Str + "/";
}

PSpecString LiteralPart::getStr(ProcessingState& pState)
{
	if (!g_bSupportUTF8) {
		return SpecString::newString(m_Str);
	} else {
		MYTHROW("UTF-8 is not supported yet");
		return NULL;
	}
}

std::string RangePart::Debug()
{
	if (_from!=_to) {
		return "["+std::to_string(_from)+":"+std::to_string(_to)+"]";
	} else {
		return "["+std::to_string(_from)+"]";
	}
}

std::string RegularRangePart::Debug()
{
	return "Range" + RangePart::Debug();
}

PSpecString RegularRangePart::getStr(ProcessingState& pState)
{
	return pState.getFromTo(_from, _to);
}

std::string WordRangePart::Debug()
{
	if (m_WordSep) {
		return "Words" + RangePart::Debug() + "(separated by /" + m_WordSep + "/)";
	} else {
		return "Words" + RangePart::Debug();
	}
}

PSpecString WordRangePart::getStr(ProcessingState& pState)
{
	char keepSeparator;
	if (m_WordSep) {
		keepSeparator = pState.m_wordSeparator;
		pState.setWSChar(m_WordSep);  // as a side-effect, invalidates the current word list
	}

	PSpecString ret;
	int wordCount = int(pState.getWordCount());
	if (_from > wordCount) {
		ret = SpecString::newString();
	} else {
		ret = pState.getFromTo(pState.getWordStart(_from), pState.getWordEnd(_to));
	}

	if (m_WordSep) {
		pState.setWSChar(keepSeparator);
	}

	return ret;
}

std::string FieldRangePart::Debug()
{
	if (m_FieldSep) {
		return "Fields" + RangePart::Debug() + "(separated by /" + m_FieldSep + "/)";
	} else {
		return "Fields" + RangePart::Debug();
	}
}

PSpecString FieldRangePart::getStr(ProcessingState& pState)
{
	char keepSeparator;
	if (m_FieldSep) {
		keepSeparator = pState.m_fieldSeparator;
		pState.setFSChar(m_FieldSep);  // as a side-effect, invalidates the current field list
	}

	PSpecString ret;
	int fieldCount = int(pState.getFieldCount());
	if (_from > fieldCount) {
		ret = SpecString::newString();
	} else {
		ret = pState.getFromTo(pState.getFieldStart(_from), pState.getFieldEnd(_to));
	}

	if (m_FieldSep) {
		pState.setFSChar(keepSeparator);
	}

	return ret;
}

SubstringPart::~SubstringPart()
{
	if (mp_BigPart) {
		delete mp_BigPart;
	}

	if (mp_SubPart) {
		delete mp_SubPart;
	}
}

std::string SubstringPart::Debug()
{
	return "Substring ("+mp_SubPart->Debug()+") of "+mp_BigPart->Debug();
}

PSpecString SubstringPart::getStr(ProcessingState& pState)
{
	PSpecString bigPart = mp_BigPart->getStr(pState);

	if (!bigPart) return NULL;

	// Create the special pState for the substring.
	ProcessingState subState;
	subState.setString(bigPart);

	PSpecString ret = mp_SubPart->getStr(subState);
	delete bigPart;
	return ret;
}

std::string NumberPart::Debug()
{
	return "Number";
}

PSpecString NumberPart::getStr(ProcessingState& pState)
{
	std::string s = std::to_string(++m_Num);
	s = std::string(NUMBER_PART_FIELD_LEN - s.length(), ' ') + s;
	return SpecString::newString(s);
}

ClockPart::ClockPart(clockType _type)
{
	m_Type = _type;
	if (m_Type==ClockType__Static) {
		m_StaticClock = getCurrentTODClock();
	}
}

std::string ClockPart::Debug()
{
	switch (m_Type) {
	case ClockType__Static:
		return "TODclock";
	case ClockType__Dynamic:
		return "TODclock(dynamic)";
	}
}

PSpecString ClockPart::getStr(ProcessingState& pState)
{
	uint64_t timeStamp;
	switch (m_Type) {
	case ClockType__Static:
		timeStamp = htonll(m_StaticClock);
		break;
	case ClockType__Dynamic:
		timeStamp = htonll(getCurrentTODClock());
	}
	return SpecString::newString(((char*)&timeStamp), 8);
}

std::string IDPart::Debug()
{
	return "ID:" + m_fieldIdentifier;
}

PSpecString IDPart::getStr(ProcessingState& pState)
{
	return SpecStringCopy(pState.fieldIdentifierGet(m_fieldIdentifier[0]));
}
