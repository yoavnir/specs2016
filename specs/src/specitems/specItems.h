#ifndef SPECS2016__SPECITEMS__SPECITEMS__H
#define SPECS2016__SPECITEMS__SPECITEMS__H

#include <vector>
#include "item.h"
#include "cli/tokens.h"
#include "processing/StringBuilder.h"
#include "processing/Reader.h"
#include "processing/Writer.h"
#include "processing/ProcessingState.h"

class itemGroup {
public:
	itemGroup();
	~itemGroup()  {}
	void Compile(std::vector<Token> &tokenVec, unsigned int& index);
	bool processDo(StringBuilder& sb, ProcessingState& pState, Reader* pRd, Writer* pRw);
	void process(StringBuilder& sb, ProcessingState& pState, Reader& rd, Writer& wr);
	void setRegularRunAtEOF()  { bFoundSelectSecond = true; }
	bool needRunoutCycle()     { return bNeedRunoutCycle;   }
	std::string Debug();
	bool readsLines();
private:
	bool bNeedRunoutCycle;
	bool bFoundSelectSecond;
	void addItem(PItem pItem);
	std::vector<PItem> m_items;
};

#endif
