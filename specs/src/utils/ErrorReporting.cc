#include <stdlib.h>
#include "ErrorReporting.h"

const char* SpecsException::what() const throw ()
{
	static char ret[1024];
#ifdef DEBUG
	sprintf(ret, "\nException: %s  (at: %s:%u)",
			msg.c_str(), fn, ln);
#else
	sprintf(ret, "\nSPECS Exception: %s", msg.c_str());
#endif
	return (const char*)(ret);
}
