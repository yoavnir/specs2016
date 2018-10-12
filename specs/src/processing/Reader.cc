#include "assert.h"
#include "Reader.h"

//class StandardReader : public Reader {
//public:
//	StandardReader();	      /* simple constructor - stdin becomes the source */
//	StandardReader(FILE* f);
//	StandardReader(std::string& fn);
//	virtual ~StandardReader();
//	virtual bool eof();
//	virtual std::string getNextRecord();
//private:
//	FILE* m_File;
//};

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

bool StandardReader::eof() {
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
