#ifndef SPECS2016__UTILS__TRACE__H
#define SPECS2016__UTILS__TRACE__H

#include <ostream>

// Destination stream for ALU debug output (--trace-alu-comp / --trace-alu-run).
std::ostream& traceStream();

// Redirect ALU debug output to a file (--trace-file / -d). Returns false
// if the file could not be opened for writing.
bool openTraceStream(const std::string& filename);


#endif // SPECS2016__UTILS__TRACE__H
