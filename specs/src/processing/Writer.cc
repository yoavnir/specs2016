#include <iostream>
#include "utils/ErrorReporting.h"
#include "Writer.h"

void WriteAllRecords(Writer *pw)
{
	while (!pw->Done()) {
		pw->WriteOut();
	}
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
}

void Writer::Write(PSpecString ps)
{
	m_queue.push(ps);
	m_countGenerated++;
}

void Writer::Begin()
{
	mp_thread = new std::thread(WriteAllRecords, this);
}

void Writer::End()
{
	m_ended = true;
	m_queue.Done();
	if (mp_thread) {
		mp_thread->join();
	}
}

bool Writer::Done()
{
	return m_ended && m_queue.empty();
}

void SimpleWriter::WriteOut()
{
	PSpecString ps;
	if (m_queue.wait_and_pop(ps)) {
		std::cout << *ps << std::endl;
		m_countWritten++;
		delete ps;
	}
}
