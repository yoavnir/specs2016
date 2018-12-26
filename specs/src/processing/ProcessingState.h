#ifndef SPECS2016__PROCESSING__PROCESSINGSTATE__H
#define SPECS2016__PROCESSING__PROCESSINGSTATE__H

#include <vector>
#include <stack>
#include <map>
#include "utils/alu.h"
#include "utils/SpecString.h"

#define DEFAULT_PAD_CHAR ' '
#define DEFAULT_WORDSEPARATOR ' '
#define DEFAULT_FIELDSEPARATOR '\t'

#define LOOP_CONDITION_FALSE (-5)

class ProcessingState {
public:
	ProcessingState();
	ProcessingState(ProcessingState* pPS);
	ProcessingState(ProcessingState& ps);
	~ProcessingState();
	void    setPadChar(char c) {m_pad = c;}
	void    setWSChar(char c) {m_wordSeparator = c; m_wordCount=-1;}
	void    setFSChar(char c) {m_fieldSeparator = c; m_fieldCount=-1;}
	void    setString(PSpecString ps);
	unsigned int getWordCount();
	unsigned int getFieldCount();
	int     getWordStart(int idx);
	int     getWordEnd(int idx);
	int     getFieldStart(int idx);
	int     getFieldEnd(int idx);
	PSpecString getFromTo(int from, int to);
	char    m_pad;
	char    m_wordSeparator;
	char    m_fieldSeparator;
	PSpecString m_ps;
	int  m_wordCount;
	int  m_fieldCount;
	std::vector<int> m_wordStart;
	std::vector<int> m_wordEnd;
	std::vector<int> m_fieldStart;
	std::vector<int> m_fieldEnd;
	void fieldIdentifierSet(char id, PSpecString ps);
	PSpecString fieldIdentifierGet(char id);
	void fieldIdentifierClear();
	bool needToEvaluate();
	bool runningOutLoop();
	void setCondition(bool isTrue);
	void observeIf();
	void observeElse();
	void observeElseIf(bool& evaluateCond);
	void observeEndIf();
	void observeWhile();
	void observeDone();
	void pushLoop(int n)  { m_Loops.push(n); }
	int  getLoopStart();
private:
	enum extremeBool {
		bFalse,
		bTrue,
		bDontCare
	};
	void identifyWords();
	void identifyFields();
	std::map<char,PSpecString> m_fieldIdentifiers;
	std::stack<extremeBool> m_Conditions;
	std::stack<int> m_Loops;    // The unsigned int holds the number of the token where the while was
};


// Helper class for the ALU
class ProcessingStateFieldIdentifierGetter : public fieldIdentifierGetter {
public:
	ProcessingStateFieldIdentifierGetter(ProcessingState* _ps) : m_ps(_ps) {}
	~ProcessingStateFieldIdentifierGetter()                               {}
	std::string Get(char id);
private:
	ProcessingState*	m_ps;
};

#endif
