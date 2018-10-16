#ifndef SPECS2016__PROCESSING__PROCESSINGSTATE__H
#define SPECS2016__PROCESSING__PROCESSINGSTATE__H

#include <string>
#include <vector>

#define DEFAULT_PAD_CHAR ' '
#define DEFAULT_WORDSEPARATOR ' '
#define DEFAULT_FIELDSEPARATPR '\t'

class ProcessingState {
public:
	ProcessingState();
	ProcessingState(ProcessingState* pPS);
	ProcessingState(ProcessingState& ps);
	void    setPadChar(char c) {m_pad = c;}
	void    setWSChar(char c) {m_wordSeparator = c; m_wordCount=-1;}
	void    setFSChar(char c) {m_fieldSeparator = c; m_fieldCount=-1;}
	void    setString(std::string* ps);
	int     getWordStart(int idx);
	int     getWordEnd(int idx);
	int     getFieldStart(int idx);
	int     getFieldEnd(int idx);
	std::string* getFromTo(int from, int to);
	char    m_pad;
	char    m_wordSeparator;
	char    m_fieldSeparator;
	std::string* m_ps;
	int  m_wordCount;
	int  m_fieldCount;
	std::vector<int> m_wordStart;
	std::vector<int> m_wordEnd;
	std::vector<int> m_fieldStart;
	std::vector<int> m_fieldEnd;
private:
	void identifyWords();
	void identifyFields();
};

#endif
