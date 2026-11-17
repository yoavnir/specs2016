#include <iostream>
#include <fstream>
#include <sstream>
#include "ErrorReporting.h"
#include "trace.h"

// Destination for ALU debug output (--trace-alu-comp / --trace-alu-run).
// Defaults to std::cerr; redirected to a file by --trace-file / -d.
static std::ofstream g_traceFile;
std::ostream* g_pTraceStream = &std::cerr;

std::ostream& traceStream()
{
	return *g_pTraceStream;
}

bool openTraceStream(const std::string& filename)
{
	g_traceFile.open(filename, std::ios::out | std::ios::trunc);
	if (!g_traceFile.is_open()) {
		return false;
	}
	g_pTraceStream = &g_traceFile;
	return true;
}

static std::ostream* pSavedStream = nullptr;

void pushTraceStream()
{
	MYASSERT(pSavedStream==nullptr);
	static std::ostringstream oss;
	oss.str("");
	oss.clear();
	pSavedStream = g_pTraceStream;
	g_pTraceStream = &oss;
}

std::string popTraceStream()
{
	MYASSERT_NOT_NULL(pSavedStream);
	std::ostringstream* pOSS = dynamic_cast<std::ostringstream*>(g_pTraceStream);
	MYASSERT_NOT_NULL(pOSS);
	auto ret = pOSS->str();
	g_pTraceStream = pSavedStream;
	pSavedStream = nullptr;
	return ret;
}

static std::string accumulatedString;

// Accumulate strings into a large buffer
void traceAccumulateStrings(const std::string& s)
{
	accumulatedString += s;
}

// Return the accumulated strings, clearing the buffer
std::string traceReturnAccumulatedStrings()
{
	auto ret = accumulatedString;
	accumulatedString.clear();
	return ret;
}
