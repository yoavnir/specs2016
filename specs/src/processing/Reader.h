#ifndef SPECS2016__PROCESSING__READER__H
#define SPECS2016__PROCESSING__READER__H

#include <fstream>
#include <memory>
#include "utils/StringQueue.h"
#include "utils/TimeUtils.h"
#include "utils/directives.h"

#define STOP_STREAM_ALL     97
#define STOP_STREAM_ANY     98
#define STOP_STREAM_INVALID 99
#define IS_SPECIFIC_STREAM(x)  ((x)<STOP_STREAM_ALL)

class Reader {
public:
	Reader() {mp_thread = NULL; m_countRead = m_countUsed = 0; m_pUnreadString = NULL; m_bAbort = false; m_bRanDry = false;}
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
protected:
	StringQueue m_queue;
	std::unique_ptr<std::thread> mp_thread;
	PSpecString   m_pUnreadString;
	unsigned long m_countRead;
	unsigned long m_countUsed;
	bool          m_bAbort;
	bool          m_bRanDry;    // true *after* the reader returned NULL once
	classifyingTimer m_Timer;
};

typedef std::shared_ptr<Reader> PReader;

class TestReader : public Reader {
public:
	TestReader(size_t maxLineCount);
	virtual ~TestReader();
	void    InsertString(const char* s);
	void    InsertString(PSpecString ps);
	virtual bool endOfSource() {return m_bAbort || (m_idx >= m_count); }
	virtual PSpecString getNextRecord() {return mp_arr[m_idx++];}
	virtual PSpecString get(classifyingTimer& tmr, unsigned int& _readerCounter) {return getNextRecord();}
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
	StandardReader(std::istream* f);
	StandardReader(std::string& fn);
	StandardReader(pipeType pipe);
	virtual ~StandardReader();
	virtual bool endOfSource();
	virtual PSpecString getNextRecord();
	virtual void setFormatFixed(unsigned int lrecl, bool blocked);
	virtual void setLineDelimiter(char c);
private:
	std::shared_ptr<std::istream> m_File;
	pipeType  m_pipe;
    char* m_buffer;
	bool  m_EOF;
	bool  m_NeedToClose;
	recordFormat m_recfm;
	unsigned int m_lrecl;
	char         m_lineDelimiter;
};

typedef std::shared_ptr<StandardReader> PStandardReader;

#define MAX_INPUT_STREAMS  8
#define DEFAULT_READER_IDX 1 // externally. Internally it is stored as zero

class multiReader : public Reader {
public:
	multiReader(PReader pDefaultReader);   // Please don't initiate with another multiReader...
	virtual ~multiReader();
	void addStream(unsigned char idx, std::istream* f);
	void addStream(unsigned char idx, std::string& fn);
	using Reader::selectStream;  // prevent a warning about overloading
	virtual void selectStream(unsigned char idx, PSpecString* ppRecord);
	virtual bool        endOfSource();
	virtual PSpecString getNextRecord();
	virtual PSpecString get(classifyingTimer& tmr, unsigned int& _readerCounter);
	virtual void        Begin();
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


#endif
