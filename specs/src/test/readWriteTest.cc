#include "processing/Reader.h"
#include "processing/Writer.h"
#include "processing/StringBuilder.h"

int main(int argc, char** argv)
{
	StringBuilder sb;
	Reader* pRead = new StandardReader();
	Writer* pWrite = new SimpleWriter();

	pRead->Begin();
	pWrite->Begin();

	while (!pRead->eof()) {
		PSpecString p = pRead->get();
		sb.insert(p,1);
		delete p;
		pWrite->Write(sb.GetString());
	}

	pWrite->End();

	delete pRead;
	delete pWrite;

	return 0;
}
