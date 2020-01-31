#ifndef SPECS2016__SPECITEMS__SPECITEMS__H
#define SPECS2016__SPECITEMS__SPECITEMS__H

#include <vector>
#include "item.h"
#include "cli/tokens.h"
#include "processing/StringBuilder.h"
#include "processing/Reader.h"
#include "processing/Writer.h"
#include "processing/ProcessingState.h"

#define MAX_DEPTH_CONDITION_STATEMENTS  64

class itemGroup {
public:
	itemGroup();
	~itemGroup();
	void Compile(std::vector<Token> &tokenVec, unsigned int& index);
	bool processDo(StringBuilder& sb, ProcessingState& pState, Reader* pRd, classifyingTimer& tmr, unsigned int& rdrCounter);
	void process(StringBuilder& sb, ProcessingState& pState, Reader& rd, classifyingTimer& tmr);
	void setRegularRunAtEOF()  { bFoundSelectSecond = true; bNeedRunoutCycle = true; }
	bool needRunoutCycle()     { return bNeedRunoutCycle;   }
	std::string Debug();
	bool readsLines();
private:
	bool bNeedRunoutCycle;
	bool bNeedRunoutCycleFromStart;
	bool bFoundSelectSecond;
	void addItem(PItem pItem);
	std::vector<PItem> m_items;
};

#endif
