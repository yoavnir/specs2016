#ifndef SPECS2016__CLI__TOKENS__H
#define SPECS2016__CLI__TOKENS__H

#include <string>
#include <vector>
#include "utils/ErrorReporting.h"

//
// The TOKEN_TYPE_LIST X-macro
//
//   X(t,r,l)
//     t = the token name
//     r = token contains a range
//     l = token contains a literal
#define TOKEN_TYPE_LIST   \
	/* The MainOptions */    \
	X(STOP,           false, true)  \
	X(ALLEOF,         false, false) \
	X(ANYEOF,         false, false) \
	X(COUNTERS,       false, false) \
	X(PRINTONLY,      false, true)  \
	X(EOF,            false, false) \
	X(KEEP,           false, false) \
	X(READ,           false, false) \
	X(READSTOP,       false, false) \
	X(WRITE,          false, false) \
	X(NOWRITE,        false, false) \
	X(ASSERT,         false, true)  \
	X(ABEND,          false, true)  \
	/* The input ranges */  \
	X(RANGELABEL,     false, true)  \
	X(ID,             false, true)  \
	X(PERIOD,         false, false) \
	X(RANGE,          true,  false) \
	X(WORDRANGE,      true,  false) \
	X(FIELDSEPARATOR, false, true)  \
	X(WORDSEPARATOR,  false, true)  \
	X(PAD,            false, true)  \
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
	X(LEFT,           false, false) \
	X(CENTER,         false, false) \
	X(RIGHT,          false, false) \
	X(NUMBER,         false, false) \
	X(TODCLOCK,       false, false) \
	X(DTODCLOCK,      false, false) \
	X(TIMEDIFF,       false, false) \
	X(SET,            false, true) \
	X(PRINT,          false, true)  \
	X(IF,             false, true)  \
	X(THEN,           false, false) \
	X(ELSE,           false, false) \
	X(ELSEIF,         false, true)  \
	X(ENDIF,          false, false) \
	X(CONTINUE,       false, false) \
	X(WHILE,          false, true)  \
	X(DO,             false, false) \
	X(DONE,           false, false) \
	X(UNREAD,         false, false) \
	X(REDO,           false, false) \
	X(BREAK,          false, true)  \
	X(SELECT,         false, true)  \
	X(FIRST,          false, false) \
	X(SECOND,         false, false) \
	X(OUTSTREAM,      false, true)  \
	X(STDERR,         false, false) \
	X(REQUIRES,       false, true)  \
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
		virtual ~TokenFieldRange() {}
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
	void            setRange(TokenFieldRange *prng) {MYASSERT(m_pRange==NULL); m_pRange = prng;}
	std::string&    Literal() {return m_literal;}
	void            setLiteral(std::string l) {MYASSERT(m_literal.empty()); m_literal = l;}
	void            setLiteral(char c) {m_literal.resize(1); m_literal[0]=c;}
	int             argIndex() {return m_argc;}
	std::string&    Orig() {return m_orig;}
	std::string&    HelpIdentify();
	void            deallocDynamic() {if (m_pRange) {delete m_pRange; m_pRange = NULL;}}
private:
	TokenListTypes  m_type;
	TokenFieldRange *m_pRange;
	std::string     m_literal;
	int             m_argc;
	std::string     m_orig;
};

extern Token dummyToken;

std::vector<Token> parseTokens(int argc, char** argv);

std::vector<Token> parseTokensFile(std::string& fileName);

void normalizeTokenList(std::vector<Token> *tokList);

bool dumpSpecificationsList(std::string specName = "");

#endif
