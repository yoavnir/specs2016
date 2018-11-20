#include "assert.h"
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
	mp_thread = NULL;
}

PSpecString Reader::get()
{
	if (eof()) return NULL;
	PSpecString ret;
	if (m_queue.wait_and_pop(ret)) {
		return ret;
	} else {
		return NULL;
	}
}

void Reader::readIntoQueue()
{
	if (!endOfSource()) {
		PSpecString nextRecord = getNextRecord();
		if (nextRecord) {
			m_queue.push(nextRecord);
		} else {
			m_queue.Done();
		}
	}
}

void Reader::Begin() {
	mp_thread = new std::thread(ReadAllRecordsIntoReaderQueue, this);
}


StandardReader::StandardReader() {
	m_File = stdin;
	m_NeedToClose = false;
	m_EOF = false;
}

StandardReader::StandardReader(FILE* f) {
	assert(f!=NULL);
	feof(f);  // so it crashes if what we've been passed is not a FILE pointer
	m_File = f;
	m_NeedToClose = false;
	m_EOF = false;
}

StandardReader::StandardReader(std::string& fn) {
	m_File = fopen(fn.c_str(), "r");  // In future, allow binary.
	if (!m_File) {
		std::string err = "File not found: " + fn;
		MYTHROW(err);
	}
	m_NeedToClose = true;
	m_EOF = false;
}

StandardReader::~StandardReader() {
	if (m_NeedToClose) {
		fclose(m_File);
	}
}

bool StandardReader::endOfSource() {
	return m_EOF;
}

PSpecString StandardReader::getNextRecord() {
	size_t len;
	char* line = fgetln(m_File, &len);
	if (!line) {
		m_EOF = true;
		return NULL;
	} else {
		// strip trailing newline if any
		if (line[len-1]=='\n') {
			len--;
		}
		return SpecString::newString(line,len);
	}
}

TestReader::TestReader(size_t maxLineCount)
{
	mp_arr = (SpecString**)malloc(sizeof(PSpecString) * maxLineCount);
	m_count = m_idx = 0;
	m_MaxCount = maxLineCount;
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
