#include <assert.h>
#include <string.h> // for memcpy
#include "Config.h"
#include "StringBuilder.h"

// Some UTF-8 routines
size_t utf8Len(std::string* s)
{
	size_t physLen = s->length();
	const char* pcs = s->c_str();

	size_t ret = 0;
	size_t i = 0;
	while (i<physLen) {
		if (0==(0x80 & pcs[i])) {    // msb of byte is zero - 1 byte character
			i++;
			ret++;
		} else if (0xc0==(0xe0 & pcs[i])) {   // top three bits are 110 - 2 byte character
			i += 2;
			ret++;
		} else if (0xe0==(0xf0 & pcs[i])) {   // top nibble is 1110 - 3 byte character
			i += 3;
			ret++;
		} else {  // 4 byte character
			i += 4;
			ret++;
		}
	}
	return ret;
}

size_t phys2charOffset(std::string* s, size_t physOffset)
{
	size_t physLen = s->length();
	if (physOffset > physLen) return 0;

	const char* pcs = s->c_str();
	size_t ret = 0;
	size_t i = 0;
	while (i<physOffset) {
		if (0==(0x80 & pcs[i])) {    // msb of byte is zero - 1 byte character
			i++;
			ret++;
		} else if (0xc0==(0xe0 & pcs[i])) {   // top three bits are 110 - 2 byte character
			i += 2;
			ret++;
		} else if (0xe0==(0xf0 & pcs[i])) {   // top nibble is 1110 - 3 byte character
			i += 3;
			ret++;
		} else {  // 4 byte character
			i += 4;
			ret++;
		}
	}
	return ret;
}

size_t char2physOffset(std::string* s, size_t charOffset)
{
	size_t physLen = s->length();
	const char* pcs = s->c_str();

	size_t ret = 0;
	size_t i = 0;
	while (i<charOffset && ret<physLen) {
		if (0==(0x80 & pcs[i])) {    // msb of byte is zero - 1 byte character
			ret++;
			i++;
		} else if (0xc0==(0xe0 & pcs[i])) {   // top three bits are 110 - 2 byte character
			ret += 2;
			i++;
		} else if (0xe0==(0xf0 & pcs[i])) {   // top nibble is 1110 - 3 byte character
			ret += 3;
			i++;
		} else {  // 4 byte character
			ret += 4;
			i++;
		}
	}
	if (i==charOffset) {
		return ret;
	}
	return 0;
}

StringBuilder::StringBuilder()
{
	mp_str = NULL;
}

StringBuilder::~StringBuilder()
{
	if (mp_str) {
		delete mp_str;
	}
}

std::string* StringBuilder::GetString()
{
	std::string* pRet = mp_str;
	mp_str = NULL;
	return pRet;
}

void StringBuilder::insert(std::string* s, size_t offset)
{
	assert(offset>0);
	if (!mp_str) {
		mp_str = new std::string;
	}
	offset--;  // translate it to C-style offsets

	if (g_bSupportUTF8) {
		size_t physOffset = char2physOffset(mp_str, offset);
		if (!physOffset)  {  // offset is beyond where the string is
			offset = (offset-utf8Len(mp_str)) + mp_str->length();
		} else {
			offset = physOffset;
		}
	}

	size_t endPos = offset + s->length();
	if (mp_str->length() < endPos) {
		mp_str->resize(endPos, m_ps.m_pad);
	}
	memcpy((void*)(mp_str->c_str()+offset), (void*)(s->c_str()), s->length());
}

void StringBuilder::insertNext(std::string* s)
{
	insert(s, mp_str->length() + 1);
}

void StringBuilder::insertNextWord(std::string* s)
{
	size_t len = mp_str->length();
	mp_str->resize(len + s->length() + 1, m_ps.m_pad);
	insert(s, len+2);
}

