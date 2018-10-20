#ifndef SPECS2016__PROCESSING__STRINGBUILDER__H
#define SPECS2016__PROCESSING__STRINGBUILDER__H

#include <string>
#include "ProcessingState.h"

class StringBuilder {
public:
	StringBuilder();
	~StringBuilder();
	std::string*   GetString();
	size_t         Length() { return mp_str->length(); }
	void           insert(std::string* str, size_t offset, bool bOnlyPhysical=false);
	void           insertNext(std::string* str);
	void           insertNextWord(std::string* str);
private:
	ProcessingState m_ps;
	std::string     *mp_str;
};

#endif
