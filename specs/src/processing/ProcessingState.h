#ifndef SPECS2016__PROCESSING__PROCESSINGSTATE__H
#define SPECS2016__PROCESSING__PROCESSINGSTATE__H

#include <vector>
#include <map>
#include "utils/SpecString.h"

#define DEFAULT_PAD_CHAR ' '
#define DEFAULT_WORDSEPARATOR ' '
#define DEFAULT_FIELDSEPARATOR '\t'

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
private:
	void identifyWords();
	void identifyFields();
	std::map<char,PSpecString> m_fieldIdentifiers;
};

#endif
