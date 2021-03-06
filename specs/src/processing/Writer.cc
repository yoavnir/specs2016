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
	m_WriterType = writerType__COUT;
}

SimpleWriter::SimpleWriter(const std::string& fn) {
	static const std::string _stderr = WRITER_STDERR;
	if (fn == _stderr) {
		m_WriterType = writerType__CERR;
	} else {
		auto pOutFile = std::make_shared<std::ofstream>(fn);
		m_File = pOutFile;
		if (!pOutFile->is_open()) {
			std::string err = "Could not open output file " + fn;
			MYTHROW(err);
		}
		m_WriterType = writerType__FILE;
	}
}

SimpleWriter::~SimpleWriter() {
	if (writerType__FILE == m_WriterType) {
		std::shared_ptr<std::ofstream> pOutFile = std::dynamic_pointer_cast<std::ofstream>(m_File);
		if (pOutFile) pOutFile->clear();
		m_File = NULL;
	}
}

void SimpleWriter::WriteOut()
{
	PSpecString ps = NULL;
	m_Timer.changeClass(timeClassInputQueue);
	bool res = m_queue.wait_and_pop(ps);
	if (res) {
		m_Timer.changeClass(timeClassIO);
		switch (m_WriterType) {
			case writerType__COUT:
				std::cout << *ps << '\n';
				break;
			case writerType__CERR:
				std::cerr << *ps << '\n';
				break;
			case writerType__FILE:
				*m_File << *ps << '\n';
		}
		m_Timer.changeClass(timeClassProcessing);
		m_countWritten++;
	} else {
		m_Timer.changeClass(timeClassProcessing);
	}
}
