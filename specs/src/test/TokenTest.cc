#include <iostream>
#include "assert.h"
#include "string.h"
#include "cli/tokens.h"

std::string callParseTokens(int argc, char** argv)
{
	std::vector<Token> vec = parseTokens(argc, argv); // skipping the program name
	normalizeTokenList(&vec);
	
	std::string ret;

	for (int i=0; i<vec.size(); i++) {
		ret += vec[i].Debug(0);
		if (vec.size()>1 && i<vec.size()-1) {
			ret += '|';
		}
	}

	return ret;
}

std::string callParseTokens(const char* _str)
{
	int argc = 0;
	char* argv[200];
	static char* str = NULL;
	if (str) free(str);
	str = strdup(_str);
	char* ptr = strtok(str, " ");
	while (ptr) {
		argv[argc++] = ptr;
		ptr = strtok(NULL, " ");
		assert(argc<200);
	}
	
	return callParseTokens(argc, argv);
}

bool checkParsing(const char* _str, const char* _expected, bool bSingle)
{
	static unsigned int ctr = 0;
	std::string parsed;
	if (bSingle) {
		char* str = strdup(_str);
		parsed = callParseTokens(1, &str);
		free(str);
	} else {
		parsed = callParseTokens(_str);
	}

	std::cout << "Test " << ++ctr;
	if (bSingle) std::cout << "(s)";
	if (parsed==_expected) {
		std::cout << ": *** OK ***  in=<" << _str << ">  out=" << parsed << "\n";
		return true;
	} else {
		std::cout << ": * NOT-OK *\n\tin=<" << _str << ">\n\tout=" << parsed << "\n\texp=" << _expected << "\n";
		return false;
	}
}

#define TESTNS(t,e) if (false==checkParsing(t,e,false)) failedTests++;
int main(int argc, char** argv)
{
	unsigned int failedTests = 0;

	TESTNS("w1 1","WORDRANGE; S:1|RANGE; S:1");
	TESTNS("1 2 1-3 4;-2 -2;-1 *-7 *;-2 1-*", "RANGE; S:1|RANGE; S:2|RANGE; S:1-3|RANGE; S:4--2|RANGE; S:-2--1|LITERAL; /*-7/|LITERAL; /*;-2/|RANGE; S:1-end");
	TESTNS("w1 word 2 word 2-3 word 2;-4 word3 wor 5", "WORDRANGE; S:1|WORDRANGE; S:2|WORDRANGE; S:2-3|WORDRANGE; S:2--4|LITERAL; /word3/|WORDRANGE; S:5");
	TESTNS("nextword  nw nextw nextwo", "NEXTWORD|NEXTWORD|NEXTWORD|NEXTWORD");
	TESTNS("nextfield nf nextf nextfie", "NEXTFIELD|NEXTFIELD|NEXTFIELD|NEXTFIELD");
	TESTNS("fieldseparator , fs , fieldsepa , fieldsep , fieldse","FIELDSEPARATOR; /,/|FIELDSEPARATOR; /,/|FIELDSEPARATOR; /,/|FIELDSEPARATOR; /,/|LITERAL; /fieldse/");
	TESTNS("substring substri substr subst of","SUBSTRING|SUBSTRING|SUBSTRING|LITERAL; /subst/|OF");
	TESTNS("x x2 x2c x2cf x2c4e x2c4e X2cfe x2p4e","LITERAL; /x/|LITERAL; /x2/|LITERAL; /,/|LITERAL; /x2cf/|LITERAL; /,N/|LITERAL; /,N/|LITERAL; /X2cfe/|LITERAL; /x2p4e/");
	TESTNS("1.8 n.8 nw.8 nf.8 nextword.8","RANGE; S:1-8|NEXT; S:1-8|NEXTWORD; S:1-8|NEXTFIELD; S:1-8|NEXTWORD; S:1-8");
	TESTNS("left center centre right lefta lef cent centrer", "LEFT|CENTER|CENTER|RIGHT|LITERAL; /lefta/|LITERAL; /lef/|LITERAL; /cent/|LITERAL; /centrer/");

	if (failedTests) {
		std::cout << "\n" << failedTests << " failed tests.\n";
	} else {
		std::cout << "\nAll tests passed.\n";
	}

	return (failedTests==0) ? 0 : 4;
}


