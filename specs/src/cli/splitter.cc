#include <vector>
#include <string>
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

static bool is_delimiter(char c) {
	return (c=='/' || c=='@' || c=='\'' || c=='"' || c=='#'
			|| c=='!' || c=='$' || c=='%' || c=='^' || c=='&' || c=='~'
			|| c=='*' || c=='+' || c=='-' || c=='_' || c=='=' || c=='.' || c==',');
}

std::vector<Token> parseTokensSplit(char* arg)
{
	std::vector<Token> ret;
	splitterState st = SpSt__WhiteSpace;

	unsigned int argindex = 0;

	char* ptr = arg;
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
			} else if (is_delimiter(*ptr)) {
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
				throw std::invalid_argument("Bad escape character at end of argument.");
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
					ret.insert(ret.end(),
							Token(TokenListType__LITERAL,
									NULL, sarg.substr(1)+delimiter,
									argindex++, sarg));
				}
				st = SpSt__WhiteSpace;
			} else if (*ptr=='\\') {
				st = SpSt__Escape;
			} else if (is_whitespace(*ptr)) {
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
		throw std::invalid_argument("Missing delimiter at end of input");
	}

	if (!sarg.empty()) {
		throw std::logic_error("whaaa?");
	}


	return ret;
}
/* parseSingleToken(&ret, arg, argindex); */
