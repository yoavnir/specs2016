
/*
 * The values here are the defaults.  They can be changed from other places in the code.
 */

#include <map>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#ifdef WIN64
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include "utils/platform.h"
#include "utils/TimeUtils.h"
#include "utils/PythonIntf.h"
#include "utils/aluRegex.h"
#include "Config.h"

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define X(nm,typ,defval,ssw,cliswitch,oval) typ g_##nm = defval;
CONFIG_PARAMS
#undef X


static std::map<std::string,std::string> ExternalLiterals;

static void useKeyValue(std::string& key, std::string& value)
{
	if (':'==key.at(key.length() - 1)) {
		key = key.substr(0,key.length() - 1);
		if (key == "timezone") {
			specTimeSetTimeZone(value);
		} else if (key == "locale") {
			specTimeSetLocale(value);
		} else if (key == "regexType") {
			setRegexType(value);
		}
		if (ExternalLiterals.find(key) == ExternalLiterals.end()) {
			ExternalLiterals[key] = value;
		}
	}
}

#ifdef WIN64
static std::string getConfigFileName() {
	if (g_configurationFile!="") {
		return g_configurationFile;
	}
	return std::string(std::getenv("HOMEDRIVE")) + std::getenv("HOMEPATH") + "\\specs.cfg";
}

std::string getPersistneceFileName() {
	return std::string(std::getenv("HOMEDRIVE")) + std::getenv("HOMEPATH") + "\\specs_persistent.sav";
}
#else
static std::string getConfigFileName() {
	if (g_configurationFile!="") {
		return g_configurationFile;
	}
	return std::string(std::getenv("HOME")) + "/.specs";
}

std::string getPersistneceFileName() {
	return std::string(std::getenv("HOME")) + "/.specs_persistent";
}
#endif

static bool is_whitespace(char c)
{
	return (c==' ' || c=='\t' || c=='\n');
}

static bool is_delimiter(char c)
{
	return (c=='/' || c=='"' || c=='\'');
}

static void alertInvalidLine(std::string& ln, unsigned int lineNum, const char* err)
{
	std::cerr << "Invalid configuration file line at line #" << lineNum <<
			" -- " << err << ":\n\t" << ln << "\n";
}

static std::string getTerminalRowsAndColumns(bool bGetRows)
{
	static bool alreadyRan = false;
	static size_t rows = 0;
	static size_t cols = 0;

	if (!alreadyRan) {
#ifdef WIN64
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

		rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

		/* sanity */

		if (w.ws_row < 10) {
			w.ws_row = 10;
		} else if (w.ws_row > 120) {
			w.ws_row = 120;
		}

		if (w.ws_col < 40) {
			w.ws_col = 40;
		} else if (w.ws_col > 480) {
			w.ws_col = 480;
		}

		rows = w.ws_row;
		cols = w.ws_col;
#endif
		alreadyRan = true;
	}

	if (bGetRows) {
		return std::to_string(rows);
	} else {
		return std::to_string(cols);
	}

}

void readConfigurationFile()
{
	std::string line;
	unsigned int lineCounter = 0;
	std::string configFileName = getConfigFileName();
	std::ifstream configFile(configFileName);
	if (configFile.is_open())
	{
		while (getline(configFile,line)) {
			lineCounter++;
			unsigned int idx = 0;
			// skip over initial whitespace
			while (idx < line.length() && is_whitespace(line[idx])) idx++;
			if (idx==line.length()) continue; // separator line - continue
			if (line[idx]=='#') continue;     // comment

			unsigned int idx2 = idx+1;
			while (idx2 < line.length() && !is_whitespace(line[idx2])) idx2++;
			if (idx2==line.length()) {
				alertInvalidLine(line, lineCounter, "Key extends to end of line.");
				continue;
			}
			std::string key = line.substr(idx,idx2-idx);

			// skip over some more whitespace
			while (idx2 < line.length() && is_whitespace(line[idx2])) idx2++;
			if (idx2==line.length()) {
				alertInvalidLine(line, lineCounter, "No value found");
				continue;
			}

			// result is either one word or delimited by one of a few delimiters
			std::string value;
			idx = idx2 + 1;
			if (is_delimiter(line[idx2])) {
				while (idx<line.length() && line[idx]!=line[idx2]) idx++;
				if (line[idx]!=line[idx2]) {
					alertInvalidLine(line, lineCounter, "Delimited value not terminated.");
					continue;
				}
				value = line.substr(idx2+1, idx-idx2-1);
			} else {
				while (idx<line.length() && !is_whitespace(line[idx])) idx++;
				value = line.substr(idx2, idx-idx2);
			}

			useKeyValue(key, value);
		}
	} else {
	}

	// Some built-in stuff
#ifdef GITTAG
	ExternalLiterals["version"] = STRINGIFY(GITTAG);
#endif
#ifdef LITERAL_PLATFORM
	ExternalLiterals["platform"] = STRINGIFY(LITERAL_PLATFORM);
#endif
	ExternalLiterals["python"] = pythonInterfaceEnabled() ? "Enabled" : "Disabled";
	if (0==ExternalLiterals.count("cols")) {
		ExternalLiterals["cols"] = getTerminalRowsAndColumns(false);
	}
	if (0==ExternalLiterals.count("rows")) {
		ExternalLiterals["rows"] = getTerminalRowsAndColumns(true);
	}
}

bool configSpecLiteralExists(std::string& key)
{
	return !ExternalLiterals[key].empty();
}

std::string& configSpecLiteralGet(std::string& key)
{
	return ExternalLiterals[key];
}

std::string& configSpecLiteralGetWithDefault(std::string& key, std::string& _default)
{
	return ExternalLiterals[key].empty() ? _default : ExternalLiterals[key];
}

void configSpecLiteralSet(std::string& key, std::string& value)
{
	ExternalLiterals[key] = value;
}

bool anyNonPrimaryInputStreamDefined()
{
	static bool ret = false;
	static bool firstRun = true;

	if (firstRun) {
		ret = (g_inputStream2 != "")
				|| (g_inputStream3 != "")
				|| (g_inputStream4 != "")
				|| (g_inputStream5 != "")
				|| (g_inputStream6 != "")
				|| (g_inputStream7 != "")
				|| (g_inputStream8 != "");
	}

	return ret;
}

bool inputStreamIsDefined(int i)
{
	switch (i) {
	case 1: return true;
	case 2: return (g_inputStream2!="");
	case 3: return (g_inputStream3!="");
	case 4: return (g_inputStream4!="");
	case 5: return (g_inputStream5!="");
	case 6: return (g_inputStream6!="");
	case 7: return (g_inputStream7!="");
	case 8: return (g_inputStream8!="");
	default:
		return false;
	}
}

const char* getFullSpecPath()
{
	static std::string res("");
	static bool ran_once = false;

	if (!ran_once) {
		static std::string pathConfigString("SPECSPATH");

		// add the path from the environment variable
		char* envpath = getenv(pathConfigString.c_str());
		if (envpath && envpath[0]) {
			char* onePath = strtok(envpath, PATH_LIST_SEPARATOR);
			while (onePath) {
				if (res.length()>0) res += PATH_LIST_SEPARATOR;
				res += onePath;
				onePath = strtok(nullptr, PATH_LIST_SEPARATOR);
			}
		}

		// Also add from the configuration string
		if (configSpecLiteralExists(pathConfigString)) {
			char* configPath = strdup(configSpecLiteralGet(pathConfigString).c_str());
			char* onePath = strtok(configPath, PATH_LIST_SEPARATOR);
			while (onePath) {
				if (res.length()>0) res += PATH_LIST_SEPARATOR;
				res += onePath;
				onePath = strtok(nullptr, PATH_LIST_SEPARATOR);
			}
			free(configPath);
		}

		// Use the default if all else fails
		if (res.empty()) {
			char* parentPath = getenv(DEFAULT_SPECS_PARENT_DIR);
			if (parentPath && parentPath[0]) {
				res += parentPath;
			} else {
				res += FALLBACK_SPECS_PARENT_DIR;
			}
			res += PATHSEP;
			res += "specs";
		}

		ran_once = true;
	}

	return res.c_str();
}
