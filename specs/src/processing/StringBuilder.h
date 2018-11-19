#ifndef SPECS2016__PROCESSING__STRINGBUILDER__H
#define SPECS2016__PROCESSING__STRINGBUILDER__H

#include "utils/SpecString.h"
#include "ProcessingState.h"

class StringBuilder {
public:
	StringBuilder();
	~StringBuilder();
	PSpecString    GetString();
	size_t         Length() { return mp_str->length(); }
	void           insert(PSpecString str, size_t offset, bool bOnlyPhysical=false);
	void           insertNext(PSpecString str);
	void           insertNextWord(PSpecString str);
	void           insertNextField(PSpecString str);
private:
	PSpecString     mp_str;
};

#endif
