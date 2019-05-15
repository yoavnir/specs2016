#include <cctype>
#include "Config.h"
#include "utils/ErrorReporting.h"
#include "Reader.h"
#include "ProcessingState.h"

#define EMPTY_FIELD_MARKER  999999999

// helper functions

/*
 * This function establishes the order between field identifiers:
 *   a, b, c..., z, A, B, ..., Z
 */
#define IS_UPPERCASE(x) ((x)>='A' && (x)<='Z')
#define IS_LOWERCASE(x) ((x)>='a' && (x)<='z')

bool breakLevelGE(char c1, char c2)
{
	if (!c2) return true;
	if (IS_UPPERCASE(c1)) {
		if (IS_UPPERCASE(c2)) {
			return (c1>=c2);
		} else if (IS_LOWERCASE(c2)) {
			return true;  // all uppercase is greater than all lowercase
		} else return false;
	} else if (IS_LOWERCASE(c1)) {
		if (IS_UPPERCASE(c2)) {
			return false;  // all uppercase is greater than all lowercase
		} else if (IS_LOWERCASE(c2)) {
			return (c1>=c2);
		} else return false;
	} else return false;
}

ProcessingState::ProcessingState()
{
	m_pad = DEFAULT_PAD_CHAR;
	if (g_bLocalWhiteSpace) {
		m_wordSeparator = LOCAL_WHITESPACE;
	} else {
		m_wordSeparator = DEFAULT_WORDSEPARATOR;
	}
	m_fieldSeparator = DEFAULT_FIELDSEPARATOR;
	m_fieldCount = 0;
	m_wordCount = 0;
	m_CycleCounter = 0;
	m_ExtraReads = 0;
	m_ps = NULL;
	m_prevPs = NULL;
	m_inputStation = STATION_FIRST;
	m_breakLevel = 0;
	m_inputStream = DEFAULT_READER_IDX;
	m_inputStreamChanged = false;
}

ProcessingState::ProcessingState(ProcessingState& ps)
{
	m_pad = ps.m_pad;
	m_wordSeparator = ps.m_wordSeparator;
	m_fieldSeparator = ps.m_fieldSeparator;
	m_fieldCount = 0;
	m_wordCount = 0;
	m_CycleCounter = 0;
	m_ExtraReads = 0;
	m_ps = NULL;
	m_prevPs = NULL;
	m_inputStation = STATION_FIRST;
	m_breakLevel = 0;
	m_inputStream = DEFAULT_READER_IDX;
	m_inputStreamChanged = false;
}

ProcessingState::ProcessingState(ProcessingState* pPS)
{
	m_pad = pPS->m_pad;
	m_wordSeparator = pPS->m_wordSeparator;
	m_fieldSeparator = pPS->m_fieldSeparator;
	m_fieldCount = 0;
	m_wordCount = 0;
	m_CycleCounter = 0;
	m_ExtraReads = 0;
	m_ps = NULL;
	m_prevPs = NULL;
	m_inputStation = STATION_FIRST;
	m_breakLevel = 0;
	m_inputStream = DEFAULT_READER_IDX;
	m_inputStreamChanged = false;
}

ProcessingState::~ProcessingState()
{
	fieldIdentifierClear();
	fieldIdentifierStatsClear();
	breakValuesClear();
	if (m_prevPs) {
		delete m_prevPs;
	}
	if (m_ps) {
		delete m_ps;
	}
}

void ProcessingState::setString(PSpecString ps)
{
	if (m_ps && ps!=m_ps) {
		if (m_prevPs) delete m_prevPs;
		m_prevPs = m_ps;
	} else {
		MYASSERT(m_prevPs==NULL);
		m_prevPs = SpecString::newString();
	}
	m_ps = ps;
	m_wordCount = -1;
	m_fieldCount = -1;
	fieldIdentifierClear();
	resetBreaks();
}

void ProcessingState::setStringInPlace(PSpecString ps)
{
	m_ps = ps;
}

void ProcessingState::setFirst()
{
	if (m_inputStation != STATION_FIRST) {
		m_inputStation = STATION_FIRST;
		m_wordCount = -1;
		m_fieldCount = -1;
	}
}

void ProcessingState::setSecond()
{
	if (m_inputStation != STATION_SECOND) {
		m_inputStation = STATION_SECOND;
		m_wordCount = -1;
		m_fieldCount = -1;
	}
}

void ProcessingState::setStream(int inputStreamIndex)
{
	MYASSERT(inputStreamIndex >= DEFAULT_READER_IDX);
	MYASSERT(inputStreamIndex <= MAX_INPUT_STREAMS);
	if (inputStreamIndex != m_inputStream) {
		m_wordCount = -1;
		m_fieldCount = -1;
		m_inputStream = inputStreamIndex;
		m_inputStreamChanged = true;
	}
}

void ProcessingState::setActiveWriter(int idx)
{
	MYASSERT(idx==STATION_STDERR || idx >= DEFAULT_READER_IDX);
	MYASSERT(idx <= MAX_INPUT_STREAMS);
	if (idx==STATION_STDERR) {
		m_outputIndex = 0;
	} else {
		m_outputIndex = idx;
		if (NULL == m_Writers[idx]) {
			std::string err = "Undefined output stream " + std::to_string(idx) + " selected.";
			MYTHROW(err);
		}
	}
}

Writer* ProcessingState::getCurrentWriter()
{
	return m_Writers[m_outputIndex];
}

PSpecString ProcessingState::extractCurrentRecord()
{
	PSpecString ret = m_ps;
	m_ps = SpecString::newString(); // empty string
	return ret;
}

#define IS_WHITESPACE(c) ((m_wordSeparator==LOCAL_WHITESPACE) ? (isspace((c))) : ((c)==m_wordSeparator))
void ProcessingState::identifyWords()
{
	m_wordStart.clear();
	m_wordEnd.clear();
	m_wordCount = 0;
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 is not yet supported");
	}
	const char* pc = currRecord()->data();
	int i = 0;
	/* skip over initial whitespace */
	while (IS_WHITESPACE(pc[i])) i++;

	while (pc[i]!=0) {
		m_wordCount++;
		m_wordStart.insert(m_wordStart.end(), i+1);
		while (!IS_WHITESPACE(pc[i]) && pc[i]!=0) i++;
		m_wordEnd.insert(m_wordEnd.end(), i);
		while (IS_WHITESPACE(pc[i])) i++;
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
	const char* pc = currRecord()->data();
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
	MYASSERT(idx!=0);
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
	MYASSERT(idx!=0);
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

	if (idx==0) {
		idx = m_fieldCount;
	} else if (idx < 0) {
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
	if (m_inputStation != STATION_SECOND) {
		MYASSERT_WITH_MSG(NULL!=m_ps,"Tried to read record in run-out cycle");
	}
	int slen = (int)(currRecord()->length());

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

	return SpecString::newString(currRecord(), from-1, to-from+1);
}

void ProcessingState::fieldIdentifierClear()
{
	for (const auto &pair : m_fieldIdentifiers) {
		delete pair.second;
	}
	m_fieldIdentifiers.clear();
}

void ProcessingState::fieldIdentifierStatsClear()
{
	for (const auto &pair : m_fiStatistics) {
		delete pair.second;
	}
	m_fiStatistics.clear();
}

void ProcessingState::breakValuesClear()
{
	for (const auto &pair : m_breakValues) {
		delete pair.second;
	}
	m_breakValues.clear();
}

void ProcessingState::fieldIdentifierSet(char id, PSpecString ps)
{
	if (m_fieldIdentifiers[id]!=NULL) {
		std::string err = std::string("Field Identifier <") + id + "> redefined.";
		MYTHROW(err);
	}

	m_fieldIdentifiers[id] = SpecStringCopy(ps);

	// Count the statistics of this field value.
	if (ALUFUNC_STATISTICAL & AluFunction::functionTypes()) {
		if (m_fiStatistics[id]==NULL) {
			m_fiStatistics[id] = new AluValueStats(id);
		} else {
			m_fiStatistics[id]->AddValue(id);
		}
	}

	if (ALUFUNC_FREQUENCY & AluFunction::functionTypes()) {
		std::string s(ps->data(), ps->length());
		if (m_freqMaps[id]==NULL) {
			m_freqMaps[id] = new frequencyMap();
		}

		auto search = m_freqMaps[id]->find(s);
		if (search == m_freqMaps[id]->end()) {
			// This string is not yet in the map
			m_freqMaps[id]->insert({s, 0});
			search = m_freqMaps[id]->find(s);
		}

		search++;
	}

	if (m_breakValues[id] && 0==ps->Compare(m_breakValues[id]->data())) return;

	if (m_breakValues[id]) delete m_breakValues[id];

	m_breakValues[id] = SpecStringCopy(ps);
	if (breakLevelGE(id, m_breakLevel)) {
		m_breakLevel = id;
	}
}

PSpecString ProcessingState::fieldIdentifierGet(char id)
{
	PSpecString ret = m_fieldIdentifiers[id];
	if (!ret) {
		std::string err = std::string("Field Identifier <") + id + "> not defined.";
		MYTHROW(err);
	}
	return ret;
}

void ProcessingState::resetBreaks()
{
	m_breakLevel = 0;
}

bool ProcessingState::breakEstablished(char id)
{
	return breakLevelGE(m_breakLevel, id);
}

PAluValueStats ProcessingState::valueStatistics(char id)
{
	return m_fiStatistics[id];
}

PFrequencyMap ProcessingState::getFrequencyMap(char id)
{
	return m_freqMaps[id];
}

bool ProcessingState::runningOutLoop()
{
	return (!m_Loops.empty() && (LOOP_CONDITION_FALSE == m_Loops.top()));
}

bool ProcessingState::needToEvaluate()
{
	return (((m_Conditions.empty()) || (m_Conditions.top() == bTrue)) && !runningOutLoop());
}

void ProcessingState::setCondition(bool isTrue)
{
	MYASSERT(m_Conditions.empty() || (bTrue == m_Conditions.top()));
	m_Conditions.push(isTrue ? bTrue : bFalse);
}

void ProcessingState::observeIf()
{
	if (runningOutLoop()) return;
	MYASSERT(!m_Conditions.empty());
	MYASSERT((bFalse == m_Conditions.top()) || (bDontCare == m_Conditions.top()));
	m_Conditions.push(bDontCare);
}

void ProcessingState::observeWhile()
{
	m_Loops.push(LOOP_CONDITION_FALSE);
}

void ProcessingState::observeDone()
{
	MYASSERT(!m_Loops.empty() && LOOP_CONDITION_FALSE == m_Loops.top());
	m_Loops.pop();
}

int ProcessingState::getLoopStart()
{
	MYASSERT(!m_Loops.empty() && LOOP_CONDITION_FALSE != m_Loops.top());
	int ret = m_Loops.top();
	m_Loops.pop();
	return ret;
}

void ProcessingState::observeElse()
{
	if (runningOutLoop()) return;
	MYASSERT(!m_Conditions.empty());
	switch (m_Conditions.top()) {
	case bTrue:
		m_Conditions.top() = bFalse;
		break;
	case bFalse:
		m_Conditions.top() = bTrue;
		break;
	default:
		;
	}
}

void ProcessingState::observeElseIf(bool& evaluateCond)
{
	if (runningOutLoop()) return;
	MYASSERT(!m_Conditions.empty());
	switch (m_Conditions.top()) {
	case bTrue:
		m_Conditions.top() = bDontCare;  // in a series of if..elseif..elseif, once true, never again
		evaluateCond = false;
		break;
	case bFalse:
		m_Conditions.pop();  // the next setCondition will replace the entry
		evaluateCond = true;
		break;
	default:
		// leave the bDontCare where it is.
		evaluateCond = false;
		break;
	}
}

void ProcessingState::observeEndIf()
{
	if (runningOutLoop()) return;
	MYASSERT(!m_Conditions.empty());
	m_Conditions.pop();
}


// Helper class for the ALU
std::string ProcessingStateFieldIdentifierGetter::Get(char id)
{
	PSpecString ret = m_ps->fieldIdentifierGet(id);
	return std::string(ret->data(), ret->length());
}
