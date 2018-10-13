#include "assert.h"
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

std::string *Reader::get()
{
	if (eof()) return NULL;
	pstr ret;
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
		throw std::invalid_argument("File not found " + fn);
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
		mp_NextRecord = new std::string(line,len);
	}
}

std::string* StandardReader::getNextRecord() {
	std::string* ret = mp_NextRecord;
	mp_NextRecord = NULL;
	ReadOneRecord();
	return ret;
}
