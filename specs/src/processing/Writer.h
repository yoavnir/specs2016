#ifndef SPECS2016__PROCESSING__WRITER__H
#define SPECS2016__PROCESSING__WRITER__H

#include <string>
#include <fstream>
#include "utils/StringQueue.h"
#include "utils/TimeUtils.h"

#define WRITER_STDERR "::stderr::";

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
	void                startProcessing() { m_Timer.changeClass(timeClassProcessing); }
	void                startDraining() { m_Timer.changeClass(timeClassDraining); }
	void                endCollectingTimeData() { m_Timer.changeClass(timeClassLast); }
	void                dumpTimeData()  { m_Timer.dump("Writer Thread"); m_queue.DumpStats("Writer Queue");}
protected:
	unsigned long m_countGenerated;
	unsigned long m_countWritten;
	bool  m_ended;
	StringQueue m_queue;
	std::unique_ptr<std::thread> mp_thread;
	classifyingTimer m_Timer;
};

typedef class Writer *PWriter;

class SimpleWriter : public Writer {
public:
	SimpleWriter();
	SimpleWriter(const std::string& fn);
	virtual ~SimpleWriter();
	virtual void WriteOut();
	std::ostream* getStream() { return m_File; }
private:
	std::ostream* m_File;
	bool m_NeedToClose;
};

#endif
