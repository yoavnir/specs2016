#ifndef SPECS2016__UTILS__DIRECTIVES__H
#define SPECS2016__UTILS__DIRECTIVES__H

#include <string>
#include <memory>

typedef std::shared_ptr<FILE> pipeType;

pipeType execCmd(std::string& cmd);

void processPlusDirective(std::string& dirline);

pipeType primaryInputPipe();

void setPrimaryInputPipe(pipeType p);

#endif
