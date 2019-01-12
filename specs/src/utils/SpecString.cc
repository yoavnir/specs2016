#include <string.h>  // for memcpy
#include "processing/Config.h"
#include "ErrorReporting.h"
#include "SpecString.h"

void StdSpecString::Overlay(PSpecString pss, size_t offset, void* pPadChar)
{
	StdSpecString* pSmallString = dynamic_cast<StdSpecString*>(pss);
	MYASSERT(pSmallString!=NULL);

	size_t endPos = offset + pSmallString->length();
	if (endPos > m_str.length()) {
		char padChar = *((char*)pPadChar);
		m_str.resize(endPos, padChar);
	}

	memcpy((void*)(m_str.c_str() + offset), pSmallString->data(), pSmallString->length());
}

// Used only for the ProcessingTest
void StdSpecString::add(PSpecString ps) {
	StdSpecString* pStringToAdd = dynamic_cast<StdSpecString*>(ps);
	if (pStringToAdd) {
		m_str += "\n";
		m_str += *pStringToAdd->getStdString();
	}
}

void StdSpecString::Overlay(SpecString& ss, size_t offset, void* pPadChar)
{
	Overlay(&ss, offset, pPadChar);
}

void StdSpecString::Resize(size_t newSize, void* pPadChar, outputAlignment oa)
{
	char padChar = *((char*)pPadChar);
	Resize(newSize, padChar, oa);
}

void StdSpecString::Resize(size_t newSize, char padChar, outputAlignment oa)
{
	if (oa!=outputAlignmentLeft) {
		int diffSize = ((int)newSize - (int)m_str.size());
		if (diffSize == 0) return;
		if (diffSize < 0) {
			if (oa==outputAlignmentRight) {
				// Cut off enough bytes from the start
				m_str = m_str.substr(size_t(-diffSize));
				return;
			} else { // oa==outputAlignmentCenter
				// Cut off just a half
				m_str = m_str.substr(size_t(-diffSize/2));
			}
		} else { // diffSize > 0
			if (oa==outputAlignmentRight) {
				// Pad to the right
				m_str = std::string(size_t(diffSize), padChar) + m_str;
				return;
			} else { // oa==outputAlignmentCenter
				// Pad just half the amount and don't return
				m_str = std::string(size_t(diffSize/2), padChar) + m_str;
			}
		}
	}

	m_str.resize(newSize, padChar);
}

void StdSpecString::_serialize(std::ostream& os) const
{
	static char asciiXlate[] = "................................ !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~................................................................................................................................";
	if (g_bOutputTranslatedAscii) {
		for (size_t i = 0; i < m_str.length(); i++) os << asciiXlate[(unsigned char)(m_str[i])];
	} else {
		os << m_str;
	}
}

PSpecString SpecString::newString()
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return NULL; // appease the compiler warning
	} else {
		return new StdSpecString();
	}
}

PSpecString SpecString::newString(const char* pstrz)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return NULL; // appease the compiler warning
	} else {
		return new StdSpecString(pstrz);
	}
}

PSpecString SpecString::newString(const char* pstrz, size_t len)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return NULL; // appease the compiler warning
	} else {
		return new StdSpecString(pstrz, len);
	}
}

PSpecString SpecString::newString(PSpecString pss, size_t start, size_t len)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return NULL; // appease the compiler warning
	} else {
		StdSpecString *psss = dynamic_cast<StdSpecString*>(pss);
		MYASSERT(psss!=NULL);
		return new StdSpecString(psss->data() + start, len);
	}
}

PSpecString SpecString::newString(std::string& st)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return NULL; // appease the compiler warning
	} else {
		return new StdSpecString(st.c_str(), st.length());
	}
}

PSpecString SpecStringCopy(PSpecString pss)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return NULL; // appease the compiler warning
	} else {
		StdSpecString *psss = dynamic_cast<StdSpecString*>(pss);
		MYASSERT(psss!=NULL);
		return new StdSpecString(*psss);
	}
}
