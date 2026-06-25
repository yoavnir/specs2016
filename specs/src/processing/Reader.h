#ifndef SPECS2016__PROCESSING__READER__H
#define SPECS2016__PROCESSING__READER__H

#include <deque>
#include <fstream>
#include <memory>
#include "utils/StringQueue.h"
#include "utils/TimeUtils.h"
#include "utils/directives.h"

#define STOP_STREAM_ALL     97
#define STOP_STREAM_ANY     98
#define STOP_STREAM_INVALID 99
#define IS_SPECIFIC_STREAM(x)  ((x)<STOP_STREAM_ALL)

// Sentinel pointer for out-of-bounds records
extern PSpecString g_pOOBSpecString;

class Reader {
public:
	Reader() {mp_thread = nullptr; m_countRead = m_countUsed = 0; m_pUnreadString = nullptr; m_bAbort = false; m_bRanDry = false;}
	virtual ~Reader();
	virtual void        selectStream(unsigned char idx);
	virtual bool        endOfSource() = 0;
	virtual PSpecString getNextRecord() = 0;
	virtual PSpecString get(classifyingTimer& tmr, unsigned int& _readerCounter);
	void                pushBack(PSpecString ps);
	virtual void        readIntoQueue();
	virtual void        Begin();
	virtual bool        eof() { return endOfSource() && m_queue.empty(); }
	void                End();
	void                abortRead() { m_bAbort = true;    }
	unsigned long 		countRead() { return m_countRead; }
	unsigned long 		countUsed() { return m_countUsed; }
	bool                hasRunDry() { return m_bRanDry;   }
	void                startProcessing() { m_Timer.changeClass(timeClassProcessing); }
	void                startDraining() { m_Timer.changeClass(timeClassDraining); }
	void                endCollectingTimeData() { m_Timer.changeClass(timeClassLast); }
	void                dumpTimeData()  { m_Timer.dump("Reader Thread"); m_queue.DumpStats("Reader Queue");}
	virtual void        setFormatFixed(unsigned int lrecl, bool blocked) {
		MYTHROW("Reader::setFormatFixed: should not be called");
	}
	virtual void        setLineDelimiter(char c) {
		MYTHROW("Reader::setLineDelimiter: should not be called");
	}
	virtual PSpecString peek(int offset);
	virtual void        setContextSizes(unsigned int forward, unsigned int backward) {}
	static bool         isOOBRecord(PSpecString ps) { return ps == g_pOOBSpecString; }
protected:
	StringQueue m_queue;
	std::unique_ptr<std::thread> mp_thread;
	PSpecString   m_pUnreadString;
	unsigned long m_countRead;
	unsigned long m_countUsed;
	bool          m_bAbort;
	bool          m_bRanDry;    // true *after* the reader returned nullptr once
	classifyingTimer m_Timer;
};

typedef std::shared_ptr<Reader> PReader;

class TestReader : public Reader {
public:
	explicit TestReader(size_t maxLineCount);
	~TestReader() override;
	void    InsertString(const char* s);
	void    InsertString(PSpecString ps);
	bool endOfSource() override {return m_bAbort || (m_idx >= m_count); }
	PSpecString getNextRecord() override {return mp_arr[m_idx++];}
	PSpecString get(classifyingTimer& tmr, unsigned int& _readerCounter) override {return getNextRecord();}
	PSpecString peek(int offset) override;
private:
	PSpecString  *mp_arr;
	size_t       m_count;
	size_t       m_idx;
	size_t       m_MaxCount;
};

typedef std::shared_ptr<TestReader> PTestReader;

enum recordFormat {
	RECFM_DELIMITED,
	RECFM_FIXED,
	RECFM_FIXED_DELIMITED
};
class StandardReader : public Reader {
public:
	StandardReader();	      /* simple constructor - stdin becomes the source */
	explicit StandardReader(std::istream* f);
	explicit StandardReader(std::string& fn);
	explicit StandardReader(pipeType pipe);
	~StandardReader() override;
	bool endOfSource() override;
	PSpecString getNextRecord() override;
	void setFormatFixed(unsigned int lrecl, bool blocked) override;
	void setLineDelimiter(char c) override;
	PSpecString peek(int offset) override;
	void        setContextSizes(unsigned int forward, unsigned int backward) override;
private:
	PSpecString getNextRecordInternal();
	std::shared_ptr<std::istream> m_File;
	pipeType  m_pipe;
    char* m_buffer;
	bool  m_EOF;

	recordFormat m_recfm;
	unsigned int m_lrecl;
	char         m_lineDelimiter;
	// Rolling context buffers
	unsigned int m_forwardContextSize;
	unsigned int m_backwardContextSize;
	std::deque<PSpecString> m_forwardBuffer;
	std::deque<PSpecString> m_backwardBuffer;
	PSpecString m_currentRecord;
	bool        m_contextInitialized;
};

typedef std::shared_ptr<StandardReader> PStandardReader;

#define MAX_INPUT_STREAMS  8
#define DEFAULT_READER_IDX 1 // externally. Internally it is stored as zero

class multiReader : public Reader {
public:
	explicit multiReader(PReader pDefaultReader);   // Please don't initiate with another multiReader...
	~multiReader() override;
	void addStream(unsigned char idx, std::istream* f);
	void addStream(unsigned char idx, std::string& fn);
	using Reader::selectStream;  // prevent a warning about overloading
	void selectStream(unsigned char idx, PSpecString* ppRecord);
	bool        endOfSource() override;
	PSpecString getNextRecord() override;
	PSpecString get(classifyingTimer& tmr, unsigned int& _readerCounter) override;
	void        Begin() override;
	void                End();
	unsigned int        getReaderIdx()  { return readerIdx+1; }
	void                setStopReader(int idx) { stopReaderIdx = idx; }
private:
	PReader             readerArray[MAX_INPUT_STREAMS];
	PSpecString         stringArray[MAX_INPUT_STREAMS];
	unsigned int        readerIdx;
	unsigned int        maxReaderIdx;
	bool                bFirstGet;
	unsigned int        stopReaderIdx;
	unsigned int        readerCounter;
};

typedef std::shared_ptr<multiReader> PMultiReader;

extern Reader* g_pReader;

#endif
