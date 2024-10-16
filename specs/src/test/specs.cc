#include <iomanip>
#include <cmath>
#include <ctime>
#include <string.h>
#include "utils/platform.h"
#include "cli/tokens.h"
#include "processing/Config.h"
#include "processing/persistent.h"
#include "specitems/specItems.h"
#include "processing/StringBuilder.h"
#include "processing/Reader.h"
#include "processing/Writer.h"
#include "utils/TimeUtils.h"
#include "utils/ErrorReporting.h"
#include "utils/PythonIntf.h"
#include "utils/aluRegex.h"
#include "utils/directives.h"

extern int g_stop_stream;
extern char g_printonly_rule;

std::string getNextArg(std::string& argName, int& argc, char**& argv)
{
	static std::string emptyString;
	argc--; argv++;
	if (argc>0) {
		return std::string(argv[0]);
	} else {
		if (argName=="help") {
			return argName;
		}
		return emptyString;
	}
}

#define NEXTARG getNextArg(argName,argc, argv)
#define X(nm,typ,defval,ssw,cliswitch,oval) \
	if (0==strcmp(argv[0], "--"#cliswitch) ||       \
			0==strcmp(argv[0], "-"#ssw)) {  		\
		argName = #nm;								\
		g_##nm = oval;								\
		goto CONTINUE;								\
	}

bool parseSwitches(int& argc, char**& argv)
{
	std::string argName;
	/* Skip the program name */
	std::string programName(argv[0]);
	argc--; argv++;

	while (argc>0) {
		if (argv[0][0]!='-') break;

		CONFIG_PARAMS

		std::cerr << "Unrecognized switch <" << argv[0] << ">\n";
		return false;

CONTINUE:
		if (g_configuredString != "") {
			auto equalsPos = g_configuredString.find('=');
			if (std::string::npos == equalsPos || 1 == equalsPos) {
				std::cerr << "Malformed parameter for --set: '" << g_configuredString << "'\n";
				return false;
			}
			auto key = g_configuredString.substr(0,equalsPos);
			auto value = g_configuredString.substr(equalsPos+1);
			configSpecLiteralSet(key,value);
			g_configuredString = "";
		}
		argc--; argv++;
	}

	// sanity checks
	if (g_recfm!="") {
		if (g_recfm=="d" || g_recfm=="D" || g_recfm=="delimited" || g_recfm=="DELIMITED") {
			g_recfm = "";
		} else if (g_recfm=="f" || g_recfm=="F" || g_recfm=="fixed" || g_recfm=="FIXED") {
			g_recfm = "F";
			if (g_lrecl <= 0) {
				std::cerr << "For the 'fixed' record format, lrecl must be set to a positive value\n";
				return false;
			}
		} else if (g_recfm=="fd" || g_recfm=="FD"){
			g_recfm = "FD";
			if (g_lrecl <= 0) {
				std::cerr << "For the 'fix-delimited' record format, lrecl must be set to a positive value\n";
				return false;
			}
		} else {
			std::cerr << "Invalid record format: <" << g_recfm << ">\n";
			return false;
		}
	}

	// info
	if (g_info) {
		std::cerr << "specs invoked as '" << programName << "'\n";
#ifndef __VERSION__
#define __VERSION__ "Unknown"
#endif
		std::cerr << "\tCompiler version: " << __VERSION__ << "\n";
		std::cerr << "\tHigh/low watermark for queues: " << QUEUE_HIGH_WM << " / " << QUEUE_LOW_WM << "\n";
		std::cerr << "\tRandom Provider: " << RandomProvider << "\n";
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#ifdef GITTAG
		std::cerr << "\tGit tag: " << STRINGIFY(GITTAG) << "\n";
#endif
		std::cerr << "\tPython version: " << STRINGIFY(PYTHON_FULL_VER) << "\n";
		std::cerr << "\tFloating point precision: " << ALUFloatPrecision << " (" << sizeof(ALUFloat) << " bytes)\n";
		exit(0);
	}

	// help
	if (g_help!="") {
#ifndef SPECS_NO_PYTHON
		readConfigurationFile();
		try {
			p_gExternalFunctions->Initialize(getFullSpecPath());
		} catch (const SpecsException& e) {
			std::cerr << "Python Interface: " << e.what(!g_bVerbose) << "\n";
			exit(0);
		}
#endif
		if (g_help == "help") {
			std::cerr << "specs help:\n";
			std::cerr << "\tspecs --help help    -- this list\n";
#ifndef SPECS_NO_PYTHON
			std::cerr << "\tspecs --help pyfuncs -- lists the available python functions\n";
#endif
			std::cerr << "\tspecs --help builtin -- lists the available builtin functions\n";
			std::cerr << "\tspecs --help specs   -- lists the available specifications\n";
			std::cerr << "\tspecs --help <name>  -- provides arguments and doc for a particular python or builtin function, or specification\n";
			return false;
		}
		if (g_help == "pyfuncs") {
#ifdef SPECS_NO_PYTHON
			std::cerr << "Python functions are not supported in this build.\n";
#else
			p_gExternalFunctions->Debug();
#endif
		} else if (g_help == "builtin") {
			aluFunc_help_builtin();
		} else if (g_help == "specs") {
			dumpSpecificationsList();
		} else if ((false == aluFunc_help_one_builtin(g_help)) && 
#ifdef SPECS_NO_PYTHON
				(true))
#else
				(false == p_gExternalFunctions->DebugOne(g_help)) &&
				(false == dumpSpecificationsList(g_help)))
#endif
		{
			std::cerr << "specs: I don't know anything about " << g_help << std::endl;
		}
		return false;
	}

	return true;
}

int main (int argc, char** argv)
{
	classifyingTimer timer;
	bool conciseExceptions = true;

	if (argc==1) { // Called without parameters
		std::cerr << "Usage: " << argv[0] << " (switches & arguments)\n\n";
#ifndef WIN64
		std::cerr << "For more information, type 'man specs'\n";
#endif
		return -4;
	}
	
	if (!parseSwitches(argc, argv)) { // also skips the program name
		return -4;
	}

	readConfigurationFile();
	persistentVarLoad();

	if (g_timeZone != "") {
		specTimeSetTimeZone(g_timeZone);
	}

#ifdef DEBUG
	conciseExceptions = !g_bVerbose;
#endif

	try {
		if (g_regexSyntaxType != "") {
			setRegexType(g_regexSyntaxType);
		}
	} catch (const SpecsException& e) {
		std::cerr << "Error: " << e.what(conciseExceptions) << std::endl;
		exit(0);
	}

#ifndef SPECS_NO_PYTHON
	if ("" != g_pythonErr) {
		try {
			p_gExternalFunctions->SetErrorHandling(g_pythonErr);
		} catch (const SpecsException& e) {
			std::cerr << "Python Interface: " << e.what(!g_bVerbose) << "\n";
			exit(0);
		}
	}

	if (EXTERNAL_FUNC_ON == g_pythonFuncs) {
		try {
			p_gExternalFunctions->Initialize(getFullSpecPath());
		} catch (const SpecsException& e) {
			std::cerr << "Python Interface: " << e.what(!g_bVerbose) << "\n";
			exit(0);
		}
#ifdef DEBUG
		p_gExternalFunctions->Debug();
#endif
	}
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
	PReader pRd = nullptr;
	PSimpleWriter pWrtrs[MAX_INPUT_STREAMS+1]; // zero will be stderr

	setStateQueryAgent(&ps);
	setPositionGetter(&sb);

	// memset(pWrtrs, 0, sizeof(void*) * (1 + MAX_INPUT_STREAMS));

	unsigned int index = 0;
	try {
		ig.Compile(vec, index);
	}  catch (const SpecsException& e) {
		std::cerr << "Error while parsing command-line arguments:\n" << e.what(conciseExceptions) << "\n";
		if (g_bVerbose) {
			std::cerr << "\nProcessing stopped at index " << index
					<< '/' << vec.size() << ":\n";
			for (size_t i=0; i<vec.size(); i++) {
				std::cerr << i+1 << ". " << vec[i].Debug() << "\n";
			}
			std::cerr << "\n" << ig.Debug();
		}
		exit (0);
	}

	// After the compilation, the token vector contents are no longer necessary
	for (size_t i=0; i<vec.size(); i++) vec[i].deallocDynamic();
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

	if (!g_outputFile.empty() && g_bShellCmd)  {  // These should not both be specified
		std::cerr << "Error: Cannot specify both --shell and --outfile\n";
		exit(0);
	}
	if (g_bShellCmd) {
		pWrtrs[1] = std::make_shared<SimpleWriter>(SimpleWriter::writerType__SHELL);
	} else if (!g_outputFile.empty()) {
		pWrtrs[1] = std::make_shared<SimpleWriter>(g_outputFile);
	} else {
		pWrtrs[1] = std::make_shared<SimpleWriter>(SimpleWriter::writerType__COUT);
	} 

	pWrtrs[0] = std::make_shared<SimpleWriter>(SimpleWriter::writerType__CERR);

	if (g_outputStream2 != "") pWrtrs[2] = std::make_shared<SimpleWriter>(g_outputStream2);
	if (g_outputStream3 != "") pWrtrs[3] = std::make_shared<SimpleWriter>(g_outputStream3);
	if (g_outputStream4 != "") pWrtrs[4] = std::make_shared<SimpleWriter>(g_outputStream4);
	if (g_outputStream5 != "") pWrtrs[5] = std::make_shared<SimpleWriter>(g_outputStream5);
	if (g_outputStream6 != "") pWrtrs[6] = std::make_shared<SimpleWriter>(g_outputStream6);
	if (g_outputStream7 != "") pWrtrs[7] = std::make_shared<SimpleWriter>(g_outputStream7);
	if (g_outputStream8 != "") pWrtrs[8] = std::make_shared<SimpleWriter>(g_outputStream8);

	for (int i=0; i <= MAX_INPUT_STREAMS ; i++) {
		if (pWrtrs[i]) pWrtrs[i]->Begin();
	}
	ps.setWriters((PWriter*)pWrtrs);

	if (g_incmd != "") {
		auto p = execCmd(g_incmd);
		if (!p) {
			std::cerr << "Error: Failed to run the following command:\n   " << g_incmd << "\n";
			return -4;
		}
		setPrimaryInputPipe(p);
	}


	if (ig.readsLines() || g_bForceFileRead) {
		if (g_inputFile.empty()) {
			if (nullptr == primaryInputPipe()) {
				pRd = std::make_shared<StandardReader>();
			} else {
				pRd = std::make_shared<StandardReader>(primaryInputPipe());
			}
		} else {
			try {
				pRd = std::make_shared<StandardReader>(g_inputFile);
			} catch (const SpecsException& e) {
				std::cerr << "Error: Failed to open input file: " << e.what(!g_bVerbose) << "\n";
				exit(0);
			}
			if (nullptr != primaryInputPipe()) {
				std::cerr << "Error: Both input file and input stream specified.\n";
				exit(0);
			}
		}

		if (g_recfm=="F") {
			pRd->setFormatFixed(g_lrecl,false);
		} else if (g_recfm=="FD") {
			pRd->setFormatFixed(g_lrecl,true);
		}

		if (g_linedel!="") {
			pRd->setLineDelimiter(g_linedel[0]);
		}

		if (anyNonPrimaryInputStreamDefined()) {
			PMultiReader pmRd = std::make_shared<multiReader>(pRd);
			if (g_inputStream2 != "") pmRd->addStream(2, g_inputStream2);
			if (g_inputStream3 != "") pmRd->addStream(3, g_inputStream3);
			if (g_inputStream4 != "") pmRd->addStream(4, g_inputStream4);
			if (g_inputStream5 != "") pmRd->addStream(5, g_inputStream5);
			if (g_inputStream6 != "") pmRd->addStream(6, g_inputStream6);
			if (g_inputStream7 != "") pmRd->addStream(7, g_inputStream7);
			if (g_inputStream8 != "") pmRd->addStream(8, g_inputStream8);
			pmRd->setStopReader(g_stop_stream);
			pRd = pmRd;
		}


		pRd->Begin();

		timer.changeClass(timeClassProcessing);

		try {
			ig.process(sb, ps, *pRd, timer);
		} catch (const SpecsException& e) {
			if (!e.isAbend()) {
				std::cerr << "Runtime error after reading " << pRd->countRead() << " lines and using " << pRd->countUsed() <<".\n";
			}
			std::cerr << e.what(conciseExceptions) << std::endl;
			pRd->abortRead();
			pRd->End();
			ps.fieldIdentifierStatsClear();
			for (int i=0; i<=MAX_INPUT_STREAMS; i++) {
				if (pWrtrs[i]) {
					pWrtrs[i]->End();
					pWrtrs[i] = nullptr;
				}
			}
			return -4;
		}

		pRd->End();
		readLines = pRd->countRead();
		usedLines = pRd->countUsed();
		generatedLines = 0;
		writtenLines = 0;
		if (!g_bPrintStats) {
			pRd = nullptr;
		} else {
			pRd->endCollectingTimeData();
		}
	} else {
		try {
			unsigned int readerCount = 0;
			ig.setRegularRunAtEOF();
			ig.processDo(sb, ps, nullptr, timer, readerCount);
		} catch (const SpecsException& e) {
			std::cerr << "Runtime error. ";
			std::cerr << e.what(conciseExceptions) << "\n";
			exit(8);
		}
		PSpecString pstr = sb.GetString();
		if (ps.shouldWrite() && !ps.printSuppressed(g_printonly_rule)) {
			auto pSW = std::dynamic_pointer_cast<SimpleWriter>(ps.getCurrentWriter());
			pSW->Write(pstr,timer);
		} else {
			ps.resetNoWrite();
		}
		readLines = 0;
		usedLines = 0;
		generatedLines = 1;
		writtenLines = 1;
	}

	clockValue timeAtEnd = specTimeGetTOD();
	std::clock_t clockAtEnd = clock();

	for (int i=0; i<=MAX_INPUT_STREAMS; i++) {
		if (pWrtrs[i]) {
			pWrtrs[i]->End();
			generatedLines += pWrtrs[i]->countGenerated();
			writtenLines = pWrtrs[i]->countWritten();
			if (g_bPrintStats) {
				pWrtrs[i]->endCollectingTimeData();
			} else {
				pWrtrs[i] = nullptr;
			}
		}
	}

	timer.changeClass(timeClassLast);

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
		clockValue runTimeMicroSeconds = timeAtEnd - timeAtStart;
		ALUFloat runTimeSeconds = ALUFloat(runTimeMicroSeconds) / ALUFloat(MICROSECONDS_PER_SECOND);
		std::cerr << "\nRun Time: " << runTimeSeconds << " seconds.\n";

		ALUFloat duration = (ALUFloat(1) * (clockAtEnd-clockAtStart)) / CLOCKS_PER_SEC;
		std::cerr << "CPU Time: " << std::floor(duration) << "." <<
				std::setfill('0') << std::setw(6) <<
				u_int64_t((duration-std::floor(duration)+0.5) * 1000000) << " seconds.\n";
		timer.dump("Main Thread");
		if (g_bThreaded) {
			if (pRd) {
				pRd->dumpTimeData();
				pRd = nullptr;
			}
			for (int i=0; i<=MAX_INPUT_STREAMS; i++) {
				if (pWrtrs[i]) {
					if (1 == i) {
						pWrtrs[i]->dumpTimeData();
					}
					pWrtrs[i] = nullptr;
				}
			}
		}

		dumpRegexStats();
	}

	persistentVarSaveIfNeeded();

	return 0;
}
