#include <iostream>
#include "utils/ErrorReporting.h"
#include "Writer.h"

void WriteAllRecords(Writer *pw)
{
	pw->startProcessing();
	while (!pw->Done()) {
		pw->WriteOut();
	}
	pw->startDraining();
}

Writer::Writer()
{
	mp_thread = NULL;
	m_ended = false;
	m_countGenerated = 0;
	m_countWritten = 0;
}

Writer::~Writer()
{
	if (mp_thread && mp_thread->joinable()) {
		mp_thread->join();
	}
	mp_thread = NULL;
}

void Writer::Write(PSpecString ps)
{
	m_queue.push(ps);
	m_countGenerated++;
}

void Writer::Begin()
{
	mp_thread = std::unique_ptr<std::thread>(new std::thread(WriteAllRecords, this));
}

void Writer::End()
{
	m_ended = true;
	m_queue.Done();
	if (mp_thread) {
		mp_thread->join();
		mp_thread = NULL;
	}
}

bool Writer::Done()
{
	return m_ended && m_queue.empty();
}

SimpleWriter::SimpleWriter() {
	m_File = &std::cout;
	m_NeedToClose = false;
}

SimpleWriter::SimpleWriter(const std::string& fn) {
	static const std::string _stderr = WRITER_STDERR;
	if (fn == _stderr) {
		m_File = &std::cerr;
		m_NeedToClose = false;
	} else {
		std::ofstream* pOutFile = new std::ofstream(fn);
		m_File = pOutFile;
		if (!pOutFile->is_open()) {
			std::string err = "Could not open output file " + fn;
			MYTHROW(err);
		}
		m_NeedToClose = true;
	}
}

SimpleWriter::~SimpleWriter() {
	if (m_NeedToClose) {
		std::ofstream* pOutFile = dynamic_cast<std::ofstream*>(m_File);
		pOutFile->clear();
		delete pOutFile;
	}
}

void SimpleWriter::WriteOut()
{
	PSpecString ps = NULL;
	m_Timer.changeClass(timeClassInputQueue);
	bool res = m_queue.wait_and_pop(ps);
	if (res) {
		m_Timer.changeClass(timeClassIO);
		*m_File << *ps << std::endl;
		m_Timer.changeClass(timeClassProcessing);
		m_countWritten++;
	} else {
		m_Timer.changeClass(timeClassProcessing);
	}
}
