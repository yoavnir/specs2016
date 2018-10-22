#ifndef SPECS2016__UTILS_SPECSTRING__H
#define SPECS2016__UTILS_SPECSTRING__H

#include <iostream>
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
	SpecString(const SpecString& ss) {}
	static PSpecString newString();
	static PSpecString newString(const char* pstrz);
	static PSpecString newString(const char* pstrz, size_t len);
	virtual ~SpecString() {}
	virtual void _serialize(std::ostream& os) const = 0;
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
	StdSpecString(const StdSpecString &sps) {m_str = *(sps.getStdString());}
	virtual ~StdSpecString() {}
	virtual const char* data() {return m_str.c_str();}
	virtual size_t length() {return m_str.length();}
	virtual void Overlay(PSpecString pss, size_t offset, void* pPadChar);
	virtual void Overlay(SpecString& ss, size_t offset, void* pPadChar);
	virtual void _serialize(std::ostream& os) const;
	const std::string* getStdString() const {return &m_str;}
private:
	std::string m_str;
};

std::ostream& operator << (std::ostream& os, const SpecString &str)
{
    str._serialize(os);

    return os;
}

#endif
