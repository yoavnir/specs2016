#include "item.h"

#define GET_NEXT_TOKEN {token = tokenVec[index]; tokenType = token.Type(); index++; }

DataField::DataField()
{
	m_inputRange = NULL;
	m_label = '\0';
	m_outStart = LAST_POS_END;
	m_maxLength = LAST_POS_END;
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
		m_label = token.Literal()[1];
		GET_NEXT_TOKEN;
	}

	switch (tokenType) {
	case TokenListType__RANGE:
	case TokenListType__WORDRANGE:
	case TokenListType__FIELDRANGE:
	case TokenListType__LITERAL:
		m_inputRange = &token;
		break;
	default:
		std::string err = "Bad inputRange with token "+TokenListType__2str(tokenType)+" and contents <"+token.Orig()+">";
		throw std::invalid_argument(err);
	}

	/* insert code here to deal with strip and conversion */

	GET_NEXT_TOKEN;

	/* output placement */
	switch (tokenType) {
	case TokenListType__RANGE:
		if (!token.Range()->isSimpleRange()) {
			std::string err = "Bad output placement with token "+TokenListType__2str(tokenType)+" and contents <"+token.Orig()+">";
			throw std::invalid_argument(err);
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
		std::string err = "Bad output placement with token "+TokenListType__2str(tokenType)+" and contents <"+token.Orig()+">";
		throw std::invalid_argument(err);
	}
}

std::string DataField::Debug() {
	std::string ret = "{Source=";
	if (m_label) ret = ret + m_label + ':';
	ret += m_inputRange->Debug(0);
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
	return ret;
}
