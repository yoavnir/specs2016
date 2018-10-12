#ifndef SPECS2016__PROCESSING__READER__H
#define SPECS2016__PROCESSING__READER__H

#include <string>
#include <stdio.h>  // for FILE

class Reader {
public:
	Reader() {}
	virtual ~Reader() {}
	virtual bool        eof() = 0;
	virtual std::string *getNextRecord() = 0;
};

class StandardReader : public Reader {
public:
	StandardReader();	      /* simple constructor - stdin becomes the source */
	StandardReader(FILE* f);
	StandardReader(std::string& fn);
	virtual ~StandardReader();
	virtual bool eof();
	virtual std::string *getNextRecord();
protected:
	void ReadOneRecord();  // after this either mp_NextRecord is set, or m_EOF is set
private:
	FILE* m_File;
	bool  m_EOF;
	bool  m_NeedToClose;
	std::string* mp_NextRecord;
};

#endif
