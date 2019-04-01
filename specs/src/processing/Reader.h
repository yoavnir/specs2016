#ifndef SPECS2016__PROCESSING__READER__H
#define SPECS2016__PROCESSING__READER__H

#include <fstream>
#include "utils/StringQueue.h"

class Reader {
public:
	Reader() {mp_thread = NULL; m_countRead = m_countUsed = 0; m_pUnreadString = NULL; m_bRanDry = false;}
	virtual ~Reader();
	virtual bool        endOfSource() = 0;
	virtual PSpecString getNextRecord() = 0;
	virtual PSpecString get();
	void                pushBack(PSpecString ps);
	virtual void        readIntoQueue();
	virtual void        Begin();
	virtual bool        eof() { return endOfSource() && m_queue.empty(); }
	void                End();
	unsigned long 		countRead() { return m_countRead; }
	unsigned long 		countUsed() { return m_countUsed; }
	bool                hasRunDry() { return m_bRanDry;   }
protected:
	StringQueue m_queue;
	std::thread *mp_thread;
	PSpecString   m_pUnreadString;
	unsigned long m_countRead;
	unsigned long m_countUsed;
	bool          m_bRanDry;    // true *after* the reader returned NULL once
};

class TestReader : public Reader {
public:
	TestReader(size_t maxLineCount);
	virtual ~TestReader();
	void    InsertString(const char* s);
	void    InsertString(PSpecString ps);
	virtual bool endOfSource() {return m_idx >= m_count;}
	virtual PSpecString getNextRecord() {return SpecStringCopy(mp_arr[m_idx++]);}
	virtual PSpecString get() {return getNextRecord();}
private:
	PSpecString  *mp_arr;
	size_t       m_count;
	size_t       m_idx;
	size_t       m_MaxCount;
};

#define STANDARD_READER_BUFFER_SIZE 65536  /* This is also the max size for an input record */

class StandardReader : public Reader {
public:
	StandardReader();	      /* simple constructor - stdin becomes the source */
	StandardReader(std::istream* f);
	StandardReader(std::string& fn);
	virtual ~StandardReader();
	virtual bool endOfSource();
	virtual PSpecString getNextRecord();
private:
	std::istream* m_File;
    char* m_buffer;
	bool  m_EOF;
	bool  m_NeedToClose;
};

#endif
