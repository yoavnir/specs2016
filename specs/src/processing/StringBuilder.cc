#include <cstring> // for memcpy
#include <algorithm>
#include "Config.h"
#include "utils/ErrorReporting.h"
#include "utils/utf8.h"
#include "ProcessingState.h"
#include "StringBuilder.h"

StringBuilder::StringBuilder()
{
	mp_str = nullptr;
	m_pos = 1;
	m_strAscii = -1;
}

StringBuilder::~StringBuilder()
{
}
PSpecString StringBuilder::GetStringUnsafe()
{
	if (!mp_str) return nullptr;
	PSpecString pRet = mp_str;
	mp_str = nullptr;
	m_pos = 1;
	m_strAscii = -1;
	return pRet;
}

PSpecString StringBuilder::GetString()
{
	if (!mp_str) return mkSpecString();
	PSpecString pRet = mp_str;
	mp_str = nullptr;
	m_pos = 1;
	m_strAscii = -1;
	return pRet;
}

bool StringBuilder::strIsAscii()
{
	if (m_strAscii < 0) {
		m_strAscii = utf8::isAscii(*mp_str) ? 1 : 0;
	}
	return m_strAscii == 1;
}

void StringBuilder::insert(PSpecString s, size_t offset)
{
	MYASSERT(offset>0);
	if (!mp_str) {
		mp_str = mkSpecString();
	}
	offset--;  // translate it to C-style offsets

	// Character and byte offsets coincide unless something is non-ASCII
	if (g_bSupportUTF8 &&
			(m_pad.size()!=1 || (m_pad[0] & 0x80) || !utf8::isAscii(*s) || !strIsAscii())) {
		size_t curLen = utf8::charCount(*mp_str);
		size_t sLen = utf8::charCount(*s);
		if (curLen < offset) {
			MYASSERT(!m_pad.empty());
			*mp_str += SpecString_Pad(m_pad, offset - curLen);
			curLen = offset;
		}
		size_t bStart = utf8::charToByte(*mp_str, offset);
		size_t bEnd = utf8::charToByte(*mp_str, std::min(offset + sLen, curLen));
		mp_str->replace(bStart, bEnd - bStart, *s);
		m_pos = offset + sLen + 1;
		m_strAscii = -1;
		return;
	}

	// Here the pad is a single byte, so padding is a plain resize
	size_t endPos = offset + s->length();
	if (mp_str->length() < endPos) {
		MYASSERT(m_pad.size()==1);
		mp_str->resize(endPos, m_pad[0]);
	}
	memcpy((void*)(mp_str->data()+offset), (void*)(s->data()), s->length());
	m_pos = endPos + 1;
	if (!g_bSupportUTF8) m_strAscii = -1;  // in UTF-8 mode we only get here with ASCII
}

void StringBuilder::insertNext(PSpecString s)
{
	insert(s, m_pos);
}

void StringBuilder::insertNextWord(PSpecString s)
{
	static PSpecString pSpace = mkSpecString(" ");
	if (s->length()==0) return;
	if (Length() > 0)
		insert(pSpace,m_pos);
	insert(s, m_pos);
}

void StringBuilder::insertNextField(PSpecString s)
{
	static PSpecString pTab = mkSpecString("\t");
	insert(pTab,m_pos);
	insert(s, m_pos);
}
