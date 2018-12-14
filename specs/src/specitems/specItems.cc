#include "utils/ErrorReporting.h"
#include "processing/Config.h"
#include "specItems.h"

ALUCounters g_counters;
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
		case TokenListType__PAD:
		case TokenListType__READ:
		case TokenListType__READSTOP:
		case TokenListType__WRITE:
		{
			TokenItem *pItem = new TokenItem(tokenVec[index++]);
			addItem(pItem);
			break;
		}
			/* check here for other types of items, whether plainItem, ifGroup, whileGroup */
		case TokenListType__RANGELABEL:
		case TokenListType__RANGE:
		case TokenListType__WORDRANGE:
		case TokenListType__FIELDRANGE:
		case TokenListType__LITERAL:
		case TokenListType__SUBSTRING:
		case TokenListType__NUMBER:
		case TokenListType__TODCLOCK:
		case TokenListType__DTODCLOCK:
		case TokenListType__ID:
		case TokenListType__PRINT:
		{
			DataField *pItem = new DataField;
			pItem->parse(tokenVec, index);
			addItem(pItem);
			break;
		}
		case TokenListType__SET:
		{
			MYASSERT(index < tokenVec.size());
			try {
				SetItem* pItem = new SetItem(tokenVec[index].Literal());
				index++;
				addItem(pItem);
			} catch(const SpecsException& e) {
				if (g_bVerbose) {
					std::cerr << "While parsing statement, got: " << e.what(true) << "\n";
				}
				std::string err = "Error in statement in "+ tokenVec[index].HelpIdentify();
				MYTHROW(err);
			}
			break;
		}
		default:
			std::string err = std::string("Unhandled token type ")
				+ TokenListType__2str(tokenVec[index].Type())
				+ " at argument " + std::to_string(tokenVec[index].argIndex());
			MYTHROW(err);
		}
	}
}

std::string itemGroup::Debug()
{
	std::string ret = "itemGroup has " + std::to_string(m_items.size()) + " items:\n";
	int idx = 1;
	for (std::vector<PItem>::iterator it = m_items.begin(); it != m_items.end(); it++, idx++) {
		PItem pIt = *it;
		ret += std::to_string(idx) + ". ";
		ret += pIt->Debug();
		ret += "\n";
	}

	return ret;
}

bool itemGroup::processDo(StringBuilder& sb, ProcessingState& pState, Reader* pRd, Writer* pWr)
{
	bool bSomethingWasDone = false;
	int i;
	PSpecString ps; // Used for processing READ and READSTOP tokens
	bool processingContinue = true;
	for (i=0; processingContinue && i<m_items.size(); i++) {
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
		case ApplyRet__Read:
		case ApplyRet__ReadStop:
			ps = pRd->get();
			if (!ps) {
				if (aRet==ApplyRet__Read) {
					ps = SpecString::newString();
				} else {
					processingContinue = false; // Stop processing if no extra record is available
				}
			}
			pState.setString(ps);
			break;
		default:
			std::string err = "Unexpected return code from TokenItem::apply: ";
			err += std::to_string(aRet);
			MYTHROW(err);
		}
	}
	return bSomethingWasDone;
}

void itemGroup::process(StringBuilder& sb, ProcessingState& pState, Reader& rd, Writer& wr)
{
	PSpecString ps;

	while ((ps=rd.get())) {
		pState.setString(ps);

		try {
			if (processDo(sb,pState, &rd, &wr)) {
				wr.Write(sb.GetString());
			}
		} catch (const SpecsException& e) {
			std::cerr << "Exception processing line " << rd.countUsed() << ": " << e.what(true) << "\n";
		}
	}
}

bool itemGroup::readsLines()
{
	for (PItem pItem : m_items) {
		if (pItem->readsLines()) return true;
	}
	return false;
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
	case TokenListType__PAD:
		pState.setPadChar(mp_Token->Literal()[0]);
		return ApplyRet__Continue;
	case TokenListType__READ:
		return ApplyRet__Read;
	case TokenListType__READSTOP:
		return ApplyRet__ReadStop;
	case TokenListType__WRITE:
		return ApplyRet__Write;
	default:
		std::string err = "Unhandled TokenItem type " + TokenListType__2str(mp_Token->Type());
		MYTHROW(err);
	}
}


SetItem::SetItem(std::string& _statement)
{
	m_rawExpression = _statement;
	AluVec expr;
	MYASSERT(parseAluStatement(_statement, m_key, &m_oper, expr));
	MYASSERT(convertAluVecToPostfix(expr, m_RPNExpression, true));
}

SetItem::~SetItem()
{
	for (AluUnit* unit : m_RPNExpression) {
		delete unit;
	}
}

ApplyRet SetItem::apply(ProcessingState& pState, StringBuilder* pSB)
{
	ALUPerformAssignment(m_key, &m_oper, m_RPNExpression, &g_counters);
	return ApplyRet__Continue;
}

