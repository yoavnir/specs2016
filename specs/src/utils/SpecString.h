#ifndef SPECS2016__UTILS_SPECSTRING__H
#define SPECS2016__UTILS_SPECSTRING__H

#include <string>

#define MAX_STR_LEN 65535
#define INVALID_LENGTH (MAX_STR_LEN+1)

class SpecString;

typedef SpecString* PSpecString;

class SpecString {
public:
	SpecString() {}   // Empty string constructor
	SpecString(const char* pstrz) {}
	SpecString(const char* pstrz, size_t len) {}
	static PSpecString newString();
	static PSpecString newString(const char* pstrz);
	static PSpecString newString(const char* pstrz, size_t len);
	virtual ~SpecString() {}
	virtual const char* data() = 0;
	virtual size_t length() = 0;
	virtual void Overlay(PSpecString pss, size_t offset, void* pPadChar) = 0;
	virtual void Overlay(SpecString& ss, size_t offset, void* pPadChar) = 0;
};

class StdSpecString : public SpecString {
public:
	StdSpecString() : m_str("") {}
	StdSpecString(const char* pstrz) : m_str(pstrz) {}
	StdSpecString(const char* pstrz, size_t len) : m_str(pstrz, len) {}
	virtual ~StdSpecString() {}
	virtual const char* data() {return m_str.c_str();}
	virtual size_t length() {return m_str.length();}
	virtual void Overlay(PSpecString pss, size_t offset, void* pPadChar);
	virtual void Overlay(SpecString& ss, size_t offset, void* pPadChar);
private:
	std::string m_str;
};


#endif
