#ifndef SPECS2016__PROCESSING__WRITER__H
#define SPECS2016__PROCESSING__WRITER__H

#include <string>
#include <thread>
#include <fstream>
#include "utils/StringQueue.h"

class Writer {
public:
	Writer();
	virtual ~Writer();
	virtual void Write(PSpecString ps);
	virtual void Begin();
	virtual void WriteOut() = 0;
	void         End();
	bool         Done();
	unsigned long 		countGenerated() { return m_countGenerated; }
	unsigned long 		countWritten()   { return m_countWritten; }
protected:
	unsigned long m_countGenerated;
	unsigned long m_countWritten;
	bool  m_ended;
	StringQueue m_queue;
	std::thread *mp_thread;
};

class SimpleWriter : public Writer {
public:
	SimpleWriter();
	SimpleWriter(std::string& fn);
	virtual ~SimpleWriter();
	virtual void WriteOut();
private:
	std::ostream* m_File;
	bool m_NeedToClose;
};

#endif
