#include "utils/platform.h"
#include "utils/ErrorReporting.h"
#include "processing/Config.h"
#include "item.h"

extern ALUCounters g_counters;

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

#define REUSE_CURRENT_TOKEN {index--;}

DataField::DataField()
{
	m_InputPart = nullptr;
	m_label = m_tailLabel = '\0';
	m_outStart = LAST_POS_END;
	m_maxLength = LAST_POS_END;
	m_strip = false;
	m_conversion = StringConversion__identity;
	m_alignment = outputAlignmentLeft;
	m_conversionParam = "";
}

DataField::~DataField() {
	m_InputPart = nullptr;
	cleanAluVec(m_outputStartExpression);
	cleanAluVec(m_outputWidthExpression);
	cleanAluVec(m_outputAlignmentExpression);
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

PSubstringPart DataField::getSubstringPart(std::vector<Token> &tokenVec, unsigned int& index)
{
	Token token = dummyToken;
	TokenListTypes tokenType;
	PPart      _pSub;
	PRangePart pSub;
	PPart      pBig;
	std::string subPartWordSeparator;
	std::string subPartFieldSeparator;

	// Check for fs or ws at the start of sub part
	GET_NEXT_TOKEN_NO_ADVANCE;

	switch (tokenType) {
	case TokenListType__WORDSEPARATOR:
		subPartWordSeparator = token.Literal();
		index++;
		break;
	case TokenListType__FIELDSEPARATOR:
		subPartFieldSeparator = token.Literal();
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
	pSub = std::dynamic_pointer_cast<RangePart>(_pSub);
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

	return std::make_shared<SubstringPart>(pSub, pBig);
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
PPart DataField::getInputPart(std::vector<Token> &tokenVec, unsigned int& _index, const std::string& _wordSep, const std::string& _fieldSep)
{
	unsigned int index = _index;
	PPart ret;
	Token token = dummyToken;
	TokenListTypes tokenType;

	GET_NEXT_TOKEN;

	switch (tokenType) {
	case TokenListType__RANGE:
		ret = std::make_shared<RegularRangePart>(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast());
		break;
	case TokenListType__WORDRANGE:
		ret = std::make_shared<WordRangePart>(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast(), _wordSep);
		break;
	case TokenListType__FIELDRANGE:
		ret = std::make_shared<FieldRangePart>(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast(), _fieldSep);
		break;
	case TokenListType__LITERAL:
		ret = std::make_shared<LiteralPart>(token.Literal());
		break;
	case TokenListType__SUBSTRING:
		ret = getSubstringPart(tokenVec, index);
		break;
	case TokenListType__NUMBER:
		ret = std::make_shared<NumberPart>();
		break;
	case TokenListType__TODCLOCK:
		ret = std::make_shared<ClockPart>(ClockType__Static);
		break;
	case TokenListType__DTODCLOCK:
		ret = std::make_shared<ClockPart>(ClockType__Dynamic);
		break;
	case TokenListType__TIMEDIFF:
		ret = std::make_shared<ClockPart>(ClockType__Diff);
		break;
	case TokenListType__ID:
		ret = std::make_shared<IDPart>(token.Literal());
		break;
	case TokenListType__PRINT:
	{
		try {
			ret = std::make_shared<ExpressionPart>(token.Literal());
		} catch(const SpecsException& e) {
			std::string err = "Expression in "+ token.HelpIdentify()
					+ ":\n" + e.what(true /* concise */);
			MYTHROW(err);
		}
		break;
	}
	default:
		return nullptr;
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
		std::string err = "Bad input part " + token.HelpIdentify();
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
		MYASSERT(m_conversion!=StringConversion__identity);
		MYASSERT(m_conversion!=StringConversion__NONE);
		GET_NEXT_TOKEN;
		if (isParametrizedConversion(m_conversion)) {
			if (tokenType!=TokenListType__LITERAL) {
				std::string err = "Bad time format " + token.HelpIdentify();
				MYTHROW(err);
			}
			m_conversionParam = token.Literal();
			GET_NEXT_TOKEN;
		}
	}

	/* output placement */
	m_maxLength = LAST_POS_END;
	switch (tokenType) {
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
	case TokenListType__RANGELABEL:
		m_outStart = LAST_POS_END;
		m_tailLabel = token.Literal()[0];
		break;
	case TokenListType__IF:
	case TokenListType__ENDIF:
	case TokenListType__ELSE:
	case TokenListType__ELSEIF:
	case TokenListType__WHILE:
	case TokenListType__DONE:
	case TokenListType__WRITE:
	case TokenListType__READ:
	case TokenListType__READSTOP:
	case TokenListType__EOF:
		/* This is a control structure?  Assume NEXTWORD and re-use this one */
		REUSE_CURRENT_TOKEN;
	case TokenListType__DUMMY:
		/* Run out of tokens?  Assume they meant to do the same as NEXTWORD */
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
	case TokenListType__GROUPSTART:
	{
		std::string composedOutputPlacement = "";
		GET_NEXT_TOKEN;
		while (token.Type()!=TokenListType__GROUPEND && token.Type()!=TokenListType__DUMMY) {
			composedOutputPlacement += token.Literal();
			GET_NEXT_TOKEN;
		}
		MYASSERT_WITH_MSG(token.Type()==TokenListType__GROUPEND, "Unterminated composed output placement");
		// index++;
		interpretComposedOutputPlacement(composedOutputPlacement);
		return;
	}
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

void DataField::interpretComposedOutputPlacement(std::string& outputPlacement)
{
	AluVec outputPlacementExpression;
	AluVec infixExpression;

	m_outStart = POS_SPECIAL_VALUE_COMPOSED;
	m_maxLength = 0;
	m_alignment = outputAlignmentLeft;

	MYASSERT(parseAluExpression(outputPlacement, outputPlacementExpression));
	MYASSERT(breakAluVecByComma(outputPlacementExpression, infixExpression));
	MYASSERT(convertAluVecToPostfix(infixExpression, m_outputStartExpression, true));

	if (outputPlacementExpression.empty()) return;

	m_maxLength = POS_SPECIAL_VALUE_COMPOSED;
	MYASSERT(breakAluVecByComma(outputPlacementExpression, infixExpression));
	MYASSERT(convertAluVecToPostfix(infixExpression, m_outputWidthExpression, true));

	if (outputPlacementExpression.empty()) return;

	m_alignment = outputAlignmentComposed;
	MYASSERT(breakAluVecByComma(outputPlacementExpression, infixExpression));
	MYASSERT(convertAluVecToPostfix(infixExpression, m_outputAlignmentExpression, true));

	MYASSERT(outputPlacementExpression.empty());
}

std::string DataField::Debug() {
	std::string ret = "{Source=";
	if (m_label) ret = ret + m_label + ':';
	ret += m_InputPart->Debug();
	/* conversion and stripping go here */
	ret += ";Dest=";
	if (m_outStart==LAST_POS_END) {
		if (m_tailLabel) {
			ret = ret + m_tailLabel + ":";
		} else {
			ret += "None";
		}
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
		pOrig = std::make_shared<std::string>();
	}

	const char* sEnd = s + len - 1;
	while (IS_BLANK(*sEnd)) {
		sEnd--;
		len--;
	}

	pOrig = std::make_shared<std::string>(s, len);
}

ApplyRet DataField::apply(ProcessingState& pState, StringBuilder* pSB)
{
	bool bWritingWasDone = false;
	PValue pComposedStartingPosition = nullptr;
	PSpecString pInput = m_InputPart->getStr(pState);
	size_t outputWidth = m_maxLength;

	if (!pInput) pInput = std::make_shared<std::string>();

	if (m_label) {
		try {
			pState.fieldIdentifierSet(m_label, pInput);
		} catch (std::out_of_range& e) {
			throw e;
		}
	}

	if (m_strip) {
		stripString(pInput);
	}

	if (m_conversion) {
		std::string currentString(pInput->data(), pInput->length());
		std::string convertedString = stringConvert(currentString, m_conversion, m_conversionParam);
		pInput = std::make_shared<std::string>(convertedString);
	}

	// truncate or expand if necessary

	if (outputWidth==POS_SPECIAL_VALUE_COMPOSED) {
		PValue res;
		if (m_outputWidthExpression.size() > 0) {
			res = evaluateExpression(m_outputWidthExpression, &g_counters);
		} else if (m_outputStartExpression.size() > 0) {
			static std::string sName("cols");
			static std::string sCols = configSpecLiteralGet(sName);
			static ALUInt cols = std::stoul(sCols);
			pComposedStartingPosition = evaluateExpression(m_outputStartExpression, &g_counters);
			ALUInt start = pComposedStartingPosition->getInt();
			if (cols >= start) {
				res = mkValue(cols - start + 1);
			} else {
				std::string err = "Composed starting position (" + std::to_string(start) +
						") is beyond screen width (" + std::to_string(cols) + ")";
				MYTHROW(err);
			}
		} else {
			/* when both are omitted, the width is the rest of the line */
			res = AluFunc_rest();
		}

		outputWidth = res->getInt();
	} else if (outputWidth > MAX_OUTPUT_POSITION) {
		MYTHROW("Excessive output width");
	}

	if (outputWidth>0 && pInput->length()!=outputWidth) {
		if (m_alignment != outputAlignmentComposed) {
			SpecString_Resize(pInput, outputWidth, pState.getPadChar(), m_alignment, ellipsisSpecNone);
		} else {
			outputAlignment al = outputAlignmentLeft;
			ellipsisSpec es = ellipsisSpecNone;
			PValue res = evaluateExpression(m_outputAlignmentExpression, &g_counters);
			std::string s = res->getStr();

			if (s[0]=='c' || s[0]=='C') {
				al = outputAlignmentCenter;
			} else if (s[0]=='r' || s[0]=='R') {
				al = outputAlignmentRight;
			}

			if (s.length() == 2) {
				switch (s[1]) {
				case '1':
					es = ellipsisSpecLeft;
					break;
				case '2':
					es = ellipsisSpecThird;
					break;
				case '3':
					es = ellipsisSpecHalf;
					break;
				case '4':
					es = ellipsisSpecTwoThirds;
					break;
				case '5':
					es = ellipsisSpecRight;
					break;
				default:
					break;
				}
			}

			SpecString_Resize(pInput, outputWidth, pState.getPadChar(), al, es);
		}
	}

	if (m_outStart==0 && outputWidth==0) {
		// This is the no output option
		if (m_tailLabel) {
			pState.fieldIdentifierSet(m_tailLabel, pInput);
		}
		goto FINISH;
	}

	pSB->setPadChar(pState.getPadChar());

	if (m_outStart==POS_SPECIAL_VALUE_NEXT) {
		pSB->insertNext(pInput);
	} else if (m_outStart==POS_SPECIAL_VALUE_NEXTWORD) {
		pSB->insertNextWord(pInput);
	} else if (m_outStart==POS_SPECIAL_VALUE_NEXTFIELD) {
		pSB->insertNextField(pInput);
	} else if (m_outStart==POS_SPECIAL_VALUE_COMPOSED) {
		PValue res;
		if (m_outputStartExpression.size() > 0) {
			if (nullptr == pComposedStartingPosition) {
				pComposedStartingPosition = evaluateExpression(m_outputStartExpression, &g_counters);
			}
			res = mkValue(pComposedStartingPosition->getInt());
		} else {
			res = AluFunc_next();
		}
		pSB->insert(pInput, res->getInt());
	} else if (m_outStart <= MAX_OUTPUT_POSITION) {
		pSB->insert(pInput, m_outStart);
	} else {
		std::string err = std::string("Output position too great: ") + std::to_string(m_outStart);
		MYTHROW(err);
	}

	bWritingWasDone = true;

FINISH:
	return bWritingWasDone ? ApplyRet__ContinueWithDataWritten : ApplyRet__Continue;
}

bool DataField::readsLines()
{
	return m_InputPart->readsLines();
}
