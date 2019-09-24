#ifndef SPECS2016__PROCESSING__STRINGBUILDER__H
#define SPECS2016__PROCESSING__STRINGBUILDER__H

#include "utils/SpecString.h"
#include "ProcessingState.h"

class StringBuilder : public positionGetter {
public:
	StringBuilder();
	~StringBuilder();
	PSpecString    GetString();
	PSpecString    GetStringUnsafe();
	size_t         Length() { return mp_str ? mp_str->length() : 0; }
	void           insert(PSpecString str, size_t offset, bool bOnlyPhysical=false);
	void           insertNext(PSpecString str);
	void           insertNextWord(PSpecString str);
	void           insertNextField(PSpecString str);
	void           setPadChar(char c)     {m_pad = c;}
	size_t         pos()                  {return m_pos;}
private:
	PSpecString     mp_str;
	size_t          m_pos; // for Next, NextWord, NextField
	char			m_pad;
};

#endif
