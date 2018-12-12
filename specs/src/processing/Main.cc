#include "cli/tokens.h"
#include "processing/Config.h"
#include "specitems/specItems.h"
#include "processing/StringBuilder.h"
#include "processing/Reader.h"
#include "processing/Writer.h"
#include "utils/ErrorReporting.h"

std::string getNextArg(int& argc, char**& argv)
{
	argc--; argv++;
	MYASSERT(argc>0);
	return std::string(argv[0]);
}

#define NEXTARG getNextArg(argc, argv)
#define X(nm,typ,defval,ssw,cliswitch,oval) \
	if (0==strcmp(argv[0], "--"#cliswitch) ||       \
			0==strcmp(argv[0], "-"#ssw)) {  		\
		g_##nm = oval;								\
		goto CONTINUE;								\
	}

bool parseSwitches(int& argc, char**& argv)
{
	/* Skip the program name */
	argc--; argv++;

	while (argc>0) {
		if (argv[0][0]!='-') break;

		CONFIG_PARAMS

CONTINUE:
		argc--; argv++;
	}

	return true;
}

int main (int argc, char** argv)
{
	readConfigurationFile();

	if (!parseSwitches(argc, argv)) { // also skips the program name
		return -4;
	}

	std::vector<Token> vec;
	if (g_specFile != "") {
		vec = parseTokensFile(g_specFile);
	} else {
		vec = parseTokens(argc, argv);
	}

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
		std::cerr << "Error while parsing command-line arguments: " << e.what(true) << "\n";
		if (g_bVerbose) {
			std::cerr<< "\nProcessing stopped at index " << index
					<< '/' << vec.size() << ":\n";
			for (int i=0; i<vec.size(); i++) {
				std::cerr << i+1 << ". " << vec[i].Debug() << "\n";
			}
			std::cerr << "\n" << ig.Debug();
		}
		exit (0);
	}

#ifdef DEBUG
	std::cerr << "After parsing, index = " << index << "/" << vec.size() << "\n";

	std::cerr << ig.Debug();
#endif

	// Connect the ALU to the processing state
	ProcessingStateFieldIdentifierGetter fiGetter(&ps);
	setFieldIdentifierGetter(&fiGetter);

	if (ig.readsLines() || g_bForceFileRead) {
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
	} else {
		TestReader tRead(5);
		ig.processDo(sb, ps, &tRead, NULL);
		std::cout << *sb.GetString() << "\n";
	}

	return 0;
}
