#ifndef SPECS2016__CLI__TOKENS__H
#define SPECS2016__CLI__TOKENS__H

#include <string>
#include <vector>
#include <assert.h>

#define TOKEN_TYPE_LIST   \
	/* The MainOptions */    \
	X(STOP,           false, false) \
	X(ALLEOF,         false, false) \
	X(ANYEOF,         false, false) \
	X(COUNTERS,       false, false) \
	X(PRINTONLY,      false, false) \
	X(EOF,            false, false) \
	X(KEEP,           false, false) \
	/* The input ranges */  \
	X(RANGELABEL,     false, true)  \
	X(PERIOD,         false, false) \
	X(RANGE,          true,  false) \
	X(WORDRANGE,      true,  false) \
	X(FIELDSEPARATOR, false, true)  \
	X(WORDSEPARATOR,  false, true)  \
	X(NEXTWORD,       true,  false) \
	X(NEXTFIELD,      true,  false) \
	X(NEXT,           true,  false) \
	X(FIELDRANGE,     true,  false) \
	X(SUBSTRING,      false, false) \
	X(OF,             false, false) \
	X(GROUPSTART,     false, false) \
	X(GROUPEND,       false, false) \
	X(STRIP,          false, false) \
	X(LITERAL,        false, true) \
	X(CONVERSION,     false, true) \
	X(DUMMY,          false, false)

#define X(t,r,l) TokenListType__##t,
enum TokenListTypes {
	TOKEN_TYPE_LIST
	TokenListType__COUNT_ITEMS
};
#undef X

#define X(t,r,l) #t,
static std::string TokenListTypeStrArr[TokenListType__COUNT_ITEMS] = {\
	TOKEN_TYPE_LIST
};
#undef X

static inline std::string& TokenListType__2str(TokenListTypes tok) { 
	return TokenListTypeStrArr[tok];
}

#define LAST_POS_END 0  // useful because 0 is an invalid index

class TokenFieldRange {
	public:
		virtual void Reset() = 0;
		virtual bool Done() = 0;
		virtual int  Next() = 0;
		virtual void setDone() {bDone=true;}
		virtual std::string Debug() {return "generic range";}
		virtual bool isSingleNumber() {return false;}
		virtual int  getSingleNumber() {return 0;}
		virtual bool isSimpleRange() {return false;}
		virtual int  getSimpleFirst() {return 0;}
		virtual int  getSimpleLast()  {return 0;}
	protected:
		bool bDone;
};

class Token {
public:
	Token(TokenListTypes _type, TokenFieldRange *pRange, std::string literal, int argc, std::string orig);
	std::string     Debug(int digits =  0);
	TokenListTypes  Type() {return m_type;}
	TokenFieldRange *Range() {return m_pRange;}
	void            setRange(TokenFieldRange *prng) {assert(m_pRange==NULL); m_pRange = prng;}
	std::string&    Literal() {return m_literal;}
	void            setLiteral(std::string l) {assert(m_literal.empty()); m_literal = l;}
	int             argIndex() {return m_argc;}
	std::string&    Orig() {return m_orig;}
	std::string&    HelpIdentify();
private:
	TokenListTypes  m_type;
	TokenFieldRange *m_pRange;
	std::string     m_literal;
	int             m_argc;
	std::string     m_orig;
};

extern Token dummyToken;

std::vector<Token> parseTokens(int argc, char** argv);
void normalizeTokenList(std::vector<Token> *tokList);

#endif
