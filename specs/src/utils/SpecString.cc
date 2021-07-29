#include <string.h>  // for memcpy
#include "processing/Config.h"
#include "ErrorReporting.h"
#include "SpecString.h"

void StdSpecString::Overlay(PSpecString pss, size_t offset, void* pPadChar)
{
	PStdSpecString pSmallString = std::dynamic_pointer_cast<StdSpecString>(pss);
	MYASSERT(pSmallString!=nullptr);

	size_t endPos = offset + pSmallString->length();
	if (endPos > m_str.length()) {
		char padChar = *((char*)pPadChar);
		m_str.resize(endPos, padChar);
	}

	memcpy((void*)(m_str.c_str() + offset), pSmallString->data(), pSmallString->length());
}

// Used only for the ProcessingTest
void StdSpecString::add(PSpecString ps) {
	PStdSpecString pStringToAdd = std::dynamic_pointer_cast<StdSpecString>(ps);
	if (pStringToAdd) {
		m_str += "\n";
		m_str += *pStringToAdd->getStdString();
	}
}

void StdSpecString::append(PSpecString ps) {
	PStdSpecString pStringToAdd = std::dynamic_pointer_cast<StdSpecString>(ps);
	if (pStringToAdd) {
		m_str += *pStringToAdd->getStdString();
	}
}

void StdSpecString::Resize(size_t newSize, void* pPadChar, outputAlignment oa, ellipsisSpec es)
{
	char padChar = *((char*)pPadChar);
	Resize(newSize, padChar, oa, es);
}

void StdSpecString::Resize(size_t newSize, char padChar, outputAlignment oa, ellipsisSpec es)
{
	if ((ellipsisSpecNone != es) && (newSize < m_str.size()) && (newSize >= 3)) {
		size_t totalLength, prefixLength, suffixLength;

		totalLength = newSize - 3;

		switch (es) {
		case ellipsisSpecLeft:
			prefixLength = 0;
			suffixLength = totalLength;
			break;
		case ellipsisSpecThird:
			prefixLength = totalLength / 3;
			suffixLength = totalLength - prefixLength;
			break;
		case ellipsisSpecHalf:
			prefixLength = totalLength / 2;
			suffixLength = totalLength - prefixLength;
			break;
		case ellipsisSpecTwoThirds:
			suffixLength = totalLength / 3;
			prefixLength = totalLength - suffixLength;
			break;
		case ellipsisSpecRight:
			prefixLength = totalLength;
			suffixLength = 0;
			break;
		default:
			MYTHROW("Internal Error");
		}

		m_str = m_str.substr(0,prefixLength) + "..." + m_str.substr(m_str.length() - suffixLength);
	}

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
		return nullptr; // appease the compiler warning
	} else {
		return std::make_shared<StdSpecString>();
	}
}

PSpecString SpecString::newString(const char* pstrz)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return nullptr; // appease the compiler warning
	} else {
		return std::make_shared<StdSpecString>(pstrz);
	}
}

PSpecString SpecString::newString(const char* pstrz, size_t len)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return nullptr; // appease the compiler warning
	} else {
		return std::make_shared<StdSpecString>(pstrz, len);
	}
}

PSpecString SpecString::newString(PSpecString pss, size_t start, size_t len)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return nullptr; // appease the compiler warning
	} else {
		PStdSpecString psss = std::dynamic_pointer_cast<StdSpecString>(pss);
		MYASSERT(psss!=nullptr);
		return std::make_shared<StdSpecString>(psss->data() + start, len);
	}
}

PSpecString SpecString::newString(std::string& st)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return nullptr; // appease the compiler warning
	} else {
		return std::make_shared<StdSpecString>(st.c_str(), st.length());
	}
}

PSpecString SpecStringCopy(PSpecString pss)
{
	if (g_bSupportUTF8) {
		MYTHROW("UTF-8 not supported yet");
		return nullptr; // appease the compiler warning
	} else {
		PStdSpecString psss = std::dynamic_pointer_cast<StdSpecString>(pss);
		MYASSERT(psss!=nullptr);
		return std::make_shared<StdSpecString>(*psss);
	}
}
