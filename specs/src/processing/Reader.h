#ifndef SPECS2016__PROCESSING__READER__H
#define SPECS2016__PROCESSING__READER__H

#include <stdio.h>  // for FILE
#include "utils/StringQueue.h"

class Reader {
public:
	Reader() {}
	virtual ~Reader();
	virtual bool        endOfSource() = 0;
	virtual PSpecString getNextRecord() = 0;
	virtual PSpecString get();
	virtual void        readIntoQueue();
	virtual void        begin();
	virtual bool        eof() { return endOfSource() && m_queue.empty(); }
protected:
	StringQueue m_queue;
	std::thread *mp_thread;
};

class TestReader : public Reader {
public:
	TestReader(void* arr, size_t count, size_t szEntry);
	virtual bool endOfSource() {return m_idx >= m_count;}
	virtual PSpecString getNextRecord() {return SpecStringCopy(mp_arr[m_idx++]);}
private:
	PSpecString  *mp_arr;
	size_t       m_count;
	size_t       m_idx;
};

class StandardReader : public Reader {
public:
	StandardReader();	      /* simple constructor - stdin becomes the source */
	StandardReader(FILE* f);
	StandardReader(std::string& fn);
	virtual ~StandardReader();
	virtual bool endOfSource();
	virtual PSpecString getNextRecord();
protected:
	void ReadOneRecord();  // after this either mp_NextRecord is set, or m_EOF is set
private:
	FILE* m_File;
	bool  m_EOF;
	bool  m_NeedToClose;
	PSpecString mp_NextRecord;
};

#endif
