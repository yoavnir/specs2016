#include <string.h>
#include <fstream>
#include "utils/ErrorReporting.h"
#include "Reader.h"

void ReadAllRecordsIntoReaderQueue(Reader* r)
{
	while (!r->endOfSource()) {
		r->readIntoQueue();
	}
}

Reader::~Reader()
{
	End();
}

void Reader::End()
{
	if (mp_thread) {
		mp_thread->join();
	}
	delete mp_thread;
	mp_thread = NULL;
}

PSpecString Reader::get()
{
	PSpecString ret;
	if (m_pUnreadString) {
		ret = m_pUnreadString;
		m_pUnreadString = NULL;
		return ret;
	}
	if (eof()) {
		m_bRanDry = true;
		return NULL;
	}
	if (m_queue.wait_and_pop(ret)) {
		m_countUsed++;
		return ret;
	} else {
		m_bRanDry = true;
		return NULL;
	}
}

void Reader::pushBack(PSpecString ps)
{
	MYASSERT_WITH_MSG(m_pUnreadString==NULL, "Only one record can be UNREAD at a time");
	m_pUnreadString = ps;
}

void Reader::readIntoQueue()
{
	if (!endOfSource()) {
		PSpecString nextRecord = getNextRecord();
		if (nextRecord) {
			m_queue.push(nextRecord);
			m_countRead++;
		} else {
			m_queue.Done();
		}
	}
}

void Reader::Begin() {
	mp_thread = new std::thread(ReadAllRecordsIntoReaderQueue, this);
}


StandardReader::StandardReader() {
	m_File = &std::cin;
	m_NeedToClose = false;
	m_EOF = false;
	m_buffer = (char*)malloc(STANDARD_READER_BUFFER_SIZE);
}

StandardReader::StandardReader(std::istream* f) {
	MYASSERT(f!=NULL);
	m_EOF = false;
	if (!f->good()) {  // so it crashes if what we've been passed is not a stream pointer
		m_EOF = true;
	}
	m_File = f;
	m_NeedToClose = false;
	m_buffer = (char*)malloc(STANDARD_READER_BUFFER_SIZE);
}

StandardReader::StandardReader(std::string& fn) {
	std::ifstream* pInputFile = new std::ifstream(fn);
	m_File = pInputFile;
	if (!pInputFile->is_open()) {
		std::string err = "File not found: " + fn;
		MYTHROW(err);
	}
	m_NeedToClose = true;
	m_EOF = false;
	m_buffer = (char*)malloc(STANDARD_READER_BUFFER_SIZE);
}

StandardReader::~StandardReader() {
	if (m_NeedToClose) {
		std::ifstream* pInputFile = dynamic_cast<std::ifstream*>(m_File);
		pInputFile->close();
		delete pInputFile;
	}
	free(m_buffer);
}

bool StandardReader::endOfSource() {
	return m_EOF;
}

PSpecString StandardReader::getNextRecord() {
	std::string line;
	if (!std::getline(*m_File, line)) {
		m_EOF = true;
		return NULL;
	} else {
		// strip trailing newline if any
		if (line.back() == '\n') {
			line.pop_back();
		}
		return SpecString::newString(line);
	}
}

TestReader::TestReader(size_t maxLineCount)
{
	mp_arr = (SpecString**)malloc(sizeof(PSpecString) * maxLineCount);
	m_count = m_idx = 0;
	m_MaxCount = maxLineCount;
}

TestReader::~TestReader()
{
	if (mp_arr) {
		size_t i;
		for (i=0; i<m_count; i++) {
			delete mp_arr[i];
		}
		free(mp_arr);
	}
}

void TestReader::InsertString(const char* s)
{
	if (m_count >= m_MaxCount) {
		MYTHROW("Attempting to insert too many lines into TestReader");
	}
	mp_arr[m_count++] = SpecString::newString(s);
}

void TestReader::InsertString(PSpecString ps)
{
	if (m_count >= m_MaxCount) {
		MYTHROW("Attempting to insert too many lines into TestReader");
	}
	mp_arr[m_count++] = ps;
}
