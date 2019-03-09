#include <iomanip>
#include <cmath>
#include <ctime>
#include <string.h>
#include "utils/platform.h"
#include "cli/tokens.h"
#include "processing/Config.h"
#include "specitems/specItems.h"
#include "processing/StringBuilder.h"
#include "processing/Reader.h"
#include "processing/Writer.h"
#include "utils/TimeUtils.h"
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
	bool conciseExceptions = true;
	readConfigurationFile();

	if (!parseSwitches(argc, argv)) { // also skips the program name
		return -4;
	}

#ifdef DEBUG
	conciseExceptions = !g_bVerbose;
#endif

	std::vector<Token> vec;

	try {
		if (g_specFile != "") {
			vec = parseTokensFile(g_specFile);
		} else {
			vec = parseTokens(argc, argv);
		}

		normalizeTokenList(&vec);
	} catch (const SpecsException& e) {
		std::cerr << "Error reading specification tokens: " << e.what(conciseExceptions) << "\n";
		exit (0);
	}
	itemGroup ig;
	StringBuilder sb;
	ProcessingState ps;
	StandardReader *pRd;
	SimpleWriter *pWr;

	setStateQueryAgent(&ps);

	unsigned int index = 0;
	try {
		ig.Compile(vec, index);
	}  catch (const SpecsException& e) {
		std::cerr << "Error while parsing command-line arguments: " << e.what(conciseExceptions) << "\n";
		if (g_bVerbose) {
			std::cerr << "\nProcessing stopped at index " << index
					<< '/' << vec.size() << ":\n";
			for (int i=0; i<vec.size(); i++) {
				std::cerr << i+1 << ". " << vec[i].Debug() << "\n";
			}
			std::cerr << "\n" << ig.Debug();
		}
		exit (0);
	}

	// After the compilation, the token vector contents are no longer necessary
	for (int i=0; i<vec.size(); i++) vec[i].deallocDynamic();
	vec.clear();

#ifdef DEBUG
	std::cerr << "After parsing, index = " << index << "/" << vec.size() << "\n";

	std::cerr << ig.Debug();
#endif

	// Connect the ALU to the processing state
	ProcessingStateFieldIdentifierGetter fiGetter(&ps);
	setFieldIdentifierGetter(&fiGetter);

	unsigned long readLines;
	unsigned long usedLines;
	unsigned long generatedLines;
	unsigned long writtenLines;
	clockValue timeAtStart = specTimeGetTOD();
	std::clock_t clockAtStart = clock();

	if (ig.readsLines() || g_bForceFileRead) {
		if (g_inputFile.empty()) {
			pRd = new StandardReader();
		} else {
			pRd = new StandardReader(g_inputFile);
		}

		if (g_outputFile.empty()) {
			pWr = new SimpleWriter();
		} else {
			pWr = new SimpleWriter(g_outputFile);
		}

		pRd->Begin();
		pWr->Begin();

		try {
			ig.process(sb, ps, *pRd, *pWr);
		} catch (const SpecsException& e) {
			std::cerr << "Runtime error after reading " << pRd->countRead() << " lines and using " << pRd->countUsed() <<".\n";
			std::cerr << e.what(conciseExceptions) << "\n";
			pRd->End();
			delete pRd;
			pWr->End();
			delete pWr;
			return -4;
		}

		pRd->End();
		readLines = pRd->countRead();
		usedLines = pRd->countUsed();
		delete pRd;
		pWr->End();
		generatedLines = pWr->countGenerated();
		writtenLines = pWr->countWritten();
		delete pWr;
	} else {
		TestReader tRead(5);
		try {
			ig.setRegularRunAtEOF();
			ig.processDo(sb, ps, &tRead, NULL);
		} catch (const SpecsException& e) {
			std::cerr << "Runtime error. ";
			std::cerr << e.what(conciseExceptions) << "\n";
			return -4;
		}
		PSpecString ps = sb.GetString();
		std::cout << *ps << "\n";
		delete ps;
		readLines = 0;
		usedLines = 0;
		generatedLines = 1;
		writtenLines = 1;
	}

	clockValue timeAtEnd = specTimeGetTOD();
	std::clock_t clockAtEnd = clock();

	if (g_bPrintStats) {
		std::cerr << "\n";
		std::cerr << "Read  " << readLines << " lines.";
		if (readLines!=usedLines) {
			std::cerr << " " << usedLines << "were used.";
		}
		std::cerr << "\nWrote " << generatedLines << " lines.";
		if (readLines!=usedLines) {
			std::cerr << " " << writtenLines << "were written out.";
		}
		clockValue runTimeSeconds = timeAtEnd - timeAtStart;
		std::cerr << "\nRun Time: " << runTimeSeconds << " seconds.\n";

		ALUFloat duration = (ALUFloat(1) * (clockAtEnd-clockAtStart)) / CLOCKS_PER_SEC;
		std::cerr << "CPU Time: " << std::floor(duration) << "." <<
				std::setfill('0') << std::setw(6) <<
				u_int64_t((duration-std::floor(duration)+0.5) * 1000000) << " seconds.\n";
	}

	return 0;
}
