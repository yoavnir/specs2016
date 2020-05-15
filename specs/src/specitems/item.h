#ifndef SPECS2016__SPECITEMS__ITEM__H
#define SPECS2016__SPECITEMS__ITEM__H

#include <vector>
#include <string>
#include <memory>
#include "cli/tokens.h"
#include "processing/conversions.h"
#include "processing/StringBuilder.h"
#include "processing/ProcessingState.h"
#include "utils/alu.h"
#include "utils/TimeUtils.h"

class InputPart {
public:
	virtual ~InputPart() {}
	virtual std::string Debug() = 0;
	virtual PSpecString getStr(ProcessingState& pState) = 0;
	virtual bool        readsLines() {return false;}
	virtual bool        forcesRunoutCycle() {return false;}
};

class LiteralPart : public InputPart {
public:
	LiteralPart(std::string& s) {m_Str = s;}
	virtual ~LiteralPart() {}
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
private:
	std::string m_Str;
};

class RangePart : public InputPart {
public:
	RangePart(int _first, int _last) {_from=_first; _to=_last;}
	virtual ~RangePart() {}
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState) = 0;
	virtual bool        readsLines() {return true;}
protected:
	int _from;
	int _to;
};

class RegularRangePart : public RangePart {
public:
	RegularRangePart(int _first, int _last) : RangePart(_first,_last) {}
	virtual ~RegularRangePart() {}
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
};

class WordRangePart : public RangePart {
public:
	WordRangePart(int _first, int _last, char sep=0) : RangePart(_first,_last) {m_WordSep = sep;}
	virtual ~WordRangePart() {}
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
private:
	char m_WordSep;
};

class FieldRangePart : public RangePart {
public:
	FieldRangePart(int _first, int _last, char sep=0) : RangePart(_first,_last) {m_FieldSep = sep;}
	virtual ~FieldRangePart() {}
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
private:
	char m_FieldSep;
};

class SubstringPart : public InputPart {
public:
	SubstringPart(RangePart* _sub, InputPart* _big) {mp_SubPart = _sub; mp_BigPart = _big;}
	virtual ~SubstringPart();
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
	virtual bool        readsLines() {return mp_BigPart->readsLines();}
private:
	RangePart* mp_SubPart;
	InputPart* mp_BigPart;
};

#define NUMBER_PART_FIELD_LEN  10
#define CLOCKDIFF_PART_FIELD_LEN 12
class NumberPart : public InputPart {
public:
	NumberPart() {m_Num = 0;}
	virtual ~NumberPart() {}
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
	virtual bool        readsLines() {return true;}
private:
	unsigned long m_Num;
};

enum clockType {
	ClockType__Static,
	ClockType__Dynamic,
	ClockType__Diff
};

class ClockPart : public InputPart {
public:
	ClockPart(clockType _type);
	virtual ~ClockPart() {}
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
private:
	clockType    m_Type;
	clockValue   m_StaticClock;
};

class IDPart : public InputPart {
public:
	IDPart(std::string _fieldIdentifier) {m_fieldIdentifier = _fieldIdentifier;}
	virtual ~IDPart() {}
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
private:
	std::string m_fieldIdentifier;
};

class ExpressionPart : public InputPart {
public:
	ExpressionPart(std::string& _expr);
	virtual ~ExpressionPart();
	virtual std::string Debug();
	virtual PSpecString getStr(ProcessingState& pState);
	virtual bool        readsLines();
	virtual bool        forcesRunoutCycle() {return expressionForcesRunoutCycle(m_RPNExpr);}
private:
	AluVec m_RPNExpr;
	bool   m_isAssignment;
	ALUCounterKey	m_counter;
	POperator       m_assnOp;
	std::string m_rawExpression;
};

enum ApplyRet {
	ApplyRet__Continue,
	ApplyRet__ContinueWithDataWritten,
	ApplyRet__Write,
	ApplyRet__Read,
	ApplyRet__ReadStop,
	ApplyRet__EnterLoop,
	ApplyRet__DoneLoop,
	ApplyRet__EOF,
	ApplyRet__UNREAD,
	ApplyRet__ReDo,
	ApplyRet__Break,
	ApplyRet__SkipToNext,
	ApplyRet__Last
};

#define POS_SPECIAL_VALUE_NEXTWORD  0xffffffff
#define POS_SPECIAL_VALUE_NEXTFIELD 0xfffffffe
#define POS_SPECIAL_VALUE_NEXT      0xfffffffd
#define POS_SPECIAL_VALUE_COMPOSED  0xfffffffc
#define MAX_OUTPUT_POSITION         65536

class Item {
public:
	virtual ~Item() {}
	virtual std::string Debug() = 0;
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB) = 0;
	virtual bool readsLines() {return false;}
	virtual bool forcesRunoutCycle() {return false;}
	virtual bool isBreak()    {return false;}
	virtual bool ApplyUnconditionally() {return false;}
};

typedef std::shared_ptr<Item> PItem;

class DataField : public Item {
public:
	DataField();
	virtual ~DataField();
	void parse(std::vector<Token> &tokenVec, unsigned int& index);
	virtual std::string Debug();
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB);
	virtual bool readsLines();
	virtual bool forcesRunoutCycle() {return m_InputPart ? m_InputPart->forcesRunoutCycle() : false;}
private:
	InputPart* getInputPart(std::vector<Token> &tokenVec, unsigned int& index, char _wordSep=0, char _fieldSep=0);
	SubstringPart* getSubstringPart(std::vector<Token> &tokenVec, unsigned int& index);
	void stripString(PSpecString &pOrig);
	void interpretComposedOutputPlacement(std::string& outputPlacement);
	char m_label;
	char m_tailLabel;
	InputPart *m_InputPart;
	size_t m_outStart;  /* zero is a special value meaning no output */
	size_t m_maxLength; /* zero is a special value meaning no limit  */
	bool   m_strip;
	StringConversions m_conversion;
	outputAlignment   m_alignment;
	std::string       m_conversionParam;
	AluVec            m_outputStartExpression;
	AluVec            m_outputWidthExpression;
	AluVec            m_outputAlignmentExpression;
};

typedef std::shared_ptr<DataField> PDataField;

class TokenItem : public Item {
public:
	TokenItem(Token& t);
	virtual ~TokenItem();
	Token* getToken()   {return mp_Token;}
	virtual std::string Debug();
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB);
	virtual bool readsLines();
private:
	Token* mp_Token;
};

typedef std::shared_ptr<TokenItem> PTokenItem;

class SetItem : public Item {
public:
	SetItem(std::string& _statement);
	virtual ~SetItem();
	virtual std::string Debug()		{return m_rawExpression;}
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB);
	virtual bool readsLines();
	virtual bool forcesRunoutCycle() { return expressionForcesRunoutCycle(m_RPNExpression);}
private:
	std::string     m_rawExpression;
	ALUCounterKey   m_key;
	AluAssnOperator m_oper;
	AluVec          m_RPNExpression;
};

typedef std::shared_ptr<SetItem> PSetItem;

class ConditionItem : public Item {
public:
	enum predicate {
		PRED_IF,
		PRED_THEN,
		PRED_ELSE,
		PRED_ELSEIF,
		PRED_ENDIF,
		PRED_WHILE,
		PRED_DO,
		PRED_DONE,
		PRED_ASSERT,
	};
	ConditionItem(std::string& _statement);
	ConditionItem(ConditionItem::predicate _p);
	virtual ~ConditionItem();
	virtual std::string Debug();
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB);
	virtual bool ApplyUnconditionally() {return true;}
	void    setElseIf();
	void    setWhile();
	void    setAssert();
	predicate pred() { return m_pred;}
	virtual bool readsLines();
	virtual bool forcesRunoutCycle() { return expressionForcesRunoutCycle(m_RPNExpression);}
private:
	bool        evaluate();
	std::string m_rawExpression;
	AluVec      m_RPNExpression;
	predicate   m_pred;
	bool   m_isAssignment;
	ALUCounterKey	m_counter;
	POperator       m_assnOp;
};

typedef std::shared_ptr<ConditionItem> PConditionItem;

class BreakItem : public Item {
public:
	BreakItem(char identifier);
	virtual ~BreakItem()  {}
	virtual std::string Debug();
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB);
	virtual bool isBreak() { return true;}
private:
	char m_identifier;
};

typedef std::shared_ptr<BreakItem> PBreakItem;

class SelectItem : public Item {
public:
	SelectItem(std::string& st, bool bIsOutput);
	virtual ~SelectItem() {}
	virtual std::string Debug();
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB);
	bool    isSelectSecond()  {return m_stream==STATION_SECOND;}
private:
	int  m_stream;
	bool bOutput;
};

typedef std::shared_ptr<SelectItem> PSelectItem;

#endif
