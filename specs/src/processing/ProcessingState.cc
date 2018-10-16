#include "ProcessingState.h"

ProcessingState::ProcessingState()
{
	m_pad = DEFAULT_PAD_CHAR;
	m_wordSeparator = DEFAULT_WORDSEPARATOR;
	m_fieldSeparator = DEFAULT_FIELDSEPARATPR;
	m_ps = NULL;
}

ProcessingState::ProcessingState(ProcessingState& ps)
{
	m_pad = ps.m_pad;
	m_wordSeparator = ps.m_wordSeparator;
	m_fieldSeparator = ps.m_fieldSeparator;
	m_ps = NULL;
}

ProcessingState::ProcessingState(ProcessingState* pPS)
{
	m_pad = pPS->m_pad;
	m_wordSeparator = pPS->m_wordSeparator;
	m_fieldSeparator = pPS->m_fieldSeparator;
	m_ps = NULL;
}

void ProcessingState::setString(std::string* ps)
{
	if (m_ps && ps!=m_ps) {
		delete m_ps;
	}
	m_ps = ps;
	m_wordCount = -1;
	m_fieldCount = -1;
}

void ProcessingState::identifyWords()
{
	m_wordStart.empty();
	m_wordEnd.empty();
	m_wordCount = 0;
	const char* pc = m_ps->c_str();
	int i = 0;
	/* skip over initial whitespace */
	while (pc[i]==m_wordSeparator) i++;

	while (pc[i]!=0) {
		m_wordCount++;
		m_wordStart.insert(m_wordStart.end(), i+1);
		while (pc[i]!=m_wordSeparator && pc[i]!=0) i++;
		m_wordEnd.insert(m_wordEnd.end(), i);
		while (pc[i]==m_wordSeparator) i++;
	}
}

void ProcessingState::identifyFields()
{
	m_fieldStart.empty();
	m_fieldEnd.empty();
	m_fieldCount = 0;
	const char* pc = m_ps->c_str();
	int i = 0;

	while (pc[i]!=0) {
		m_fieldCount++;
		m_fieldStart.insert(m_fieldStart.end(), i+1);
		while (pc[i]!=m_fieldSeparator && pc[i]!=0) i++;
		m_fieldEnd.insert(m_fieldEnd.end(), i);
		if (pc[i]==m_fieldSeparator) i++;
	}
}

int ProcessingState::getFieldStart(int idx) {
	if (m_fieldCount==-1) {
		identifyFields();
	}
	assert(idx!=0);
	if (idx < 0) {
		if ((-idx) > m_fieldCount) return 0;
		idx += m_fieldCount + 1;
	}
	if (idx > m_fieldCount) return 0;
	return m_fieldStart[idx-1];
}

int ProcessingState::getWordStart(int idx) {
	if (m_wordCount==-1) {
		identifyWords();
	}
	assert(idx!=0);
	if (idx < 0) {
		if ((-idx) > m_wordCount) return 0;
		idx += m_wordCount + 1;
	}
	if (idx > m_wordCount) return 0;
	return m_wordStart[idx-1];
}

int ProcessingState::getFieldEnd(int idx) {
	if (m_fieldCount==-1) {
		identifyFields();
	}
	assert(idx!=0);
	if (idx < 0) {
		if ((-idx) > m_fieldCount) return 0;
		idx += m_fieldCount + 1;
	}
	if (idx > m_fieldCount) return 0;
	return m_fieldEnd[idx-1];
}

int ProcessingState::getWordEnd(int idx) {
	if (m_wordCount==-1) {
		identifyWords();
	}
	assert(idx!=0);
	if (idx < 0) {
		if ((-idx) > m_wordCount) return 0;
		idx += m_wordCount + 1;
	}
	if (idx > m_wordCount) return 0;
	return m_wordEnd[idx-1];
}

// Convention: returns NULL for an empty string
// Convention: from=0 means from the start (same as 1)
// Convention: to=0 means to the end
std::string* ProcessingState::getFromTo(int from, int to)
{
	int slen = (int)(m_ps->length());

	// conventions
	if (from==0) from=1;
	if (to==0) to=slen;

	// fix the negatives
	if (from<0) {
		from = from + slen + 1;
		if (from<1) from = 1;
	}
	if (to<0) {
		to = to + slen + 1;
		if (to<1) to = 1;
	}

	// Consider overflows
	if (from>slen) return NULL;
	if (to>slen) to = slen;

	// to < from ==> empty string
	if (to<from) return NULL;

	return new std::string(*m_ps, from-1, to-from+1);
}
