#include "utils/platform.h"
#include "processing/Config.h"
#include "utils/TimeUtils.h"
#include "utils/ErrorReporting.h"
#include "item.h"
#include <math.h>

extern ALUCounters g_counters;

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
		return nullptr;
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
	if (pState.recordNotAvailable()) return SpecString::newString();
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
	if (pState.recordNotAvailable()) return SpecString::newString();
	char keepSeparator = DEFAULT_WORDSEPARATOR;
	if (m_WordSep) {
		keepSeparator = pState.getWSChar();
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
	if (pState.recordNotAvailable()) return SpecString::newString();
	char keepSeparator = DEFAULT_FIELDSEPARATOR;
	if (m_FieldSep) {
		keepSeparator = pState.getFSChar();
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
	mp_BigPart = nullptr;
	mp_SubPart = nullptr;
}

std::string SubstringPart::Debug()
{
	return "Substring ("+mp_SubPart->Debug()+") of "+mp_BigPart->Debug();
}

PSpecString SubstringPart::getStr(ProcessingState& pState)
{
	PSpecString bigPart = mp_BigPart->getStr(pState);

	if (!bigPart) return nullptr;

	// Create the special pState for the substring.
	ProcessingState subState;
	subState.setString(bigPart);

	PSpecString ret = mp_SubPart->getStr(subState);
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
	m_StaticClock = specTimeGetTOD();
}

std::string ClockPart::Debug()
{
	switch (m_Type) {
	case ClockType__Static:
		return "TODclock";
	case ClockType__Dynamic:
		return "TODclock(dynamic)";
	case ClockType__Diff:
		return "TimeDiff";
	default:
		MYTHROW("Invalid clock type");
	}
}

PSpecString ClockPart::getStr(ProcessingState& pState)
{
	clockValue timeStamp;
	switch (m_Type) {
	case ClockType__Static:
		timeStamp = m_StaticClock;
		break;
	case ClockType__Dynamic:
		timeStamp = specTimeGetTOD();
		break;
	case ClockType__Diff:
		{
			clockValue diff = specTimeGetTOD() - m_StaticClock;
			std::string s = std::to_string(diff);
			s = std::string(CLOCKDIFF_PART_FIELD_LEN - s.length(), ' ') + s;
			return SpecString::newString(s);
		}
	}
	std::string asString = std::to_string(timeStamp);
	return SpecString::newString(asString);
}

std::string IDPart::Debug()
{
	return "ID:" + m_fieldIdentifier;
}

PSpecString IDPart::getStr(ProcessingState& pState)
{
	return SpecStringCopy(pState.fieldIdentifierGet(m_fieldIdentifier[0]));
}

ExpressionPart::ExpressionPart(std::string& _expr)
{
	AluVec infixExpression;
	MYASSERT(parseAluExpression(_expr, infixExpression));
	if (expressionIsAssignment(infixExpression)) {
		PUnit aUnit = infixExpression[0];
		auto pCounterUnit = std::dynamic_pointer_cast<AluUnitCounter>(aUnit);
		MYASSERT(nullptr != pCounterUnit);
		m_counter = pCounterUnit->getKey();
		m_assnOp = std::dynamic_pointer_cast<AluAssnOperator>(infixExpression[1]);
		MYASSERT(nullptr != m_assnOp);
		infixExpression.erase(infixExpression.begin(), infixExpression.begin()+2);
		m_isAssignment = true;
	} else {
		m_isAssignment = false;
	}

	MYASSERT(convertAluVecToPostfix(infixExpression, m_RPNExpr, true));
	m_rawExpression = _expr;
}

ExpressionPart::~ExpressionPart()
{
}

std::string ExpressionPart::Debug()
{
	return "Expression:" + m_rawExpression;
}

PSpecString ExpressionPart::getStr(ProcessingState& pState)
{
	PValue res;
	if (m_isAssignment) {
		ALUPerformAssignment(m_counter, m_assnOp, m_RPNExpr, &g_counters);
		res = g_counters.getPointer(m_counter);
	} else {
		res = evaluateExpression(m_RPNExpr, &g_counters);
	}
	std::string ret = res->getStr();
	return SpecString::newString(ret);
}

bool ExpressionPart::readsLines()
{
	return AluExpressionReadsLines(m_RPNExpr);
}
