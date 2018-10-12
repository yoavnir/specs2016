#include <stdio.h>
#include "Writer.h"

void SimpleWriter::Write(std::string& str)
{
	printf("%s\n", str.c_str());
}

void SimpleWriter::Write(std::string* pstr)
{
	printf("%s\n", pstr->c_str());
	delete pstr;
}
