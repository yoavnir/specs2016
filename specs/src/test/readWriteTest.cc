#include "utils/ErrorReporting.h"
#include "processing/Reader.h"
#include "processing/Writer.h"
#include "processing/StringBuilder.h"

int main(int argc, char** argv)
{
	classifyingTimer tmr;
	StringBuilder sb;
	unsigned int readerCount = 1;
	Reader* pRead = new StandardReader();
	Writer* pWrite = new SimpleWriter();

	pRead->Begin();
	pWrite->Begin();

	while (!pRead->eof()) {
		PSpecString p = pRead->get(tmr, readerCount);
		sb.insert(p,1);
		pWrite->Write(sb.GetString());
	}

	pWrite->End();

	delete pRead;
	delete pWrite;

	return 0;
}
