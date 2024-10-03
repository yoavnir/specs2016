#ifndef SPECS2016__PROCESSING__WRITER__H
#define SPECS2016__PROCESSING__WRITER__H

#include <string>
#include <fstream>
#include <memory>
#include "utils/StringQueue.h"
#include "utils/TimeUtils.h"

class Writer {
public:
	Writer();
	virtual ~Writer();
	virtual void Write(PSpecString ps, classifyingTimer& tmr);
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
	virtual void WriteOutDo(PSpecString, classifyingTimer& tmr);
	unsigned long m_countGenerated;
	unsigned long m_countWritten;
	bool  m_ended;
	StringQueue m_queue;
	std::unique_ptr<std::thread> mp_thread;
	classifyingTimer m_Timer;
};

typedef std::shared_ptr<Writer> PWriter;

class SimpleWriter : public Writer {
public:
	enum writerType {
		writerType__COUT,
		writerType__CERR,
		writerType__SHELL,
		writerType__FILE
	};
	SimpleWriter(writerType typ);
	SimpleWriter(const std::string& fn);
	virtual ~SimpleWriter();
	virtual void WriteOut();
private:
	virtual void WriteOutDo(PSpecString ps, classifyingTimer& tmr);
	std::shared_ptr<std::ostream> m_File;
	writerType m_WriterType;
};

typedef std::shared_ptr<SimpleWriter> PSimpleWriter;

// A writer to an internal string. 
// Only used by ProcessingTest
class StringWriter : public Writer {
public:
	StringWriter() {}
	virtual ~StringWriter() {}
	virtual void WriteOut() {}
	virtual void WriteOutDo(PSpecString ps, classifyingTimer& tmr) 
	{
		m_queue.push(ps);
	}
	virtual PSpecString getString() {
		if (!m_queue.empty()) {
			PSpecString ret;
			m_queue.wait_and_pop(ret);
			return ret;
		} else {
			return nullptr;  // SpecString::newString();
		}
	}
};

typedef std::shared_ptr<StringWriter> PStringWriter;

#endif
