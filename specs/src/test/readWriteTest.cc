#include "utils/ErrorReporting.h"
#include "processing/Reader.h"
#include "processing/Writer.h"
#include "processing/StringBuilder.h"

int main(int argc, char** argv)
{
	classifyingTimer tmr;
	StringBuilder sb;
	Reader* pRead = new StandardReader();
	Writer* pWrite = new SimpleWriter();

	pRead->Begin();
	pWrite->Begin();

	while (!pRead->eof()) {
		PSpecString p = pRead->get(tmr);
		sb.insert(p,1);
		delete p;
		pWrite->Write(sb.GetString());
	}

	pWrite->End();

	delete pRead;
	delete pWrite;

	return 0;
}
