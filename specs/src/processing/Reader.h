#ifndef SPECS2016__PROCESSING__READER__H
#define SPECS2016__PROCESSING__READER__H

#include <string>
#include <stdio.h>  // for FILE
#include "StringQueue.h"

class Reader {
public:
	Reader() {}
	virtual ~Reader();
	virtual bool        endOfSource() = 0;
	virtual std::string *getNextRecord() = 0;
	virtual std::string *get();
	virtual void        readIntoQueue();
	virtual void        begin();
	virtual bool        eof() { return endOfSource() && m_queue.empty(); }
protected:
	StringQueue m_queue;
	std::thread *mp_thread;
};

class StandardReader : public Reader {
public:
	StandardReader();	      /* simple constructor - stdin becomes the source */
	StandardReader(FILE* f);
	StandardReader(std::string& fn);
	virtual ~StandardReader();
	virtual bool endOfSource();
	virtual std::string *getNextRecord();
protected:
	void ReadOneRecord();  // after this either mp_NextRecord is set, or m_EOF is set
private:
	FILE* m_File;
	bool  m_EOF;
	bool  m_NeedToClose;
	std::string* mp_NextRecord;
};

#endif
