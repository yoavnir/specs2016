#include <array>
#include "utils/ErrorReporting.h"
#include "processing/Config.h"
#include "directives.h"

static const char whitespace[] = " \t\n\r";

// Return the first whitespace-delimited token and assign the rest to str
static std::string breakOffOneToken(std::string& str)
{
	// Find first non-whitespace
	auto start = str.find_first_not_of(whitespace);
	MYASSERT(std::string::npos != start);

	// Find next whitespace
	auto finish = str.find_first_of(whitespace, start);

	auto ret = str.substr(start, finish-start);

	// Find non-whitespace after that
	if (std::string::npos != finish) {
		finish = str.find_first_not_of(whitespace,finish);
	}

	if (std::string::npos != finish) {
		str.erase(0,finish);
	} else {
		str = "";
	}

	return ret;
}

// Run a command and return the output
//typedef std::unique_ptr<FILE, decltype(&pclose)> pipeType;
#define MAX_LINE_LENGTH 65536
static pipeType execCmd(std::string& cmd)
{
	pipeType pipe(popen(cmd.c_str(), "r"), pclose);
	return pipe;
}

static pipeType g_primaryInput;

pipeType primaryInputPipe()
{
	return g_primaryInput;
}

/*
 * Function: processPlusDirective
 * Processes a string that contains a "plus" directive
 *
 * Input: dir - a string containing the plus directive
 *
 * Returns: nothing. Asserts if the line is not a valid directive
 */
void processPlusDirective(std::string& dirline)
{
	std::array<char, MAX_LINE_LENGTH> buffer;
	std::string directive;
	try {
		directive = breakOffOneToken(dirline);
	} catch (SpecsException& e) {
		std::string err = "Invalid directive: <" + dirline + ">";
		MYTHROW(err);
	}

	if (directive=="+SET") {
		std::string variable;
		try {
			variable = breakOffOneToken(dirline);
		} catch (SpecsException& e) {
			std::string err = "Invalid +SET directive: <" + dirline + ">";
			MYTHROW(err);
		}

		auto pipe = execCmd(dirline);
		if (!pipe) {
			std::string err = "While processing +SET directive for variable " + variable + ", failed to run command: " + dirline;
			MYTHROW(err);
		}

		std::string line("");
		if (nullptr != fgets(buffer.data(), buffer.size(), pipe.get())) {
			line = buffer.data();
			while (line.back()=='\n' || line.back()=='\r') {
				line.erase(line.length()-1);
			}
		}

		configSpecLiteralSet(variable, line);
	} else if (directive=="+IN") {
		// Like in The Highlander, there can only be one
		MYASSERT(g_primaryInput==NULL);

		auto pipe = execCmd(dirline);
		if (!pipe) {
			std::string err = "While processing +IN directive, failed to run command: " + dirline;
			MYTHROW(err);
		}
		g_primaryInput = pipe;
	} else {
		std::string err = "Unknown directive: " + directive;
		MYTHROW(err);
	}
}
