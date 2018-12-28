#ifndef SPECS2016__PROCESSING__PROCESSINGSTATE__H
#define SPECS2016__PROCESSING__PROCESSINGSTATE__H

#include <vector>
#include <stack>
#include <map>
#include "utils/alu.h"
#include "utils/aluFunctions.h"

#define DEFAULT_PAD_CHAR ' '
#define DEFAULT_WORDSEPARATOR ' '
#define DEFAULT_FIELDSEPARATOR '\t'

#define LOOP_CONDITION_FALSE (-5)

class ProcessingState : public stateQueryAgent {
public:
	ProcessingState();
	ProcessingState(ProcessingState* pPS);
	ProcessingState(ProcessingState& ps);
	~ProcessingState();

	void    setPadChar(char c) {m_pad = c;}
	void    setWSChar(char c) {m_wordSeparator = c; m_wordCount=-1;}
	void    setFSChar(char c) {m_fieldSeparator = c; m_fieldCount=-1;}

	char    getPadChar() { return m_pad;            }
	char    getWSChar()  { return m_wordSeparator;  }
	char    getFSChar()  { return m_fieldSeparator; }

	void    setString(PSpecString ps);

	// The stateQueryAgent interface
	virtual unsigned int getWordCount();
	virtual unsigned int getFieldCount();
	virtual int     getWordStart(int idx);
	virtual int     getWordEnd(int idx);
	virtual int     getFieldStart(int idx);
	virtual int     getFieldEnd(int idx);
	virtual PSpecString getFromTo(int from, int to);
	virtual bool    isRunIn()   { return (m_CycleCounter==1); }
	virtual bool    isRunOut()  { return (m_ps==NULL); } // NOTE: will return true before first record

	void fieldIdentifierSet(char id, PSpecString ps);
	void incrementCycleCounter() { m_CycleCounter++; }
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
	char    m_pad;
	char    m_wordSeparator;
	char    m_fieldSeparator;
	PSpecString m_ps;
	int  m_wordCount;
	int  m_fieldCount;
	unsigned int m_CycleCounter;
	std::vector<int> m_wordStart;
	std::vector<int> m_wordEnd;
	std::vector<int> m_fieldStart;
	std::vector<int> m_fieldEnd;
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
