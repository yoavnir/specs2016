#ifndef SPECS2016__UTILS__TRACE__H
#define SPECS2016__UTILS__TRACE__H

#include <sstream>
#include <fstream>

// Destination stream for ALU debug output (--debug-alu-comp / --debug-alu-run).
std::ostream& traceStream();

// Redirect ALU debug output to a file (--debug-alu-file / -d). Returns false
// if the file could not be opened for writing.
bool openTraceStream(const std::string& filename);


#endif // SPECS2016__UTILS__TRACE__H
