#ifndef SPECS2016__UTILS__DIRECTIVES__H
#define SPECS2016__UTILS__DIRECTIVES__H

#include <string>

typedef std::shared_ptr<FILE> pipeType;

void processPlusDirective(std::string& dirline);

pipeType primaryInputPipe();

#endif
