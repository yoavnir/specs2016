#include <assert.h>
#include "utils/ErrorReporting.h"
#include "specItems.h"

itemGroup::itemGroup()
{
	m_items.clear();
}

void itemGroup::addItem(Item *pItem)
{
	m_items.insert(m_items.end(), pItem);
}

void itemGroup::Compile(std::vector<Token> &tokenVec, unsigned int& index)
{
	while (index < tokenVec.size()) {
		switch (tokenVec[index].Type()) {
		case TokenListType__FIELDSEPARATOR:
		case TokenListType__WORDSEPARATOR:
		{
			TokenItem *pItem = new TokenItem(tokenVec[index++]);
			addItem(pItem);
			break;
		}
			/* check here for other types of items, whether plainItem, ifGroup, whileGroup */
		case TokenListType__RANGE:
		case TokenListType__WORDRANGE:
		case TokenListType__FIELDRANGE:
		case TokenListType__LITERAL:
		case TokenListType__SUBSTRING:
		case TokenListType__NUMBER:
		{
			DataField *pItem = new DataField;
			pItem->parse(tokenVec, index);
			addItem(pItem);
			break;
		}
		default:
			std::string err = std::string("Unhandled token type ") + TokenListType__2str(tokenVec[index].Type());
			MYTHROW(err);
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

bool itemGroup::processDo(StringBuilder& sb, ProcessingState& pState, Reader* pRd, Writer* pWr)
{
	bool bSomethingWasDone = false;
	int i;
	for (i=0; i<m_items.size(); i++) {
		PItem pit = m_items[i];
		ApplyRet aRet = pit->apply(pState, &sb);
		switch (aRet) {
		case ApplyRet__Continue:
			bSomethingWasDone = true;
			break;
		case ApplyRet__Write:
			if (bSomethingWasDone) {
				pWr->Write(sb.GetString());
			} else {
				pWr->Write(SpecString::newString());
			}
			bSomethingWasDone = false;
			break;
		default:
			assert(2==1);
		}
	}
	return bSomethingWasDone;
}

void itemGroup::process(StringBuilder& sb, ProcessingState& pState, Reader& rd, Writer& wr)
{
	PSpecString ps;

	while ((ps=rd.get())) {
		pState.setString(ps);

		if (processDo(sb,pState, &rd, &wr)) {
			wr.Write(sb.GetString());
		}
	}
}

TokenItem::TokenItem(Token& t)
{
	mp_Token = new Token(t);
}

TokenItem::~TokenItem()
{
	if (mp_Token) {
		delete mp_Token;
		mp_Token = NULL;
	}
}

std::string TokenItem::Debug()
{
	return mp_Token->Debug();
}

ApplyRet TokenItem::apply(ProcessingState& pState, StringBuilder* pSB)
{
	switch (mp_Token->Type()) {
	case TokenListType__FIELDSEPARATOR:
		pState.setFSChar(mp_Token->Literal()[0]);
		return ApplyRet__Continue;
	case TokenListType__WORDSEPARATOR:
		pState.setWSChar(mp_Token->Literal()[0]);
		return ApplyRet__Continue;
	default:
		std::string err = "Unhandled TokenItem type " + TokenListType__2str(mp_Token->Type());
		MYTHROW(err);
	}
}
