#include "../processing/Reader.h"
#include "../processing/Writer.h"


int main(int argc, char** argv)
{
	Reader* pRead = new StandardReader();
	Writer* pWrite = new SimpleWriter();

	while (!pRead->eof()) {
		pWrite->Write(pRead->getNextRecord());
	}

	return 0;
}
