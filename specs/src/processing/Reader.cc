#include "assert.h"
#include "../utils/ErrorReporting.h"
#include "Reader.h"

void ReadAllRecordsIntoReaderQueue(Reader* r)
{
	while (!r->endOfSource()) {
		r->readIntoQueue();
	}
}

Reader::~Reader()
{
	if (mp_thread) {
		mp_thread->join();
	}
}

PSpecString Reader::get()
{
	if (eof()) return NULL;
	PSpecString ret;
	m_queue.wait_and_pop(ret);
	return ret;
}

void Reader::readIntoQueue()
{
	if (!endOfSource()) {
		m_queue.push(getNextRecord());
	}
}

void Reader::begin() {
	mp_thread = new std::thread(ReadAllRecordsIntoReaderQueue, this);
}


StandardReader::StandardReader() {
	m_File = stdin;
	m_NeedToClose = false;
	m_EOF = false;
	mp_NextRecord = NULL;
	ReadOneRecord();
}

StandardReader::StandardReader(FILE* f) {
	assert(f!=NULL);
	feof(f);  // so it crashes if what we've been passed is not a FILE pointer
	m_File = f;
	m_NeedToClose = false;
	m_EOF = false;
	mp_NextRecord = NULL;
	ReadOneRecord();
}

StandardReader::StandardReader(std::string& fn) {
	m_File = fopen(fn.c_str(), "r");  // In future, allow binary.
	if (!m_File) {
		std::string err = "File not found: " + fn;
		MYTHROW(err);
	}
	m_NeedToClose = true;
	m_EOF = false;
	mp_NextRecord = NULL;
	ReadOneRecord();
}

StandardReader::~StandardReader() {
	if (m_NeedToClose) {
		fclose(m_File);
	}
}

bool StandardReader::endOfSource() {
	return m_EOF;
}

void StandardReader::ReadOneRecord() {
	assert(mp_NextRecord==NULL);
	size_t len;
	char* line = fgetln(m_File, &len);
	if (!line) {
		m_EOF = true;
	} else {
		// strip trailing newline if any
		if (line[len-1]=='\n') {
			len--;
		}
		mp_NextRecord = SpecString::newString(line,len);
	}
}

PSpecString StandardReader::getNextRecord() {
	PSpecString ret = mp_NextRecord;
	mp_NextRecord = NULL;
	ReadOneRecord();
	return ret;
}

TestReader::TestReader(void* arr, size_t count, size_t szEntry)
{
	int i;
	mp_arr = (SpecString**)malloc(sizeof(PSpecString) * count);
	for (i=0; i<count; i++) {
		mp_arr[i] = (PSpecString)arr;
		arr = (void*)(((char*)arr) + szEntry);
	}
	m_count = count;
	m_idx = 0;
}
