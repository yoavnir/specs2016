#ifndef SPECS2016__SPECITEMS__SPECITEMS__H
#define SPECS2016__SPECITEMS__SPECITEMS__H

#include <vector>
#include "item.h"
#include "../cli/tokens.h"

class itemGroup {
public:
	itemGroup();
	void parse(std::vector<Token> &tokenVec, unsigned int& index);
	void Debug();
private:
	void addItem(PItem pItem);
	std::vector<PItem> m_items;
};

#endif
