#ifndef SPECS2016__PROCESSING__WRITER__H
#define SPECS2016__PROCESSING__WRITER__H

#include <string>
#include <thread>
#include "StringQueue.h"

class Writer {
public:
	Writer();
	virtual ~Writer();
	virtual void Write(pstr ps);
	virtual void Begin();
	virtual void WriteOut() = 0;
	void         End();
	bool         Done();
protected:
	bool  m_ended;
	StringQueue m_queue;
	std::thread *mp_thread;
};

class SimpleWriter : public Writer {
public:
	virtual void WriteOut();
};

#endif
