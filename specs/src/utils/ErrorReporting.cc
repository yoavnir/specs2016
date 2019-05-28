#include <stdlib.h>
#include <string.h>
#include "ErrorReporting.h"

const char* SpecsException::what(bool concise) const throw ()
{
	static char ret[1024];
	if (concise) {
		strcpy(ret,msg.c_str());
	} else {
#ifdef DEBUG
		sprintf(ret, "\nException: %s  (at: %s:%u)",
			msg.c_str(), fn, ln);
#else
		sprintf(ret, "\nSPECS Exception: %s", msg.c_str());
#endif
	}
	return (const char*)(ret);
}

const bool SpecsException::isAbend() const throw()
{
	return (const bool)(bIsAbend);
}
