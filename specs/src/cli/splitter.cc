#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/platform.h"
#include "utils/ErrorReporting.h"
#include "processing/Config.h"
#include <dirent.h>
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
							Token(TokenListType__LITERAL, NULL /* range */,
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
	// Special case - a line that is just a pound sign
	if (st=="#") return std::string("");
	
	std::size_t found = st.rfind("# ");
	if (found==std::string::npos) return st;

	if (found>0 && !is_whitespace(st[found-1])) return st;

	if (found==0) return std::string("");

	return st.substr(0,found);
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
			onePath = strtok(NULL, PATH_LIST_SEPARATOR);
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
			spec += " ";
			spec += removeComment(line);
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
			DIR *dir;
			struct dirent *ent;
			if ((dir = opendir (onePath)) != NULL) {
				while ((ent = readdir (dir)) != NULL) {
					if (isPotentiallyASpecificationName(ent->d_name) &&
							(specName==ent->d_name || specName=="")) {
						std::cerr << "Specification <" << ent->d_name << ">";
						std::string fullpath = std::string(onePath) + PATHSEP + ent->d_name;
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
							closedir(dir);
							free(spath);
							return true;
						}
					}
				}
				closedir(dir);
			}
			onePath = strtok(NULL, PATH_LIST_SEPARATOR);
		}
	}
	if (spath) free(spath);
	return (specName=="");
}
