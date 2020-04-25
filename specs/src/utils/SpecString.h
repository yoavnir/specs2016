#ifndef SPECS2016__UTILS_SPECSTRING__H
#define SPECS2016__UTILS_SPECSTRING__H

#include <iostream>
#include <memory>
#include <string>

#define MAX_STR_LEN 65535
#define INVALID_LENGTH (MAX_STR_LEN+1)

enum outputAlignment {
	outputAlignmentLeft,
	outputAlignmentCenter,
	outputAlignmentRight,
	outputAlignmentComposed
};

enum ellipsisSpec {
	ellipsisSpecNone,
	ellipsisSpecLeft,
	ellipsisSpecThird,
	ellipsisSpecHalf,
	ellipsisSpecTwoThirds,
	ellipsisSpecRight
};

class SpecString;

typedef std::shared_ptr<SpecString> PSpecString;

class SpecString {
public:
	SpecString() {}   // Empty string constructor
	SpecString(const char* pstrz) {}
	SpecString(const char* pstrz, size_t len) {}
	SpecString(const SpecString& ss) {}
	static PSpecString newString();
	static PSpecString newString(const char* pstrz);
	static PSpecString newString(const char* pstrz, size_t len);
	static PSpecString newString(PSpecString pss, size_t start, size_t len);
	static PSpecString newString(std::string& st);
	virtual ~SpecString() {}
	virtual void add(PSpecString ps) = 0;
	virtual void append(PSpecString ps) = 0;
	virtual void _serialize(std::ostream& os) const = 0;
	virtual const char* data() = 0;
	virtual std::string* sdata() = 0;
	virtual size_t length() = 0;
	virtual void Overlay(PSpecString pss, size_t offset, void* pPadChar) = 0;
	virtual void Resize(size_t newSize, void* pPadChar, outputAlignment oa, ellipsisSpec es) = 0;
	virtual void Resize(size_t newSize, char padChar, outputAlignment oa, ellipsisSpec es) = 0;
	virtual int  Compare(const char* pstrz) = 0;
	virtual int  Compare(std::string& str) = 0;
};

class StdSpecString : public SpecString {
public:
	StdSpecString() : m_str("") {}
	StdSpecString(const char* pstrz) : m_str(pstrz) {}
	StdSpecString(const char* pstrz, size_t len) : m_str(pstrz, len) {}
	StdSpecString(const StdSpecString &sps) {m_str = *(sps.getStdString());}
	virtual ~StdSpecString() {}
	virtual void add(PSpecString ps);
	virtual void append(PSpecString ps);
	virtual const char* data() {return m_str.c_str();}
	virtual std::string* sdata() {return &m_str;}
	virtual size_t length() {return m_str.length();}
	virtual void Overlay(PSpecString pss, size_t offset, void* pPadChar);
	virtual void Resize(size_t newSize, void* pPadChar, outputAlignment oa, ellipsisSpec es);
	virtual void Resize(size_t newSize, char padChar, outputAlignment oa, ellipsisSpec es);
	virtual void _serialize(std::ostream& os) const;
	const std::string* getStdString() const {return &m_str;}
	virtual int  Compare(const char* pstrz) {return m_str.compare(pstrz);}
	virtual int  Compare(std::string& str) {return m_str.compare(str);}
private:
	std::string m_str;
};

typedef std::shared_ptr<StdSpecString> PStdSpecString;

std::ostream& operator<< (std::ostream& os, const SpecString &str);

PSpecString SpecStringCopy(PSpecString pss);

#endif
