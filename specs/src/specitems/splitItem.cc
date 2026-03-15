#include "utils/platform.h"
#include "processing/Config.h"
#include "utils/ErrorReporting.h"
#include "item.h"
#include "utils/alu.h"

SplitItem::SplitItem(bool isField)
{
	m_isField = isField;
	m_separator = "";
	m_ofInputPart = nullptr;
	m_outStart = 0;
	m_maxLength = 0;
	m_alignment = outputAlignmentLeft;
	m_splitting = false;
	m_currentPiece = 0;
	m_savedSBPos = 0;
}

SplitItem::~SplitItem()
{
	m_pieces.clear();
	m_savedPrefix = nullptr;
	m_ofInputPart = nullptr;
}

std::string SplitItem::Debug()
{
	std::string ret = m_isField ? "SPLITF" : "SPLITW";
	if (!m_separator.empty()) {
		ret += " sep=/" + m_separator + "/";
	}
	if (m_ofInputPart) {
		ret += " OF " + m_ofInputPart->Debug();
	}
	ret += " -> " + debugOutputPlacement(m_outStart, m_maxLength, m_alignment);
	return ret;
}

void SplitItem::interpretComposedOutputPlacement(std::string& outputPlacement)
{
	// Parse composed output placement like "1.5" or "1.5 left"
	// This is copied from DataField::interpretComposedOutputPlacement
	size_t dotPos = outputPlacement.find('.');
	if (dotPos == std::string::npos) {
		std::string err = "Invalid composed output placement: " + outputPlacement;
		MYTHROW(err);
	}
	
	std::string startStr = outputPlacement.substr(0, dotPos);
	std::string widthStr = outputPlacement.substr(dotPos + 1);
	
	// Parse alignment if present
	size_t spacePos = widthStr.find(' ');
	std::string alignStr;
	if (spacePos != std::string::npos) {
		alignStr = widthStr.substr(spacePos + 1);
		widthStr = widthStr.substr(0, spacePos);
	}
	
	// Build expressions for start and width
	MYASSERT(parseAluExpression(startStr, m_outputStartExpression));
	MYASSERT(parseAluExpression(widthStr, m_outputWidthExpression));
	
	if (!alignStr.empty()) {
		MYASSERT(parseAluExpression(alignStr, m_outputAlignmentExpression));
	}
	
	m_outStart = POS_SPECIAL_VALUE_COMPOSED;
}

void SplitItem::parseOutputPlacement(std::vector<Token> &tokenVec, unsigned int& index)
{
	// If at end of tokens, default to NEXTWORD (final output placement)
	if (index >= tokenVec.size()) {
		m_outStart = POS_SPECIAL_VALUE_NEXTWORD;
		return;
	}
	
	Token& outTok = tokenVec[index];
	
	switch (outTok.Type()) {
	case TokenListType__RANGE:
	{
		PTokenFieldRange pRange = outTok.Range();
		if (!pRange->isSimpleRange()) {
			std::string err = "Bad output placement " + outTok.HelpIdentify();
			MYTHROW(err);
		}
		if (pRange->isSingleNumber()) {
			m_outStart = pRange->getSingleNumber();
		} else {
			m_outStart = pRange->getSimpleFirst();
			m_maxLength = pRange->getSimpleLast() - m_outStart + 1;
		}
		index++;
		break;
	}
	case TokenListType__NEXTWORD:
		m_outStart = POS_SPECIAL_VALUE_NEXTWORD;
		index++;
		break;
	case TokenListType__NEXTFIELD:
		m_outStart = POS_SPECIAL_VALUE_NEXTFIELD;
		index++;
		break;
	case TokenListType__NEXT:
		m_outStart = POS_SPECIAL_VALUE_NEXT;
		index++;
		break;
	case TokenListType__LITERAL:
	{
		std::string lit = outTok.Literal();
		if (lit.find('.') != std::string::npos) {
			interpretComposedOutputPlacement(lit);
			index++;
		} else {
			std::string err = "Bad output placement Token " + TokenListType__2str(outTok.Type()) + 
				" at index " + std::to_string(outTok.argIndex()) + " with content <" + outTok.Orig() + ">";
			MYTHROW(err);
		}
		break;
	}
	// Control structures and other spec units: assume NEXTWORD, don't consume
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
	case TokenListType__REDO:
	case TokenListType__FIELDSEPARATOR:
	case TokenListType__WORDSEPARATOR:
	case TokenListType__OUTSTREAM:
	case TokenListType__BREAK:
	case TokenListType__SELECT:
	case TokenListType__ASSERT:
	case TokenListType__ABEND:
		m_outStart = POS_SPECIAL_VALUE_NEXTWORD;
		break;
	default:
		std::string err = "Bad output placement Token " + TokenListType__2str(outTok.Type()) + 
			" at index " + std::to_string(outTok.argIndex()) + " with content <" + outTok.Orig() + ">";
		MYTHROW(err);
	}
	
	// Check for optional width and alignment (like DataField)
	if (index < tokenVec.size() && TokenListType__PERIOD == tokenVec[index].Type()) {
		index++;
		if (index >= tokenVec.size()) {
			std::string err = "Period must be followed by width specification";
			MYTHROW(err);
		}
		Token& widthTok = tokenVec[index];
		if (TokenListType__RANGE == widthTok.Type()) {
			PTokenFieldRange pRange = widthTok.Range();
			if (!pRange->isSingleNumber()) {
				std::string err = "Width must be a single number";
				MYTHROW(err);
			}
			m_maxLength = pRange->getSingleNumber();
			index++;
		} else {
			std::string err = "Period must be followed by width specification";
			MYTHROW(err);
		}
		
		// Check for alignment
		if (index < tokenVec.size()) {
			Token& alignTok = tokenVec[index];
			switch (alignTok.Type()) {
			case TokenListType__LEFT:
				m_alignment = outputAlignmentLeft;
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
				break;
			}
		}
	}
}

PPart SplitItem::getInputPart(std::vector<Token> &tokenVec, unsigned int& _index)
{
	if (_index >= tokenVec.size()) return nullptr;
	
	unsigned int index = _index;
	Token& token = tokenVec[index];
	PPart ret;
	
	switch (token.Type()) {
	case TokenListType__RANGE:
		// A plain range in SPLITW/SPLITF OF context is a character range
		ret = std::make_shared<RegularRangePart>(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast());
		index++;
		break;
	case TokenListType__WORDRANGE:
		ret = std::make_shared<WordRangePart>(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast(), std::string(""));
		index++;
		break;
	case TokenListType__FIELDRANGE:
		ret = std::make_shared<FieldRangePart>(token.Range()->getSimpleFirst(), token.Range()->getSimpleLast(), std::string(""));
		index++;
		break;
	case TokenListType__LITERAL:
		ret = std::make_shared<LiteralPart>(token.Literal());
		index++;
		break;
	case TokenListType__NUMBER:
		ret = std::make_shared<NumberPart>();
		index++;
		break;
	default:
		return nullptr;
	}
	
	_index = index;
	return ret;
}

void SplitItem::parse(std::vector<Token> &tokenVec, unsigned int& index)
{
	// The SPLITW/SPLITF token has already been consumed
	// The normalization has already extracted separator into the literal
	Token& splitTok = tokenVec[index-1];
	
	m_separator = splitTok.Literal();
	
	// Check for optional OF clause (parsed here, not during normalization,
	// to support full InputPart types: char ranges, word ranges, field ranges)
	if (index < tokenVec.size() && tokenVec[index].Type() == TokenListType__OF) {
		index++; // consume OF
		m_ofInputPart = getInputPart(tokenVec, index);
		if (!m_ofInputPart) {
			std::string err = "OF must be followed by an input part";
			MYTHROW(err);
		}
	}
	
	// Parse output placement
	parseOutputPlacement(tokenVec, index);
}

ApplyRet SplitItem::apply(ProcessingState& pState, StringBuilder* pSB)
{
	if (!m_splitting) {
		// First call - perform the split
		m_pieces.clear();
		m_currentPiece = 0;
		
		// Determine the string to split:
		// If OF clause is present, resolve it to get the input string
		// Otherwise, use the full current input record
		PSpecString inputStr;
		if (m_ofInputPart) {
			inputStr = m_ofInputPart->getStr(pState);
		}
		
		// Set up a ProcessingState for splitting
		// If we have an OF input, create a temp state for the resolved string
		// Otherwise, use the current pState directly
		ProcessingState* pSplitState;
		ProcessingState tempState;
		if (m_ofInputPart) {
			if (!inputStr || inputStr->empty()) {
				return ApplyRet__Continue;
			}
			tempState.setString(inputStr);
			tempState.setFSChars(m_separator.empty() ? pState.getFSChars() : m_separator);
			tempState.setWSChars(m_separator.empty() ? pState.getWSChars() : m_separator);
			pSplitState = &tempState;
		} else {
			pSplitState = &pState;
		}
		
		// Save current separator and set override if specified
		std::string savedSeparator;
		if (!m_separator.empty() && !m_ofInputPart) {
			if (m_isField) {
				savedSeparator = pState.getFieldSeparator();
				pState.alterFieldSeparator(m_separator);
			} else {
				savedSeparator = pState.getWordSeparator();
				pState.alterWordSeparator(m_separator);
			}
		}
		
		// Extract all pieces
		int count = m_isField ? pSplitState->getFieldCount() : pSplitState->getWordCount();
		
		for (int i = 1; i <= count; i++) {
			int start, end;
			if (m_isField) {
				start = pSplitState->getFieldStart(i);
				end = pSplitState->getFieldEnd(i);
			} else {
				start = pSplitState->getWordStart(i);
				end = pSplitState->getWordEnd(i);
			}
			PSpecString piece = pSplitState->getFromTo(start, end);
			if (piece) {
				m_pieces.push_back(piece);
			} else {
				// Empty field - push empty string
				m_pieces.push_back(std::make_shared<std::string>());
			}
		}
		
		// Restore separator if we changed it
		if (!m_separator.empty() && !m_ofInputPart) {
			if (m_isField) {
				pState.alterFieldSeparator(savedSeparator);
			} else {
				pState.alterWordSeparator(savedSeparator);
			}
		}
		
		if (m_pieces.empty()) {
			return ApplyRet__Continue;
		}
		
		// Save the current StringBuilder state as the prefix (non-destructive)
		PSpecString currentStr = pSB->PeekString();
		if (currentStr) {
			m_savedPrefix = std::make_shared<std::string>(*currentStr);
		} else {
			m_savedPrefix = nullptr;
		}
		m_savedSBPos = pSB->getPosition();
		
		// Place the first piece
		PSpecString firstPiece = m_pieces[0];
		
		// Ensure StringBuilder has pad character set
		pSB->setPadChar(pState.getPadChar());
		
		// Handle output placement
		if (m_outStart == POS_SPECIAL_VALUE_COMPOSED) {
			// Evaluate composed output placement
			PValue startVal = evaluateExpression(m_outputStartExpression, nullptr);
			PValue widthVal = evaluateExpression(m_outputWidthExpression, nullptr);
			m_outStart = startVal->getInt();
			m_maxLength = widthVal->getInt();
			
			if (!m_outputAlignmentExpression.empty()) {
				PValue alignVal = evaluateExpression(m_outputAlignmentExpression, nullptr);
				std::string alignStr = alignVal->getStr();
				if (alignStr == "left") m_alignment = outputAlignmentLeft;
				else if (alignStr == "right") m_alignment = outputAlignmentRight;
				else if (alignStr == "center" || alignStr == "centre") m_alignment = outputAlignmentCenter;
			}
		}
		
		// Apply width and alignment if specified
		if (m_maxLength > 0) {
			SpecString_Resize(firstPiece, m_maxLength, pState.getPadChar(), m_alignment, ellipsisSpecNone);
		}
		
		// Insert at the appropriate position
		if (m_outStart == POS_SPECIAL_VALUE_NEXTWORD) {
			pSB->insertNextWord(firstPiece);
		} else if (m_outStart == POS_SPECIAL_VALUE_NEXTFIELD) {
			pSB->insertNextField(firstPiece);
		} else if (m_outStart == POS_SPECIAL_VALUE_NEXT) {
			pSB->insertNext(firstPiece);
		} else if (m_outStart > 0) {
			pSB->insert(firstPiece, m_outStart, true);
		}
		
		m_splitting = true;
		m_currentPiece = 0;
		
		return ApplyRet__SplitStart;
	} else {
		// Subsequent call - place current piece
		if (m_currentPiece >= m_pieces.size()) {
			return ApplyRet__Continue;
		}
		
		PSpecString piece = m_pieces[m_currentPiece];
		
		// Ensure StringBuilder has pad character set
		pSB->setPadChar(pState.getPadChar());
		
		// Handle output placement for composed expressions
		if (m_outStart == POS_SPECIAL_VALUE_COMPOSED) {
			PValue startVal = evaluateExpression(m_outputStartExpression, nullptr);
			PValue widthVal = evaluateExpression(m_outputWidthExpression, nullptr);
			size_t outStart = startVal->getInt();
			size_t maxLength = widthVal->getInt();
			
			outputAlignment align = m_alignment;
			if (!m_outputAlignmentExpression.empty()) {
				PValue alignVal = evaluateExpression(m_outputAlignmentExpression, nullptr);
				std::string alignStr = alignVal->getStr();
				if (alignStr == "left") align = outputAlignmentLeft;
				else if (alignStr == "right") align = outputAlignmentRight;
				else if (alignStr == "center" || alignStr == "centre") align = outputAlignmentCenter;
			}
			
			if (maxLength > 0) {
				SpecString_Resize(piece, maxLength, pState.getPadChar(), align, ellipsisSpecNone);
			}
			
			if (outStart > 0) {
				pSB->insert(piece, outStart, true);
			}
		} else {
			// Apply width and alignment if specified
			if (m_maxLength > 0) {
				SpecString_Resize(piece, m_maxLength, pState.getPadChar(), m_alignment, ellipsisSpecNone);
			}
			
			// Insert at the appropriate position
			if (m_outStart == POS_SPECIAL_VALUE_NEXTWORD) {
				pSB->insertNextWord(piece);
			} else if (m_outStart == POS_SPECIAL_VALUE_NEXTFIELD) {
				pSB->insertNextField(piece);
			} else if (m_outStart == POS_SPECIAL_VALUE_NEXT) {
				pSB->insertNext(piece);
			} else if (m_outStart > 0) {
				pSB->insert(piece, m_outStart, true);
			}
		}
		
		return ApplyRet__SplitContinue;
	}
}

bool SplitItem::hasMorePieces() const
{
	return m_splitting && (m_currentPiece + 1 < m_pieces.size());
}

void SplitItem::nextPiece()
{
	if (m_currentPiece + 1 < m_pieces.size()) {
		m_currentPiece++;
	}
}

void SplitItem::resetSplit()
{
	m_splitting = false;
	m_currentPiece = 0;
	m_pieces.clear();
	m_savedPrefix = nullptr;
	m_savedSBPos = 0;
}

void SplitItem::restorePrefix(StringBuilder* pSB)
{
	// Restore the StringBuilder to the saved prefix state
	// We need to make a fresh copy to avoid the saved prefix being modified
	if (m_savedPrefix) {
		PSpecString freshCopy = std::make_shared<std::string>(*m_savedPrefix);
		pSB->setString(freshCopy);
		pSB->setPosition(m_savedSBPos);
	} else {
		pSB->clear();
	}
}
