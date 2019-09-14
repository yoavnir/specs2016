#ifndef SPECS2016__PROCESSING__READER__H
#define SPECS2016__PROCESSING__READER__H

#include <fstream>
#include "utils/StringQueue.h"

class Reader {
public:
	Reader() {mp_thread = NULL; m_countRead = m_countUsed = 0; m_pUnreadString = NULL; m_bAbort = false; m_bRanDry = false;}
	virtual ~Reader();
	virtual void        selectStream(unsigned char idx);
	virtual bool        endOfSource() = 0;
	virtual PSpecString getNextRecord() = 0;
	virtual PSpecString get();
	void                pushBack(PSpecString ps);
	virtual void        readIntoQueue();
	virtual void        Begin();
	virtual bool        eof() { return endOfSource() && m_queue.empty(); }
	void                End();
	void                abortRead() { m_bAbort = true;    }
	unsigned long 		countRead() { return m_countRead; }
	unsigned long 		countUsed() { return m_countUsed; }
	bool                hasRunDry() { return m_bRanDry;   }
	virtual void        setFormatFixed(unsigned int lrecl, bool blocked) {
		MYTHROW("Reader::setFormatFixed: should not be called");
	}
	virtual void        setLineDelimiter(char c) {
		MYTHROW("Reader::setLineDelimiter: should not be called");
	}
protected:
	StringQueue m_queue;
	std::thread *mp_thread;
	PSpecString   m_pUnreadString;
	unsigned long m_countRead;
	unsigned long m_countUsed;
	bool          m_bAbort;
	bool          m_bRanDry;    // true *after* the reader returned NULL once
};

class TestReader : public Reader {
public:
	TestReader(size_t maxLineCount);
	virtual ~TestReader();
	void    InsertString(const char* s);
	void    InsertString(PSpecString ps);
	virtual bool endOfSource() {return m_bAbort || (m_idx >= m_count); }
	virtual PSpecString getNextRecord() {return SpecStringCopy(mp_arr[m_idx++]);}
	virtual PSpecString get() {return getNextRecord();}
private:
	PSpecString  *mp_arr;
	size_t       m_count;
	size_t       m_idx;
	size_t       m_MaxCount;
};

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
	virtual ~StandardReader();
	virtual bool endOfSource();
	virtual PSpecString getNextRecord();
	virtual void setFormatFixed(unsigned int lrecl, bool blocked);
	virtual void setLineDelimiter(char c);
private:
	std::istream* m_File;
    char* m_buffer;
	bool  m_EOF;
	bool  m_NeedToClose;
	recordFormat m_recfm;
	unsigned int m_lrecl;
	char         m_lineDelimiter;
};

#define MAX_INPUT_STREAMS  8
#define DEFAULT_READER_IDX 1 // externally. Internally it is stored as zero

class multiReader : public Reader {
public:
	multiReader(Reader* pDefaultReader);   // Please don't initiate with another multiReader...
	virtual ~multiReader();
	void addStream(unsigned char idx, std::istream* f);
	void addStream(unsigned char idx, std::string& fn);
	virtual void selectStream(unsigned char idx, PSpecString* ppRecord);
	virtual bool        endOfSource();
	virtual PSpecString getNextRecord();
	virtual PSpecString get();
	virtual void        Begin();
	void                End();
	unsigned int        getReaderIdx()  { return readerIdx+1; }
private:
	Reader*             readerArray[MAX_INPUT_STREAMS];
	PSpecString         stringArray[MAX_INPUT_STREAMS];
	unsigned int        readerIdx;
	unsigned int        maxReaderIdx;
	bool                bFirstGet;
};


#endif
