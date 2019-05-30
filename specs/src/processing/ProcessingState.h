#ifndef SPECS2016__PROCESSING__PROCESSINGSTATE__H
#define SPECS2016__PROCESSING__PROCESSINGSTATE__H

#include <vector>
#include <stack>
#include <map>
#include "processing/Writer.h"
#include "utils/alu.h"
#include "utils/aluFunctions.h"

#define DEFAULT_PAD_CHAR ' '
#define LOCAL_WHITESPACE  -1
#define DEFAULT_WORDSEPARATOR ' '
#define DEFAULT_FIELDSEPARATOR '\t'

#define STATION_FIRST  -1
#define STATION_SECOND -2
#define STATION_STDERR  0

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
	void    setStringInPlace(PSpecString ps);

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
	virtual ALUInt  getRecordCount()    { return ALUInt(m_CycleCounter + m_ExtraReads); }
	virtual ALUInt  getIterationCount() { return ALUInt(m_CycleCounter); }
	virtual bool    breakEstablished(char id);
	virtual PAluValueStats valueStatistics(char id);
	virtual PFrequencyMap  getFrequencyMap(char id);

	void fieldIdentifierSet(char id, PSpecString ps);
	void incrementCycleCounter() { m_CycleCounter++; }
	void incrementExtraReads()   { m_ExtraReads++; }
	PSpecString fieldIdentifierGet(char id);
	bool fieldIdentifierIsSet(char id);
	PSpecString extractCurrentRecord();
	void fieldIdentifierClear();
	void fieldIdentifierStatsClear();
	void breakValuesClear();
	void resetBreaks();
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
	void setFirst();
	void setSecond();
	void setStream(int i);
	int  getActiveInputStation() { return m_inputStation; }
	PSpecString currRecord() { return (m_inputStation==STATION_FIRST) ? m_ps : m_prevPs; }
	bool recordNotAvailable() { return NULL==currRecord(); }
	bool inputStreamHasChanged() { return m_inputStreamChanged; }
	void resetInputStreamFlag() { m_inputStreamChanged = false; }
	int  getActiveInputStream() { return m_inputStream; }
	void setWriters(PWriter* p)  {m_Writers = p; m_outputIndex = 1; }
	void setActiveWriter(int idx);
	Writer* getCurrentWriter();
	void setNoWrite()            { m_bNoWrite = true;  }
	void resetNoWrite()          { m_bNoWrite = false; }
	bool shouldWrite()           { return !m_bNoWrite; }
private:
	enum extremeBool {
		bFalse,
		bTrue,
		bDontCare
	};
	char    m_pad;
	char    m_wordSeparator;
	char    m_fieldSeparator;
	PSpecString m_ps;  // The current record
	PSpecString m_prevPs; // The previous record
	int  m_wordCount;
	int  m_fieldCount;
	unsigned int m_CycleCounter;
	unsigned int m_ExtraReads;
	std::vector<int> m_wordStart;
	std::vector<int> m_wordEnd;
	std::vector<int> m_fieldStart;
	std::vector<int> m_fieldEnd;
	void identifyWords();
	void identifyFields();
	std::map<char,PSpecString> m_fieldIdentifiers;
	std::map<char,PAluValueStats> m_fiStatistics;
	std::map<char,PSpecString> m_breakValues;
	std::map<char,PFrequencyMap> m_freqMaps;
	char m_breakLevel;
	std::stack<extremeBool> m_Conditions;
	std::stack<int> m_Loops;    // The unsigned int holds the number of the token where the while was
	int             m_inputStation;
	int             m_inputStream;
	bool            m_inputStreamChanged;
	PWriter         *m_Writers;
	int             m_outputIndex;
	bool            m_bNoWrite;
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
