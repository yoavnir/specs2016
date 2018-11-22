#include "Config.h"
#include "utils/ErrorReporting.h"
#include "ProcessingState.h"

#define EMPTY_FIELD_MARKER  999999999

ProcessingState::ProcessingState()
{
	m_pad = DEFAULT_PAD_CHAR;
	m_wordSeparator = DEFAULT_WORDSEPARATOR;
	m_fieldSeparator = DEFAULT_FIELDSEPARATOR;
	m_fieldCount = 0;
	m_wordCount = 0;
	m_ps = NULL;
}

ProcessingState::ProcessingState(ProcessingState& ps)
{
	m_pad = ps.m_pad;
	m_wordSeparator = ps.m_wordSeparator;
	m_fieldSeparator = ps.m_fieldSeparator;
	m_fieldCount = 0;
	m_wordCount = 0;
	m_ps = NULL;
}

ProcessingState::ProcessingState(ProcessingState* pPS)
{
	m_pad = pPS->m_pad;
	m_wordSeparator = pPS->m_wordSeparator;
	m_fieldSeparator = pPS->m_fieldSeparator;
	m_fieldCount = 0;
	m_wordCount = 0;
	m_ps = NULL;
}

ProcessingState::~ProcessingState()
{
	fieldIdentifierClear();
}

void ProcessingState::setString(PSpecString ps)
{
	if (m_ps && ps!=m_ps) {
		delete m_ps;
	}
	m_ps = ps;
	m_wordCount = -1;
	m_fieldCount = -1;
	fieldIdentifierClear();
}

void ProcessingState::identifyWords()
{
	m_wordStart.clear();
	m_wordEnd.clear();
	m_wordCount = 0;
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 is not yet supported");
	}
	const char* pc = m_ps->data();
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
	m_fieldStart.clear();
	m_fieldEnd.clear();
	m_fieldCount = 0;
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 is not yet supported");
	}
	const char* pc = m_ps->data();
	int i = 0;

	while (pc[i]!=0) {
		m_fieldCount++;
		m_fieldStart.insert(m_fieldStart.end(), i+1);
		while (pc[i]!=m_fieldSeparator && pc[i]!=0) i++;
		m_fieldEnd.insert(m_fieldEnd.end(), (i==0) ? EMPTY_FIELD_MARKER : i);
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

unsigned int ProcessingState::getWordCount()
{
	if (m_wordCount==-1) {
		identifyWords();
	}

	return m_wordCount;
}

unsigned int ProcessingState::getFieldCount()
{
	if (m_fieldCount==-1) {
		identifyFields();
	}

	return m_fieldCount;
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

	if (idx==0) {
		idx = m_wordCount;
	} else if (idx < 0) {
		if ((-idx) > m_wordCount) return 0;
		idx += m_wordCount + 1;
	}
	if (idx > m_wordCount) return 0;
	return m_wordEnd[idx-1];
}

// Convention: returns NULL for an empty string
// Convention: from=0 means from the start (same as 1)
// Convention: to=0 means to the end
PSpecString ProcessingState::getFromTo(int from, int to)
{
	int slen = (int)(m_ps->length());

	if (from==1 && to==EMPTY_FIELD_MARKER) return SpecString::newString();

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

	return SpecString::newString(m_ps, from-1, to-from+1);
}

void ProcessingState::fieldIdentifierClear()
{
	for (const auto &pair : m_fieldIdentifiers) {
		delete pair.second;
	}
	m_fieldIdentifiers.clear();
}

void ProcessingState::fieldIdentifierSet(char id, PSpecString ps)
{
	if (m_fieldIdentifiers[id]!=NULL) {
		std::string err = std::string("Field Identifier <") + id + "> redefined.";
		MYTHROW(err);
	}
	m_fieldIdentifiers[id] = SpecStringCopy(ps);
}

PSpecString ProcessingState::fieldIdentifierGet(char id)
{
	PSpecString ret = m_fieldIdentifiers[id];
	if (!ret) {
		std::string err = std::string("Field Identifier <") + id + "> not defined yet.";
		MYTHROW(err);
	}
	return ret;
}
