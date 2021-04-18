#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/platform.h"
#include "utils/directives.h"
#include "utils/ErrorReporting.h"
#include "processing/Config.h"
#include "tokens.h"

void parseSingleToken(std::vector<Token> *pVec, std::string arg, int argidx);

enum splitterState {
	SpSt__Start,
	SpSt__WhiteSpace,
	SpSt__Escape,
	SpSt__GettingChars
};

static bool is_whitespace(char c) {
	return (c==' ' || c=='\t');
}

static bool isPossibleDelimiter(char c) {
	return (c=='/' || c=='"' || c=='\'');
}

std::vector<Token> parseTokensSplit(const char* arg)
{
	std::vector<Token> ret;
	splitterState st = SpSt__WhiteSpace;

	unsigned int argindex = 1;

	const char* ptr = arg;
	std::string sarg;
	char delimiter = '\0';

	while (*ptr) {
		switch (st) {
		case SpSt__WhiteSpace:
			if (!sarg.empty()) {
				parseSingleToken(&ret, sarg, argindex++);
				sarg.clear();
			}
			while (is_whitespace(*ptr)) ptr++;
			if (!*ptr) continue;
			st = SpSt__Start;
			sarg.clear();
			delimiter = '\0';
			/* intentional fall-through */
		case SpSt__Start:
			if (*ptr=='\\') {
				st = SpSt__Escape;
			} else if (*ptr=='(') {
				st = SpSt__GettingChars;
				delimiter = ')';
				sarg+=*ptr;
			} else if (isPossibleDelimiter(*ptr)) {
				st = SpSt__GettingChars;
				delimiter = *ptr;
				sarg+=*ptr;
			} else {
				st = SpSt__GettingChars;
				sarg+=*ptr;
			}
			ptr++;
			break;
		case SpSt__Escape:
			if (!*ptr) {
				MYTHROW("Bad escape character at end of argument");
			}
			sarg+=*ptr++;
			st = SpSt__GettingChars;
			break;
		case SpSt__GettingChars:
			if (!*ptr) {
				parseSingleToken(&ret, sarg, argindex++);
				continue;
			} else if (*ptr==delimiter) {
				if (delimiter==')') {
					sarg+=')';
					parseSingleToken(&ret, sarg, argindex++);
				} else {
					std::string delimitedToken = sarg.substr(1);
					sarg += delimiter;
					ret.insert(ret.end(),
							Token(TokenListType__LITERAL, nullptr /* range */,
									delimitedToken, argindex++, sarg));
				}
				sarg.clear();
				st = SpSt__WhiteSpace;
			} else if (*ptr=='\\') {
				st = SpSt__Escape;
			} else if (!delimiter && is_whitespace(*ptr)) {
				st = SpSt__WhiteSpace;
				continue; // avoid incrementing ptr just yet
			} else {
				sarg += *ptr;
			}
			ptr++;
			break;
		}
	}

	if (SpSt__GettingChars==st && delimiter=='\0') {
		parseSingleToken(&ret, sarg, argindex++);
		sarg.clear();
	}

	if (SpSt__GettingChars==st && delimiter!='\0') {
		MYTHROW("Missing delimiter at end of input");
	}

	if (!sarg.empty()) {
		MYTHROW("whaaa?");
	}


	return ret;
}

// A comment is defined as starting with the last hash mark + space  ("# ") sequence 
// on the line, preceded by whitespace unless it's at the start of the line.
std::string removeComment(std::string& st)
{
	std::string ret;
	std::size_t found = 0;

	// Special case - a line that is just a pound sign
	if (st=="#") {
		ret = std::string("");
		goto FINISH;
	}
	
	found = st.rfind("# ");

	if (found==std::string::npos || (found>0 && !is_whitespace(st[found-1]))) {
		ret = st;
		goto FINISH;
	} else if (found==0) {
		ret = std::string("");
		goto FINISH;
	}

	ret = st.substr(0,found);

FINISH:
	// return empty string if the string is all whitespace
	if (std::string::npos == ret.find_first_not_of(" \t\n\r")) {
		ret = std::string("");
	}
	return ret;
}

static void openSpecFile(std::ifstream& theFile, std::string& fileName)
{
	theFile.open(fileName);
	if (theFile.is_open()) return;

	// No?  Try the path
	char* spath = strdup(getFullSpecPath());
	if (spath && spath[0]) {
		char* onePath = strtok(spath, PATH_LIST_SEPARATOR);
		while (onePath) {
			std::string fullpath = std::string(onePath) + PATHSEP + fileName;
			theFile.open(fullpath);
			if (theFile.is_open()) {
				free(spath);
				return;
			}
			onePath = strtok(nullptr, PATH_LIST_SEPARATOR);
		}
	}
	if (spath) free(spath);
}

std::vector<Token> parseTokensFile(std::string& fileName)
{
	std::ifstream specFile;
	openSpecFile(specFile, fileName);
	if (specFile.is_open()) {
		std::string spec;
		std::string line;
		while (getline(specFile,line)) {
			if (spec=="" && line[0]=='+') {
				processPlusDirective(line);
				continue;
			}
			std::string deCommentedLine = removeComment(line);
			if (deCommentedLine.length() > 0) {
				spec += " ";
				spec += removeComment(line);
			}
		}
		return parseTokensSplit(spec.c_str());
	} else {
		std::string err = "Spec file not found: " + fileName;
		MYTHROW(err);
	}
}

static bool isPotentiallyASpecificationName(const char* _s)
{
	if (_s[0]=='.' || _s[0]=='_') return false;
	if (strstr(_s, ".py")) return false;
	return true;
}

bool dumpSpecificationsList(std::string specName)
{
	char* spath = strdup(getFullSpecPath());
	if (spath && spath[0]) {
		char* onePath = strtok(spath, PATH_LIST_SEPARATOR);
		while (onePath) {
			auto fileNameList = getDirectoryFileNames(onePath);
			unsigned int idx;
			for (idx=0; fileNameList[idx]; idx++) {
				if (isPotentiallyASpecificationName(fileNameList[idx]) &&
						(specName==fileNameList[idx] || specName=="")) {
					std::cerr << "Specification <" << fileNameList[idx] << ">";
					std::string fullpath = std::string(onePath) + PATHSEP + fileNameList[idx];
					std::ifstream specFile;
					specFile.open(fullpath);
					std::string line;
					if (specName=="") {
						if (getline(specFile,line)) {
							if (line[0]=='#') {
								std::cerr << ": " << line.substr(1);
							}
						}
						std::cerr << std::endl;
					} else {
						bool bStartCommentEnded = false;
						std::cerr << std::endl;
						while (!bStartCommentEnded && getline(specFile,line)) {
							if (line[0]=='#') {
								std::cerr << "\t" << line.substr(1) << std::endl;
							} else {
								bStartCommentEnded = true;
							}
						}
						std::cerr << std::endl;
						free(spath);
						return true;
					}
				}
			}
			onePath = strtok(nullptr, PATH_LIST_SEPARATOR);
		}
	}
	if (spath) free(spath);
	return (specName=="");
}
