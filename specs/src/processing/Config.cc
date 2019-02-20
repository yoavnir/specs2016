
/*
 * The values here are the defaults.  They can be changed from other places in the code.
 */

#include <map>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "utils/platform.h"
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
		ExternalLiterals[key] = value;
	}
}

#ifdef WIN64
static std::string getConfigFileName() {
	return std::string(std::getenv("HOMEDRIVE")) + std::getenv("HOMEPATH") + "\\specs.cfg";
}
#else
static std::string getConfigFileName() {
	return std::string(std::getenv("HOME")) + "/.specs";
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
}

bool configSpecLiteralExists(std::string& key)
{
	return !ExternalLiterals[key].empty();
}

std::string& configSpecLiteralGet(std::string& key)
{
	return ExternalLiterals[key];
}
