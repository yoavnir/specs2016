#include <stdlib.h>
#include "ErrorReporting.h"

const char* SpecsException::what() const throw ()
{
	static char ret[1024];
	sprintf(ret, "SPECS: In file: %s line: %u; %s",
			fn, ln, msg.c_str());
	return (const char*)(ret);
}
