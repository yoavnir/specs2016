#ifndef SPECS2016__PROCESSING__WRITER__H
#define SPECS2016__PROCESSING__WRITER__H

#include <string>

class Writer {
public:
	Writer() {}
	virtual ~Writer() {}
	virtual void Write(std::string* pstr) = 0;  // also deallocates the string
	virtual void Write(std::string& str) = 0;  // does not deallocated the string
};

class SimpleWriter : public Writer {
public:
	SimpleWriter() {}
	virtual ~SimpleWriter() {}
	virtual void Write(std::string* pstr);
	virtual void Write(std::string& str);
};

#endif
