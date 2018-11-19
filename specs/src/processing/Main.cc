#include "cli/tokens.h"
#include "specitems/specItems.h"
#include "processing/StringBuilder.h"
#include "processing/Reader.h"
#include "processing/Writer.h"
#include "utils/ErrorReporting.h"

bool parseSwitches(int& argc, char**& argv)
{
	int argumentCount = 0;
	/* Skip the program name */
	argc--; argv++; argumentCount++;

	while (argv[0][0]=='-' && (argv[0][1]<'1' || argv[0][1]>'9')) {
		fprintf(stderr, "Invalid switch at position %d: %s\n", argumentCount, argv[0]);
		return false;
	}

	return true;
}

int main (int argc, char** argv)
{
	if (!parseSwitches(argc, argv)) { // also skips the program name
		return -4;
	}

	std::vector<Token> vec = parseTokens(argc, argv);
	normalizeTokenList(&vec);
	itemGroup ig;
	StringBuilder sb;
	ProcessingState ps;
	StandardReader *pRd;
	SimpleWriter *pWr;

	unsigned int index = 0;
	try {
		ig.Compile(vec, index);
	}  catch (const SpecsException& e) {
		std::cerr << "Error while parsing command-line arguments:\n"
				<< e.what() << "\n\nProcessing stopped at index " << index
				<< '/' << vec.size() << ":\n";
		for (int i=0; i<vec.size(); i++) {
			std::cerr << i << ". " << vec[i].Debug() << "\n";
		}
		std::cerr << "\n" << ig.Debug();
		exit (0);
	}

#ifdef DEBUG
	std::cerr << "After parsing, index = " << index << "/" << vec.size() << "\n";

	std::cerr << ig.Debug();
#endif

	pRd = new StandardReader();
	pWr = new SimpleWriter;

	pRd->Begin();
	pWr->Begin();

	ig.process(sb, ps, *pRd, *pWr);

	pRd->End();
	delete pRd;
	pWr->End();
	delete pWr;
	vec.clear();

	return 0;
}
