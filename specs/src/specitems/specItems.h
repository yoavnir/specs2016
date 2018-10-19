#ifndef SPECS2016__SPECITEMS__SPECITEMS__H
#define SPECS2016__SPECITEMS__SPECITEMS__H

#include <vector>
#include "item.h"
#include "../cli/tokens.h"
#include "../processing/StringBuilder.h"
#include "../processing/Reader.h"
#include "../processing/Writer.h"
#include "../processing/ProcessingState.h"

class itemGroup {
public:
	itemGroup();
	void parse(std::vector<Token> &tokenVec, unsigned int& index);
	void process(StringBuilder& sb, ProcessingState& pState, Reader& rd, Writer& wr);
	std::string Debug();
private:
	void addItem(PItem pItem);
	std::vector<PItem> m_items;
};

#endif
