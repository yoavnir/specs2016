#include <stdlib.h>
#include <string.h>
#include "ErrorReporting.h"

#define RETSIZE 1024

const char* SpecsException::what(bool concise) const throw ()
{
	static char ret[RETSIZE];
	if (concise) {
		strcpy(ret,msg.c_str());
	} else {
#ifdef DEBUG
		snprintf(ret, RETSIZE, "\nException: %s  (at: %s:%u)",
			msg.c_str(), fn, ln);
#else
		snprintf(ret, RETSIZE, "\nSPECS Exception: %s", msg.c_str());
#endif
	}
	return (const char*)(ret);
}

const bool SpecsException::isAbend() const throw()
{
	return (const bool)(bIsAbend);
}
