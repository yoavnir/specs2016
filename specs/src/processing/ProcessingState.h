#ifndef SPECS2016__PROCESSING__PROCESSINGSTATE__H
#define SPECS2016__PROCESSING__PROCESSINGSTATE__H

#include <vector>
#include <stack>
#include <map>
#include "processing/Writer.h"
#include "utils/alu.h"
#include "utils/aluFunctions.h"

#define DEFAULT_PAD_CHAR ' '
#define LOCAL_WHITESPACE  ""
#define DEFAULT_WORDSEPARATOR " "
#define DEFAULT_FIELDSEPARATOR "\t"

#define STATION_FIRST  -1
#define STATION_SECOND -2
#define STATION_STDERR  0

#define LOOP_CONDITION_FALSE (-5)

#define PRINTONLY_PRINTALL  '\0'
#define PRINTONLY_EOF       '_'

class ProcessingState : public stateQueryAgent {
public:
	ProcessingState();
	ProcessingState(ProcessingState* pPS);
	ProcessingState(ProcessingState& ps);
	~ProcessingState();
	void    Reset();

	void    setPadChar(char c) {m_pad = c;}
	void    setWSChars(const std::string& c) {m_wordSeparator = c; m_wordSeparatorLocal = c.empty(); m_wordCount=-1;}
	void    setFSChars(const std::string& c) {m_fieldSeparator = c; m_fieldCount=-1;}

	char    getPadChar() { return m_pad;            }
	std::string& getWSChars()  { return m_wordSeparator;  }
	std::string& getFSChars()  { return m_fieldSeparator; }

	void    setString(PSpecString ps, bool bResetState = true);
	void    setStringInPlace(PSpecString ps);

	// The stateQueryAgent interface
	unsigned int getWordCount() override;
	unsigned int getFieldCount() override;
	int     getWordStart(int idx) override;
	int     getWordEnd(int idx) override;
	int     getFieldStart(int idx) override;
	int     getFieldEnd(int idx) override;
	PSpecString getFromTo(int from, int to) override;
	bool    isRunIn() override   { return (m_CycleCounter==1); }
	bool    isRunOut() override  { return (m_ps==nullptr); } // NOTE: will return true before first record
	bool    isEOF() override     { return m_bEOF; }
	ALUInt  getRecordCount() override    { return ALUInt(m_CycleCounter + m_ExtraReads); }
	ALUInt  getContextOffset() override  { return ALUInt(m_contextOffset); }
	ALUInt  getIterationCount() override { return ALUInt(m_CycleCounter); }
	bool    breakEstablished(char id) override;
	PAluValueStats valueStatistics(char id) override;
	PFrequencyMap  getFrequencyMap(char id) override;

	void fieldIdentifierSet(char id, PSpecString ps);
	void incrementCycleCounter() { m_CycleCounter++; }
	void incrementExtraReads()   { m_ExtraReads++; }
	PSpecString fieldIdentifierGet(char id);
	bool fieldIdentifierIsSet(char id) override;
	PSpecString extractCurrentRecord();
	void fieldIdentifierClear();
	void fieldIdentifierStatsClear();

	void alterFieldSeparator(const std::string& sep) override;
	std::string& getFieldSeparator() override;
	void alterWordSeparator(const std::string& sep) override;
	std::string& getWordSeparator() override;

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
	void setContextString(PSpecString ps, int offset = 0);
	int  getActiveInputStation() { return m_inputStation; }
	PSpecString currRecord() override { return (m_inputStation==STATION_FIRST) ? m_ps : m_prevPs; }
	PSpecString inputRecord() override { return m_inputRecord; }
	bool recordNotAvailable() { return nullptr==currRecord(); }
	bool inputStreamHasChanged() { return m_inputStreamChanged; }
	void resetInputStreamFlag() { m_inputStreamChanged = false; }
	int  getActiveInputStream() { return m_inputStream; }
	void setWriters(PWriter* p)  {m_Writers = p; m_outputIndex = 1; }
	void setActiveWriter(int idx);
	PWriter getCurrentWriter();
	void setNoWrite()            { m_bNoWrite = true;  }
	void resetNoWrite()          { m_bNoWrite = false; }
	bool shouldWrite()           { return !m_bNoWrite; }
	bool printSuppressed(char printRule);
	void setEOF()                { m_bEOF = true;      }
private:
	enum extremeBool {
		bFalse,
		bTrue,
		bDontCare
	};
	char    m_pad;
	bool m_wordSeparatorLocal;
	std::string m_wordSeparator;
	std::string m_fieldSeparator;
	PSpecString m_ps;  // The current record
	PSpecString m_prevPs; // The previous record
	PSpecString m_inputRecord; // The real input record (unaffected by CONTEXT)
	int  m_wordCount;
	int  m_fieldCount;
	unsigned int m_CycleCounter;
	unsigned int m_ExtraReads;
	int  m_contextOffset;
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
	bool            m_bEOF;
};


// Helper class for the ALU
class ProcessingStateFieldIdentifierGetter : public fieldIdentifierGetter {
public:
	ProcessingStateFieldIdentifierGetter(ProcessingState* _ps) : m_ps(_ps) {}
	~ProcessingStateFieldIdentifierGetter() override                               {}
	std::string Get(char id) override;
private:
	ProcessingState*	m_ps;
};

#endif
