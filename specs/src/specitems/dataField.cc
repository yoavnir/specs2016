#include "item.h"

#define GET_NEXT_TOKEN {token = tokenVec[index]; tokenType = token.Type(); index++; }

DataField::DataField()
{
	m_inputRange = NULL;
	m_label = '\0';
	m_outStart = LAST_POS_END;
	m_maxLength = LAST_POS_END;
	m_strip = false;
	m_conversion = StringConversion__identity;
}

DataField::~DataField() {

}

void DataField::parse(std::vector<Token> &tokenVec, unsigned int& index)
{
	Token& token = dummyToken;
	TokenListTypes tokenType;

	GET_NEXT_TOKEN;

	/* handle letter prefix for an input range */
	if (tokenType==TokenListType__RANGELABEL) {
		m_label = token.Literal()[0];
		GET_NEXT_TOKEN;
	}

	switch (tokenType) {
	case TokenListType__RANGE:
	case TokenListType__WORDRANGE:
	case TokenListType__FIELDRANGE:
	case TokenListType__LITERAL:
		m_inputRange = new Token(token);
		break;
	default:
		throw std::invalid_argument("Bad inputRange " + token.HelpIdentify());
	}

	/* insert code here to deal with strip and conversion */
	GET_NEXT_TOKEN;

	if (tokenType==TokenListType__STRIP) {
		m_strip = true;
		GET_NEXT_TOKEN;
	}

	if (tokenType==TokenListType__CONVERSION) {
		m_conversion = getConversionByName(token.Literal());
		assert(m_conversion!=StringConversion__identity);
		assert(m_conversion!=StringConversion__NONE);
		GET_NEXT_TOKEN;
	}

	/* output placement */
	switch (tokenType) {
	case TokenListType__RANGE:
		if (!token.Range()->isSimpleRange()) {
			throw std::invalid_argument("Bad output placement " + token.HelpIdentify());
		}
		if (token.Range()->isSingleNumber()) {
			m_outStart = token.Range()->getSingleNumber();
			m_maxLength = LAST_POS_END;
		} else {
			m_outStart = token.Range()->getSimpleFirst();
			m_maxLength = token.Range()->getSimpleLast() - m_outStart + 1;
		}
		break;
	case TokenListType__PERIOD:
		m_outStart = LAST_POS_END;
		break;
	default:
		throw std::invalid_argument("Bad output placement " + token.HelpIdentify());
	}
}

std::string DataField::Debug() {
	std::string ret = "{Source=";
	if (m_label) ret = ret + m_label + ':';
	ret += m_inputRange->Debug();
	/* conversion and stripping go here */
	ret += ";Dest=";
	if (m_outStart==LAST_POS_END) {
		ret += "None";
	} else {
		ret += std::to_string(m_outStart);
		if (m_maxLength!=LAST_POS_END) {
			ret += '-' + std::to_string(m_outStart+m_maxLength-1);
		}
	}

	if (m_strip) ret += " STRIP";
	if (m_conversion!=StringConversion__identity) ret += " " + StringConversion__2str(m_conversion);
	ret += '}';
	return ret;
}

ApplyRet DataField::apply(ProcessingState& pState, StringBuilder* pSB)
{
	int _from, _to;
	std::string* pInput;
	switch (m_inputRange->Type()) {
	case TokenListType__RANGE:
		_from = m_inputRange->Range()->getSimpleFirst();
		_to = m_inputRange->Range()->getSimpleLast();
		pInput = pState.getFromTo(_from, _to);
		break;
	case TokenListType__WORDRANGE:
		_from = pState.getWordStart(m_inputRange->Range()->getSimpleFirst());
		_to = pState.getWordEnd(m_inputRange->Range()->getSimpleLast());
		pInput = pState.getFromTo(_from, _to);
		break;
	case TokenListType__FIELDRANGE:
		_from = pState.getWordStart(m_inputRange->Range()->getSimpleFirst());
		_to = pState.getWordEnd(m_inputRange->Range()->getSimpleLast());
		pInput = pState.getFromTo(_from, _to);
		break;
	case TokenListType__LITERAL:
		pInput = new std::string(m_inputRange->Literal());
		break;
	default:
		assert(2==1);
	}

	if (!pInput) pInput = new std::string;

	// truncate or expand if necessary
	if (m_maxLength>0 && pInput->length()!=m_maxLength) {
		pInput->resize(m_maxLength);  // TODO: Add placement
	}

	pSB->insert(pInput, m_outStart);
	delete pInput;
	return ApplyRet__Continue;
}
