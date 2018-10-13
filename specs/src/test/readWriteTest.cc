#include "../processing/Reader.h"
#include "../processing/Writer.h"

int main(int argc, char** argv)
{
	Reader* pRead = new StandardReader();
	Writer* pWrite = new SimpleWriter();

	pRead->begin();
	pWrite->Begin();

	while (!pRead->eof()) {
		pWrite->Write(pRead->get());
	}

	pWrite->End();

	delete pRead;
	delete pWrite;

	return 0;
}
