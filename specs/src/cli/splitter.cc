#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "utils/ErrorReporting.h"
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
	char delimiter;

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

std::vector<Token> parseTokensFile(std::string& fileName)
{
	std::ifstream specFile(fileName);
	if (specFile.is_open()) {
		std::string spec;
		std::string line;
		while (getline(specFile,line)) {
			spec += " ";
			spec += line;
		}
		return parseTokensSplit(spec.c_str());
	} else {
		std::string err = "Spec file not found: " + fileName;
		MYTHROW(err);
	}
}
