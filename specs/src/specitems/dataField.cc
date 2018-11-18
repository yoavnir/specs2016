#include "utils/ErrorReporting.h"
#include "item.h"

#define GET_NEXT_TOKEN_NO_ADVANCE {           \
		if (index>=tokenVec.size()) {         \
			token = dummyToken;               \
			tokenType = TokenListType__DUMMY; \
		} else {                              \
			token = tokenVec[index];          \
			tokenType = token.Type();         \
		}                                     \
	}

#define GET_NEXT_TOKEN {                      \
	GET_NEXT_TOKEN_NO_ADVANCE;                \
	if (tokenType!=TokenListType__DUMMY) {    \
		index++;                              \
	}}

DataField::DataField()
{
	m_InputPart = NULL;
	m_label = '\0';
	m_outStart = LAST_POS_END;
	m_maxLength = LAST_POS_END;
	m_strip = false;
	m_conversion = StringConversion__identity;
	m_alignment = outputAlignmentLeft;
}

DataField::~DataField() {
	fprintf(stderr, "~DataField\n");
}

/*
 * Method: getSubstringPart
 *
 * Description: attempts to retrieve a substring part description from the stream of
 *              tokens.
 *
 * Parameters:
 *   - tokenVec - a vector of tokens
 *   - index - a running index of tokens within the vector
 *
 * Returns:
 *   - a pointer to a newly-allocated SubstringPart.
 *   - throws an exception if not valid
 *
 * Notes:
 *   - index is incremented as necessary past the tokens representing the input part.
 *   - index is not safe -- must verify it's not beyond the vector
 *   - Caller must determine if a NULL return is acceptable
 */

SubstringPart* DataField::getSubstringPart(std::vector<Token> &tokenVec, unsigned int& index)
{
	Token token = dummyToken;
	TokenListTypes tokenType;
	InputPart* _pSub;
	RangePart* pSub;
	InputPart* pBig;
	char       subPartWordSeparator = 0;
	char       subPartFieldSeparator = 0;

	// Check for fs or ws at the start of sub part
	GET_NEXT_TOKEN_NO_ADVANCE;

	switch (tokenType) {
	case TokenListType__WORDSEPARATOR:
		subPartWordSeparator = token.Literal()[0];
		index++;
		break;
	case TokenListType__FIELDSEPARATOR:
		subPartFieldSeparator = token.Literal()[0];
		index++;
		break;
	default:
		;
	}

	_pSub = getInputPart(tokenVec, index, subPartWordSeparator, subPartFieldSeparator);
	if (!_pSub) {
		std::string err = "Invalid input part following SUBSTRING token " + token.HelpIdentify();
		MYTHROW(err);
	}

	// sub must be of type RangePart
	pSub = dynamic_cast<RangePart*>(_pSub);
	if (!pSub) {
		std::string err = "Invalid range part following SUBSTRING token " + token.HelpIdentify();
		MYTHROW(err);
	}

	GET_NEXT_TOKEN; // next token MUST be OF

	if (tokenType!=TokenListType__OF) {
		std::string err = "Missing OF following SUBSTRING " + token.HelpIdentify();
		MYTHROW(err);
	}

	// Check for fs or ws at the start of big part
	GET_NEXT_TOKEN_NO_ADVANCE;

	switch (tokenType) {
	case TokenListType__WORDSEPARATOR:
		subPartWordSeparator = token.Literal()[0];
		index++;
		break;
	case TokenListType__FIELDSEPARATOR:
		subPartFieldSeparator = token.Literal()[0];
		index++;
		break;
	default:
		;
	}

	pBig = getInputPart(tokenVec, index, subPartWordSeparator, subPartFieldSeparator);
	if (!pBig) {
		GET_NEXT_TOKEN_NO_ADVANCE;
		std::string err = "Invalid big part following SUBSTRING-OF " + token.HelpIdentify();
		MYTHROW(err);
	}

	return new SubstringPart(pSub, pBig);
}

/*
 * Method: getInputPart
 *
 * Description: attempts to retrieve an input part description from the stream of
 *              tokens.
 *
 * Parameters:
 *   - tokenVec - a vector of tokens
 *   - index - a running index of tokens within the vector
 *
 * Returns:
 *   - a pointer to a newly-allocated InputPart.
 *   - NULL if the tokens do not represent an input part.
 *   - if returns NULL, index is NOT incremented
 *
 * Notes:
 *   - index is incremented as necessary past the tokens representing the input part.
 *   - index is not safe -- must verify it's not beyond the vector
 *   - Caller must determine if a NULL return is acceptable
 */
InputPart* DataField::getInputPart(std::vector<Token> &tokenVec, unsigned int& _index, char _wordSep, char _fieldSep)
{
	unsigned int index = _index;
	InputPart* ret;
	Token token = dummyToken;
	TokenListTypes tokenType;

	GET_NEXT_TOKEN;

	switch (tokenType) {
	case TokenListType__RANGE:
		ret = new RegularRangePart(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast());
		break;
	case TokenListType__WORDRANGE:
		ret = new WordRangePart(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast(), _wordSep);
		break;
	case TokenListType__FIELDRANGE:
		ret = new FieldRangePart(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast(), _fieldSep);
		break;
	case TokenListType__LITERAL:
		ret = new LiteralPart(token.Literal());
		break;
	case TokenListType__SUBSTRING:
		ret = getSubstringPart(tokenVec, index);
		break;
	case TokenListType__NUMBER:
		ret = new NumberPart();
		break;
	case TokenListType__TODCLOCK:
		ret = new ClockPart(ClockType__Static);
		break;
	case TokenListType__DTODCLOCK:
		ret = new ClockPart(ClockType__Dynamic);
		break;
	default:
		return NULL;
	}

	_index = index;
	return ret;
}

void DataField::parse(std::vector<Token> &tokenVec, unsigned int& index)
{
	Token token = dummyToken;
	TokenListTypes tokenType;

	GET_NEXT_TOKEN_NO_ADVANCE;

	/* handle letter prefix for an input range */
	if (tokenType==TokenListType__RANGELABEL) {
		m_label = token.Literal()[0];
		index++;
		GET_NEXT_TOKEN_NO_ADVANCE;
	}

	m_InputPart = getInputPart(tokenVec, index);

	if (!m_InputPart) {
		std::string err = "Bad inputRange " + token.HelpIdentify();
		MYTHROW(err);
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
	m_maxLength = LAST_POS_END;
	switch (tokenType) {
	case TokenListType__DUMMY:
		/* Run out of tokens?  Can't do that. We need an output placement */
		MYTHROW("Missing output placement at end of args");
		break;
	case TokenListType__RANGE:
		if (!token.Range()->isSimpleRange()) {
			std::string err = "Bad output placement " + token.HelpIdentify();
			MYTHROW(err);
		}
		if (token.Range()->isSingleNumber()) {
			m_outStart = token.Range()->getSingleNumber();
		} else {
			m_outStart = token.Range()->getSimpleFirst();
			m_maxLength = token.Range()->getSimpleLast() - m_outStart + 1;
		}
		break;
	case TokenListType__PERIOD:
		m_outStart = LAST_POS_END;
		break;
	case TokenListType__NEXTWORD:
		m_outStart = POS_SPECIAL_VALUE_NEXTWORD;
		if (token.Range()) {
			m_maxLength = (size_t)(token.Range()->getSimpleLast());
		}
		break;
	case TokenListType__NEXTFIELD:
		m_outStart = POS_SPECIAL_VALUE_NEXTFIELD;
		if (token.Range()) {
			m_maxLength = (size_t)(token.Range()->getSimpleLast());
		}
		break;
	case TokenListType__NEXT:
		m_outStart = POS_SPECIAL_VALUE_NEXT;
		if (token.Range()) {
			m_maxLength = (size_t)(token.Range()->getSimpleLast());
		}
		break;
	default:
		std::string err = "Bad output placement " + token.HelpIdentify();
		MYTHROW(err);
	}

	GET_NEXT_TOKEN_NO_ADVANCE;
	switch (tokenType) {
	case TokenListType__LEFT:
		index++;
		break;
	case TokenListType__RIGHT:
		m_alignment = outputAlignmentRight;
		index++;
		break;
	case TokenListType__CENTER:
		m_alignment = outputAlignmentCenter;
		index++;
		break;
	default:
		;
	}
}

std::string DataField::Debug() {
	std::string ret = "{Source=";
	if (m_label) ret = ret + m_label + ':';
	ret += m_InputPart->Debug();
	/* conversion and stripping go here */
	ret += ";Dest=";
	if (m_outStart==LAST_POS_END) {
		ret += "None";
	} else {
		if (m_outStart==POS_SPECIAL_VALUE_NEXT) {
			ret += "Next";
		} else if (m_outStart==POS_SPECIAL_VALUE_NEXTWORD) {
			ret += "NextWord";
		} else if (m_outStart==POS_SPECIAL_VALUE_NEXTFIELD) {
			ret += "NextField";
		} else {
			ret += std::to_string(m_outStart);
		}
		if (m_maxLength!=LAST_POS_END) {
			ret += '.' + std::to_string(m_maxLength);
		}
	}

	switch (m_alignment) {
	case outputAlignmentCenter: ret += " (centered)"; break;
	case outputAlignmentRight:  ret += " (right)"; break;
	default: ;
	}

	if (m_strip) ret += " STRIP";
	if (m_conversion!=StringConversion__identity) ret += " " + StringConversion__2str(m_conversion);
	ret += '}';
	return ret;
}

#define IS_BLANK(c) (c==0x20 || c==0x09 || c==0x0a || c==0x0d)
void DataField::stripString(PSpecString &pOrig)
{
	PSpecString old = pOrig;
	const char* s = pOrig->data();
	size_t len = pOrig->length();
	while (IS_BLANK(*s) && len>0) {
		s++; len--;
	}

	if (!len) {
		delete old;
		pOrig = SpecString::newString();
	}

	const char* sEnd = s + len - 1;
	while (IS_BLANK(*sEnd)) {
		sEnd--;
		len--;
	}

	pOrig = SpecString::newString(s, len);
	delete old;
}

ApplyRet DataField::apply(ProcessingState& pState, StringBuilder* pSB)
{
	int _from, _to;
	PSpecString pInput = m_InputPart->getStr(pState);

	if (!pInput) pInput = SpecString::newString();

	if (m_strip) {
		stripString(pInput);
	}

	if (m_conversion) {
		std::string currentString(pInput->data(), pInput->length());
		std::string convertedString = stringConvert(currentString, m_conversion);
		delete pInput;
		pInput = SpecString::newString(convertedString);
	}

	// truncate or expand if necessary
	if (m_maxLength>0 && pInput->length()!=m_maxLength) {
		pInput->Resize(m_maxLength, pSB->getPad(), m_alignment);
	}

	if (m_outStart==POS_SPECIAL_VALUE_NEXT) {
		pSB->insertNext(pInput);
	} else if (m_outStart==POS_SPECIAL_VALUE_NEXTWORD) {
		pSB->insertNextWord(pInput);
	} else if (m_outStart==POS_SPECIAL_VALUE_NEXTFIELD) {
		pSB->insertNextField(pInput);
	} else if (m_outStart <= MAX_OUTPUT_POSITION) {
		pSB->insert(pInput, m_outStart);
	} else {
		std::string err = std::string("Output position too great: ") + std::to_string(m_outStart);
		MYTHROW(err);
	}
	delete pInput;
	return ApplyRet__Continue;
}
