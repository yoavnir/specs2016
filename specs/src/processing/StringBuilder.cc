#include <assert.h>
#include <string.h> // for memcpy
#include "StringBuilder.h"

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

