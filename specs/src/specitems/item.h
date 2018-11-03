#ifndef SPECS2016__SPECITEMS__ITEM__H
#define SPECS2016__SPECITEMS__ITEM__H

#include <vector>
#include <string>
#include "cli/tokens.h"
#include "processing/conversions.h"
#include "processing/StringBuilder.h"
#include "processing/ProcessingState.h"

enum ApplyRet {
	ApplyRet__Continue,
	ApplyRet__Write,
	ApplyRet__Last
};

#define POS_SPECIAL_VALUE_NEXTWORD  0xffffffff
#define POS_SPECIAL_VALUE_NEXTFIELD 0xfffffffe
#define POS_SPECIAL_VALUE_NEXT      0xfffffffd
#define MAX_OUTPUT_POSITION         65536

class Item {
public:
	virtual ~Item() {}
	virtual std::string Debug() = 0;
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB) = 0;
};

typedef Item* PItem;

class DataField : public Item {
public:
	DataField();
	virtual ~DataField();
	void parse(std::vector<Token> &tokenVec, unsigned int& index);
	virtual std::string Debug();
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB);
private:
	char m_label;
	Token *m_inputRange;
	size_t m_outStart;  /* zero is a special value meaning no output */
	size_t m_maxLength; /* zero is a special value meaning no limit  */
	bool   m_strip;
	StringConversions m_conversion;
};

class TokenItem : public Item {
public:
	TokenItem(Token& t);
	virtual ~TokenItem();
	Token* getToken()   {return mp_Token;}
	virtual std::string Debug();
	virtual ApplyRet apply(ProcessingState& pState, StringBuilder* pSB);
private:
	Token* mp_Token;
};

#endif
