#ifndef SPECS2016__UTILS__TRACE__H
#define SPECS2016__UTILS__TRACE__H

#include <ostream>

// Destination stream for trace output (--trace-*).
std::ostream& traceStream();

// Redirect trace output to a file (--trace-file / -d). Returns false
// if the file could not be opened for writing.
bool openTraceStream(const std::string& filename);

// Redirect trace output temporarily to an internal 
// string buffer.
void pushTraceStream();

// End redirection to string buffer, returning the string
std::string popTraceStream();

// Accumulate strings into a large buffer
void traceAccumulateStrings(const std::string& s);

// Return the accumulated strings, clearing the buffer
std::string traceReturnAccumulatedStrings();

#define TRACE_RUN if (g_bDebugSpecsRun) traceStream()

#define TRACE_COMP if (g_bDebugSpecsCompile) traceStream()

#endif // SPECS2016__UTILS__TRACE__H
