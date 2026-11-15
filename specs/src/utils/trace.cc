#include <iostream>
#include <ostream>
#include "trace.h"

// Destination for ALU debug output (--debug-alu-comp / --debug-alu-run).
// Defaults to std::cerr; redirected to a file by --debug-alu-file / -d.
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
