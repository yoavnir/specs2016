#include <string.h>
#include <regex>
#include "utils/platform.h"
#include "tokens.h"
#include "processing/Config.h"
#include "utils/ErrorReporting.h"
#include "processing/conversions.h"
#include "processing/Reader.h"
#include "processing/ProcessingState.h"

extern std::string conv_X2CH(std::string& s);

Token dummyToken(TokenListType__DUMMY, NULL, "", 0, std::string("dummyToken"));

std::vector<Token> parseTokensSplit(const char* arg);

class TokenFieldRangeSimple : public TokenFieldRange {
	public:
		TokenFieldRangeSimple(int _from, int _to) {m_first = _from; m_last = _to; m_idx = _from; bIsSingleNumber = false;}
		virtual void Reset() {m_idx = m_first; bDone = false;}
		virtual bool Done()  {return bDone || (m_idx > m_last && m_last!=LAST_POS_END);}
		virtual int  Next() {
			if (Done()) {
				return LAST_POS_END;
			}
			/* skip zero index */
			if (m_idx==-1) {
				m_idx = 1;
				return -1;
			}
			else return m_idx++;
		}
		virtual std::string Debug() {
			if (bIsSingleNumber) {
				return std::string("S:") + std::to_string(m_first);
			}
			return std::string("S:")+
				std::to_string(m_first) +
				"-" +
				((m_last==LAST_POS_END) ? "end" : std::to_string(m_last));
		}
		void setSingleNumber() {bIsSingleNumber = true;}
		virtual bool isSingleNumber() {return bIsSingleNumber;}
		virtual int  getSingleNumber() {return m_first;}
		virtual bool isSimpleRange() {return true;}
		virtual int  getSimpleFirst() {return m_first;}
		virtual int  getSimpleLast() {return m_last;}
	private:
		int m_first, m_last, m_idx;
		bool bIsSingleNumber;
};

class TokenFieldRangeComplex : public TokenFieldRange {
public:
	TokenFieldRangeComplex();
	virtual void Reset();
	virtual bool Done();
	virtual int  Next();
	virtual void setDone();
	void         Add(TokenFieldRange *rng);
	virtual std::string Debug();
private:
	int m_idx;
	std::vector<TokenFieldRange*> rangeVec;
};

TokenFieldRangeComplex::TokenFieldRangeComplex()
{
	m_idx = 0;
}

void TokenFieldRangeComplex::Reset()
{
	m_idx = 0;
	if (rangeVec.size()>0) {
		rangeVec[0]->Reset();
	}
}

bool TokenFieldRangeComplex::Done()
{
	return (m_idx >= rangeVec.size());
}

int TokenFieldRangeComplex::Next()
{
	if (Done()) {
		return LAST_POS_END;
	}

	int ret = rangeVec[m_idx]->Next();
	if (rangeVec[m_idx]->Done()) {
		m_idx++;
		if (m_idx < rangeVec.size()) {
			rangeVec[m_idx]->Reset();
		}
	}
	return ret;
}

void TokenFieldRangeComplex::setDone()
{
	if (!Done()) {
		m_idx++;
		if (m_idx < rangeVec.size()) {
			rangeVec[m_idx]->Reset();
		}
	}
}

std::string TokenFieldRangeComplex::Debug()
{
	std::string ret = "C:(";
	for (int i=0; i<rangeVec.size(); i++) {
		if (i>0) {
			ret += " ";
		}
		ret += rangeVec[i]->Debug();
	}
	ret += ")";
	return ret;
}

void TokenFieldRangeComplex::Add(TokenFieldRange *rng)
{
	rangeVec.insert(rangeVec.end(), rng);
}


Token::Token(TokenListTypes _type, TokenFieldRange *pRange, std::string literal, int argc, std::string orig)
{
	m_type = _type;
	m_pRange = pRange;
	m_literal = literal;
	m_argc = argc;
	m_orig = orig;
}

#define X(t,r,l) case TokenListType__##t: \
					if (r && m_pRange) ret += "; " + m_pRange->Debug(); \
					if (l) ret +=  (m_literal.empty()) ? ("; EMPTY!!!") : ("; /" + m_literal + "/");  \
					break;
std::string Token::Debug(int digits)
{
	std::string ret = TokenListType__2str(m_type);
	if (digits) {
		ret.resize(16-digits,' ');
	}
	switch (m_type) {
		TOKEN_TYPE_LIST
		case TokenListType__COUNT_ITEMS:
			MYTHROW("Bad m_type in Token");
	}
	return ret;
}
#undef X

std::string& Token::HelpIdentify()
{
	static std::string ret = "Token " + TokenListType__2str(m_type) + " at index " +
			std::to_string(m_argc) + " with content <" + ((m_literal.empty()) ? m_orig : m_literal) + ">";
	return ret;
}

/* Helper functions */
static TokenFieldRange *parseAsSingleNumber(std::string s)
{
	long int l;
	try {
		l = std::stol(s);
	} catch(std::invalid_argument& e) {
		return NULL;
	}
	if (l==0 || s!=std::to_string(l)) {
		return NULL;
	}
	TokenFieldRangeSimple* pRet = new TokenFieldRangeSimple(l,l);
	pRet->setSingleNumber();
	return pRet;
}

static TokenFieldRange *parseAsFromToRange(std::string s)
{
	static std::string hyphens("-;:"); // all can separate the two numbers
	size_t posOfHyphen;
	bool   bRealHyphen; // rather than semicolon or colon
	long int _from, _to;
	try {
		_from = std::stol(s, &posOfHyphen);
	} catch(std::invalid_argument& e) {
		return NULL;
	}
	if (_from==0 || s.substr(0,posOfHyphen)!=std::to_string(_from)
		|| (std::string::npos==hyphens.find(s[posOfHyphen]))) {
		return NULL;
	}

	bRealHyphen = (s[posOfHyphen]=='-');

	try {
		_to = std::stol(s.substr(posOfHyphen+1));
		if (_to==0 || s.substr(posOfHyphen+1)!=std::to_string(_to)) {
			return NULL;
		}
		if (bRealHyphen) {
			if (_to < _from || _from < 1) {
				return NULL;
			}
		}
	} catch (std::invalid_argument& e) {
		if (s.substr(posOfHyphen+1)=="*") {
			_to = LAST_POS_END;
		} else {
			return NULL;
		}
	}

	return new TokenFieldRangeSimple(_from, _to);
}

static TokenFieldRange *parseAsFromLenRange(std::string s)
{
	size_t posOfDot;
	long int _from, _to, _len;
	try {
		_from = std::stol(s, &posOfDot);
	} catch(std::invalid_argument& e) {
		return NULL;
	}
	if (_from==0 || s.substr(0,posOfDot)!=std::to_string(_from) || s[posOfDot]!='.') {
		return NULL;
	}
	try {
		_len = std::stol(s.substr(posOfDot+1));
	} catch (std::invalid_argument& e) {
		return NULL;
	}
	if (_len<=0 || s.substr(posOfDot+1)!=std::to_string(_len)) {
		return NULL;
	}

	_to = _from + _len - 1;
	if (_from<0 && _to>=0) {
		_to++;  // skipping the zero
	}

	return new TokenFieldRangeSimple(_from, _to);
}

static TokenFieldRange *parseAsAnySimpleRangeSpec(std::string s)
{
	TokenFieldRange *ret = NULL;
	ret = parseAsSingleNumber(s);
	if (!ret)
		ret = parseAsFromToRange(s);
	if (!ret)
		ret = parseAsFromLenRange(s);
	return ret;
}

static bool compareStringWithMinLength(std::string& shortString, const char* sz, size_t minLen)
{
	if (shortString.length() < minLen || shortString.length() > strlen(sz)) {
		return false;
	}

	return 0==strncasecmp(shortString.c_str(), sz, shortString.length());
}

#define CASEEQ(s,sz) (strcasecmp(s.c_str(), sz)==0)
#define NEXT_TOKEN return
#define SIMPLETOKEN(s,t)   \
		if (CASEEQ(arg,#s)) {   \
			pVec->insert(pVec->end(),   \
					Token(TokenListType__##t, NULL, "", argidx, arg)); \
			return; \
		}

#define SIMPLETOKENV(s,t,l)  \
		if (compareStringWithMinLength(arg,#s,l)) {   \
			pVec->insert(pVec->end(),    \
					Token(TokenListType__##t, NULL, "", argidx, arg)); \
			return;  \
		}

/* Big parser functions */
static bool isPossibleDelimiter(char c) {
	return (c=='/' || c=='"' || c=='\'');
}
static void parseInputRangesTokens(std::vector<Token> *pVec, std::string s, int argidx);
void parseSingleToken(std::vector<Token> *pVec, std::string arg, int argidx)
{
	TokenFieldRange* pRange = NULL;

	/* various ifs, buts and maybes */

	/* Some simple tokens */
	SIMPLETOKENV(substring,SUBSTRING,6);  // can be shortened down to substr
	SIMPLETOKENV(word,WORDRANGE,1);
	SIMPLETOKENV(field,FIELDRANGE,1);
	SIMPLETOKENV(fieldseparator,FIELDSEPARATOR,8); // down to fieldsep
	SIMPLETOKEN(fs,FIELDSEPARATOR);
	SIMPLETOKENV(wordseparator,WORDSEPARATOR,7); // down to wordsep
	SIMPLETOKEN(ws,WORDSEPARATOR);
	SIMPLETOKEN(pad, PAD);
	SIMPLETOKEN(of,OF);
	SIMPLETOKEN(stop, STOP);
	SIMPLETOKEN(alleof, ALLEOF);
	SIMPLETOKEN(anyeof, ANYEOF);
	SIMPLETOKEN(counters, COUNTERS);
	SIMPLETOKEN(printonly, PRINTONLY);
	SIMPLETOKEN(eof, EOF);
	SIMPLETOKEN(keep, KEEP);
	SIMPLETOKEN(strip, STRIP);
	SIMPLETOKEN(left, LEFT);
	SIMPLETOKEN(right, RIGHT);
	SIMPLETOKEN(center, CENTER);
	SIMPLETOKEN(centre, CENTER);
	SIMPLETOKEN(number, NUMBER);
	SIMPLETOKEN(read, READ);
	SIMPLETOKEN(readstop, READSTOP);
	SIMPLETOKEN(write, WRITE);
	SIMPLETOKEN(nowrite, NOWRITE);
	SIMPLETOKEN(noprint, NOWRITE);
	SIMPLETOKEN(id, ID);
	SIMPLETOKENV(todclock, TODCLOCK, 3);
	SIMPLETOKENV(dtodclock, DTODCLOCK, 4);
	SIMPLETOKEN(timediff, TIMEDIFF);
	SIMPLETOKEN(set, SET);
	SIMPLETOKEN(print, PRINT);
	SIMPLETOKEN(if, IF);
	SIMPLETOKEN(else, ELSE);
	SIMPLETOKEN(then, THEN);
	SIMPLETOKEN(endif, ENDIF);
	SIMPLETOKEN(elseif, ELSEIF);
	SIMPLETOKEN(while, WHILE);
	SIMPLETOKEN(do, DO);
	SIMPLETOKEN(done, DONE);
	SIMPLETOKEN(unread, UNREAD);
	SIMPLETOKEN(redo, REDO);
	SIMPLETOKEN(break, BREAK);
	SIMPLETOKEN(select, SELECT);
	SIMPLETOKEN(first, FIRST);
	SIMPLETOKEN(second, SECOND);
	SIMPLETOKEN(outstream, OUTSTREAM);
	SIMPLETOKEN(stderr, STDERR);

	/* range label */
	if (arg.length()==2 && arg[1]==':' &&
			((arg[0]>='a' && arg[0]<='z') || (arg[0]>='A' && arg[0]<='Z'))) {
		pVec->insert(pVec->end(),
				Token(TokenListType__RANGELABEL,
						NULL, arg.substr(0,1), argidx, arg));
		NEXT_TOKEN;
	}

	/* period */
	if (arg.length()==1 && arg[0]=='.') {
		pVec->insert(pVec->end(),
				Token(TokenListType__PERIOD,
						NULL, "", argidx, arg));
		NEXT_TOKEN;
	}

	/* conversions */
	if (StringConversion__NONE!=getConversionByName(arg)) {
		pVec->insert(pVec->end(),
				Token(TokenListType__CONVERSION,
						NULL, arg, argidx, arg));
		NEXT_TOKEN;
	}

	/* Try as a range */
	if ((pRange = parseAsAnySimpleRangeSpec(arg))) {
		pVec->insert(pVec->end(),
				Token(TokenListType__RANGE,
						pRange, arg, argidx, arg));
		NEXT_TOKEN;
	}

	/* Next, try a word range */
	if (arg[0]=='w' || arg[0]=='W') {
		if ((pRange = parseAsAnySimpleRangeSpec(arg.substr(1)))) {
			pVec->insert(pVec->end(),
					Token(TokenListType__WORDRANGE,
							pRange, arg, argidx, arg));
			NEXT_TOKEN;
		}
	}

	/* Next, try a field range */
	if (arg[0]=='f' || arg[0]=='F') {
		if ((pRange = parseAsAnySimpleRangeSpec(arg.substr(1)))) {
			pVec->insert(pVec->end(),
					Token(TokenListType__FIELDRANGE,
							pRange, arg, argidx, arg));
			NEXT_TOKEN;
		}
	}

	/* Next Word and Next Field */
	size_t lastdot = arg.find_last_of('.');
	size_t firstdot = arg.find_first_of('.');
	if (lastdot==firstdot) { // either they're both NULL or there's exactly one dot
		TokenFieldRangeSimple *pSimpleRange = NULL;
		std::string nwnf = arg;
		if (firstdot!=std::string::npos) {
			nwnf = arg.substr(0,firstdot);
			std::string fieldLength = arg.substr(firstdot+1);
			try {
				int lFieldLength = std::stoi(fieldLength);
				if (lFieldLength > 0 && std::to_string(lFieldLength)==fieldLength) {
					pSimpleRange = new TokenFieldRangeSimple(1,lFieldLength);
				} else {
					goto CONT1;
				}
			} catch (std::invalid_argument& e) {
				goto CONT1;
			}
		}

		if (CASEEQ(nwnf,"nw") || CASEEQ(nwnf,"nword") || compareStringWithMinLength(nwnf,"nextword",5)) {
			pVec->insert(pVec->end(), Token(TokenListType__NEXTWORD, pSimpleRange, "", argidx, arg));
			NEXT_TOKEN;
		}

		if (CASEEQ(nwnf,"nf") || CASEEQ(nwnf,"nfield") || compareStringWithMinLength(nwnf,"nextfield",5)) {
			pVec->insert(pVec->end(), Token(TokenListType__NEXTFIELD, pSimpleRange, "", argidx, arg));
			NEXT_TOKEN;
		}

		if (compareStringWithMinLength(nwnf, "next", 1)) {
			pVec->insert(pVec->end(), Token(TokenListType__NEXT, pSimpleRange, "", argidx, arg));
			NEXT_TOKEN;
		}

		delete pSimpleRange;
	}

CONT1:
	/* input ranges */
	if (arg[0]=='(' && arg.back()==')') {
		pVec->insert(pVec->end(), Token(TokenListType__GROUPSTART, NULL, "", argidx, "("));
		parseInputRangesTokens(pVec, arg, argidx);
		pVec->insert(pVec->end(), Token(TokenListType__GROUPEND, NULL, "", argidx, ")"));
		NEXT_TOKEN;
	}

	/* Check for hex literal */
	if ((arg[0]=='x') && (arg.length() > 1) && (1==arg.length() % 2)) {
		try {
			std::string hexLiteral = arg.substr(1);
			std::string literal = conv_X2CH(hexLiteral);
			pVec->insert(pVec->end(),
				Token(TokenListType__LITERAL, NULL /* range */,
						literal, argidx, arg));
			NEXT_TOKEN;
		} catch(ConversionException& e) {
			;  // Do nothing. It just wasn't a hex string
		}
	}

	/* Check for a configuration literal */
	std::string key = arg.substr(1);
	if ((arg[0]=='@') && (arg.length() > 1) && (configSpecLiteralExists(key))) {
		std::string literal = configSpecLiteralGet(key);
		pVec->insert(pVec->end(),
				Token(TokenListType__LITERAL, NULL /* range */,
						literal, argidx, arg));
		NEXT_TOKEN;
	}

	/* Add as literal */
	{
		std::string literal;
		if (arg.front()==arg.back() && arg.length()>=2 && isPossibleDelimiter(arg.front())) {
			literal = arg.substr(1, arg.length()-2);
		} else {
			literal = arg;
		}

		pVec->insert(pVec->end(),
			Token(TokenListType__LITERAL, NULL /* range */,
				literal, argidx, arg));
		NEXT_TOKEN;
	}
}

#define MAX_INPUT_RANGES_IN_GROUP 256
static void parseInputRangesTokens(std::vector<Token> *pVec, std::string s, int argidx)
{
	char* localCopy = strdup(s.c_str()+1); // +2 to get rid of opening parenthesis
	char* itemPtrs[MAX_INPUT_RANGES_IN_GROUP];
	localCopy[s.length()-2]=0; // Gets rid of closing parenthesis

	unsigned int idx = 0;
	itemPtrs[idx] = strtok(localCopy, " ");
	while (itemPtrs[idx] && idx<MAX_INPUT_RANGES_IN_GROUP) {
		idx++;
		itemPtrs[idx] = strtok(NULL, " ");
	}
	if (idx==MAX_INPUT_RANGES_IN_GROUP) {
		std::string err = "Too many items in ranges group at index " + std::to_string(argidx);
		MYTHROW(err);
	}
	for (int i=0; i<idx; i++) {
		parseSingleToken(pVec, std::string(itemPtrs[i]), argidx);
	}

	free(localCopy);
}

std::vector<Token> parseTokens(int argc, char** argv)
{
	if (argc==1) {
		return parseTokensSplit(argv[0]);
	}
	std::vector<Token> ret;
	
	unsigned int argindex = 1;

	while (argc>0) {
		std::string arg(argv[0]);
		parseSingleToken(&ret, arg, argindex);
		argc--; argv++; argindex++;
	}
	
	return ret;
}

static bool mayBeLiteral(Token& tok)
{
	switch (tok.Type()) {
	case TokenListType__LITERAL:
	case TokenListType__PERIOD:
	case TokenListType__RANGE:
		/* TODO: expand this list */
		return true;
	default:
		return false;
	}
}

static bool mayBeFieldIdentifier(Token& tok)
{
	// doesn't matter how we parsed it.  As long as it's one character
	return (tok.Orig().length()==1);
}

std::string getLiteral(Token& tok)
{
	if (TokenListType__LITERAL==tok.Type()) {
		return tok.Literal();
	}
	if (mayBeLiteral(tok)) {
		return tok.Orig();
	}
	return std::string("");
}

void normalizeTokenList(std::vector<Token> *tokList)
{
	if (tokList->size()==0) return;

	for (int i=0; i<tokList->size()-1; i++) {
		Token& tok = tokList->at(i);
		Token& nextTok = tokList->at(i+1);
		switch (tok.Type()) {
		case TokenListType__WORDRANGE:
		case TokenListType__FIELDRANGE:
			if (tok.Range()==NULL) {
				if (nextTok.Type()!=TokenListType__RANGE) {
					std::string err = "Bad word/field range <"+nextTok.Orig()+"> at index "+ std::to_string(nextTok.argIndex())+".";
					MYTHROW(err);
				}
				tok.setRange(nextTok.Range());
				tokList->erase(tokList->begin()+(i+1));
			}
			break;
		case TokenListType__WORDSEPARATOR:
			if (tok.Literal()=="" && getLiteral(nextTok)=="default") {
				if (g_bLocalWhiteSpace) {
					nextTok.setLiteral(LOCAL_WHITESPACE);
				} else {
					nextTok.setLiteral(DEFAULT_WORDSEPARATOR);
				}
			}
		case TokenListType__FIELDSEPARATOR:
		case TokenListType__PAD:
			if (tok.Literal()=="") {
				if (mayBeLiteral(nextTok)) {
					if (getLiteral(nextTok).length()!=1) {
						std::string err = "Bad separator or pad character <"+nextTok.Orig() +
								">  with length " + std::to_string(getLiteral(nextTok).length()) +
								" at index "+std::to_string(nextTok.argIndex())+". Must be single character.";
						MYTHROW(err);
					}
					tok.setLiteral(getLiteral(nextTok));
					tokList->erase(tokList->begin()+(i+1));
				} else if (nextTok.Type()==TokenListType__RANGE) {
					TokenFieldRange *pRange = nextTok.Range();
					if (!pRange->isSingleNumber()) {
						std::string err = "Bad field separator <"+nextTok.Orig()+"> at index "+std::to_string(nextTok.argIndex())+". Must be single character.";
						MYTHROW(err);
					}
					int num = pRange->getSingleNumber();
					if (num<0 || num>=10) {
						std::string err = "Bad field separator <"+nextTok.Orig()+"> at index "+std::to_string(nextTok.argIndex())+". Must be single character.";
						MYTHROW(err);
					}
					tok.setLiteral(std::to_string(num));
					tokList->erase(tokList->begin()+(i+1));
				} else {
					std::string err = "Bad word/field separator <"+nextTok.Orig()+"> at index "+std::to_string(nextTok.argIndex())+". Must be single character.";
					MYTHROW(err);
				}
			}
			break;
		case TokenListType__ID:
		{
			if (tok.Literal()=="") {
				if (mayBeFieldIdentifier(nextTok)) {
					tok.setLiteral(getLiteral(nextTok));
					tokList->erase(tokList->begin()+(i+1));
				} else {
					std::string err = "Bad field identifier <"+nextTok.Orig()+"> for ID at index "+std::to_string(nextTok.argIndex());
					MYTHROW(err);
				}
			}
			break;
		}
		case TokenListType__SET:
		case TokenListType__PRINT:
		case TokenListType__IF:
		case TokenListType__ELSEIF:
		case TokenListType__WHILE:
		{
			if (tok.Literal()=="") {
				if (TokenListType__GROUPSTART == nextTok.Type()) {
					std::string expression = "(";
					do {
						tokList->erase(tokList->begin()+(i+1));
						nextTok = tokList->at(i+1);
						expression += getLiteral(nextTok);
					} while (TokenListType__GROUPEND != nextTok.Type());
					expression += ")";
					tokList->erase(tokList->begin()+(i+1));
					tok.setLiteral(expression);
				} else {
					tok.setLiteral(getLiteral(nextTok));
					tokList->erase(tokList->begin()+(i+1));
				}
			}
			break;
		}
		case TokenListType__SELECT:
		{
			if (tok.Literal()=="") {
				TokenListTypes nextType = nextTok.Type();
				switch (nextType) {
				case TokenListType__FIRST:
					tok.setLiteral("FIRST");
					break;
				case TokenListType__SECOND:
					tok.setLiteral("SECOND");
					break;
				case TokenListType__RANGE:
				{
					TokenFieldRange* pRange = nextTok.Range();
					// We just want one number between 1 and 8. Anything else causes an exception.
					if (!pRange || !pRange->isSingleNumber()) {
						std::string err = "Invalid input stream descriptor: " + pRange->Debug();
						MYTHROW(err);
					}
					int streamIndex = pRange->getSingleNumber();
					if (streamIndex < DEFAULT_READER_IDX || streamIndex > MAX_INPUT_STREAMS) {
						std::string err = "Invalid input stream descriptor: "+ std::to_string(streamIndex);
						MYTHROW(err);
					}
					tok.setLiteral(std::to_string(streamIndex));
					nextTok.deallocDynamic();
					break;
				}
				default:
					std::string err = "Invalid token of type " + TokenListType__2str(nextType) +
						" with content <" + nextTok.Orig() + "> following SELECT";
					MYTHROW(err);
				}
				tokList->erase(tokList->begin()+(i+1));
			}
			break;
		}
		case TokenListType__OUTSTREAM:
		{
			if (tok.Literal()=="") {
				TokenListTypes nextType = nextTok.Type();
				switch (nextType) {
				case TokenListType__STDERR:
					tok.setLiteral("err");
					break;
				case TokenListType__RANGE:
				{
					TokenFieldRange* pRange = nextTok.Range();
					// We just want one number between 1 and 8. Anything else causes an exception.
					if (!pRange || !pRange->isSingleNumber()) {
						std::string err = "Invalid input stream descriptor: " + pRange->Debug();
						MYTHROW(err);
					}
					int streamIndex = pRange->getSingleNumber();
					if (streamIndex < DEFAULT_READER_IDX || streamIndex > MAX_INPUT_STREAMS) {
						std::string err = "Invalid input stream descriptor: "+ std::to_string(streamIndex);
						MYTHROW(err);
					}
					tok.setLiteral(std::to_string(streamIndex));
					nextTok.deallocDynamic();
					break;
				}
				default:
					std::string err = "Invalid token of type " + TokenListType__2str(nextType) +
						" with content <" + nextTok.Orig() + "> following OUTSTREAM";
					MYTHROW(err);
				}
				tokList->erase(tokList->begin()+(i+1));
			}
			break;
		}
		case TokenListType__BREAK:  // next token must be a literal one-letter
		{
			if (tok.Literal()=="") {
				if (mayBeFieldIdentifier(nextTok)) {
					tok.setLiteral(getLiteral(nextTok));
					tokList->erase(tokList->begin()+(i+1));
				} else {
					std::string err = "Bad field identifier <"+nextTok.Orig()+"> for BREAK at index "+std::to_string(nextTok.argIndex());
					MYTHROW(err);
				}
			}
			break;
		}
		case TokenListType__FIRST:
		case TokenListType__SECOND:
		{
			std::string err = "Invalid token of type " + TokenListType__2str(tok.Type());
			MYTHROW(err);
			break;
		}
		default:
			break;
		}
	}
}

