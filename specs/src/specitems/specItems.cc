#include <assert.h>
#include "specItems.h"

itemGroup::itemGroup()
{
	m_items.clear();
}

void itemGroup::addItem(Item *pItem)
{
	m_items.insert(m_items.end(), pItem);
}

void itemGroup::parse(std::vector<Token> &tokenVec, unsigned int& index)
{
	while (index < tokenVec.size()) {
		/* check here for other types of items, whether plainItem, ifGroup, whileGroup */
		{
			DataField *pItem = new DataField;
			pItem->parse(tokenVec, index);
			addItem(pItem);
		}
	}
}

std::string itemGroup::Debug()
{
	std::string ret = "itemGroup has " + std::to_string(m_items.size()) + " items:\n";
	int idx = 1;
	for (std::vector<PItem>::iterator it = m_items.begin(); it != m_items.end(); it++, idx++) {
		ret += std::to_string(idx) + ". ";
		ret += (*it)->Debug();
		ret += "\n";
	}

	return ret;
}

void itemGroup::process(StringBuilder& sb, ProcessingState& pState, Reader& rd, Writer& wr)
{
	std::string *ps;
	bool bSomethingWasDone = false;
	while ((ps=rd.get())) {
		int i;
		pState.setString(ps);
		for (i=0; i<m_items.size(); i++) {
			PItem pit = m_items[i];
			ApplyRet aRet = pit->apply(pState, &sb);
			switch (aRet) {
			case ApplyRet__Continue:
				bSomethingWasDone = true;
				break;
			case ApplyRet__Write:
				if (bSomethingWasDone) {
					wr.Write(sb.GetString());
				} else {
					wr.Write(new std::string);
				}
				bSomethingWasDone = false;
				break;
			default:
				assert(2==1);
			}
		}
		if (bSomethingWasDone) wr.Write(sb.GetString());
	}
}
