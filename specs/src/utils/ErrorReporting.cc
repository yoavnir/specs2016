#include <cstdlib>
#include <cstring>
#include "ErrorReporting.h"

#define RETSIZE 1024

const char* SpecsException::what(bool concise) const noexcept
{
	thread_local char ret[RETSIZE];
	if (concise) {
		std::strncpy(ret, msg.c_str(), RETSIZE - 1);
		ret[RETSIZE - 1] = '\0';
	} else {
#ifdef DEBUG
		std::snprintf(ret, RETSIZE, "\nException: %s  (at: %s:%u)",
			msg.c_str(), fn, ln);
#else
		std::snprintf(ret, RETSIZE, "\nSPECS Exception: %s", msg.c_str());
#endif
	}
	return ret;
}

SpecsException _mythrow(const char* _fn, unsigned int _ln, const char* _msg, bool _abend)
{
	return SpecsException(_fn, _ln, _msg, _abend);
}

SpecsException _mythrow(const char* _fn, unsigned int _ln, std::string& _msg, bool _abend)
{
	return SpecsException(_fn, _ln, _msg, _abend);
}

const bool SpecsException::isAbend() const noexcept
{
	return bIsAbend;
}
