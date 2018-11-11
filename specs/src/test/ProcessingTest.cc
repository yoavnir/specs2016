#include <iostream>
#include <string>
#include "cli/tokens.h"
#include "specitems/specItems.h"
#include "processing/ProcessingState.h"
#include "processing/StringBuilder.h"

#define VERIFY(sp,ex) {          \
		PSpecString ps = runTestOnExample(sp);  \
		testCount++;                            \
		std::cout << "Test #" << testCount << " ";     \
		if (!ps) {                              \
			std::cout << "*** NOT OK ***: Got (NULL); Expected: <" << ex << ">\n"; \
			errorCount++;                       \
		} else {                                \
			if (ps->Compare(ex)) {              \
				std::cout << "*** NOT OK ***:\n\tGot <" << ps->data() << ">\n\tExp <" << ex << ">\n"; \
				errorCount++;                   \
			} else {                            \
				std::cout << "***** OK *****: <" << ex << ">\n"; \
			}                                   \
		}                                       \
}

PSpecString runTestOnExample(const char* _specList)
{
	ProcessingState ps;
	StdSpecString example1("The quick brown fox jumped over the   lazy dog");
	ps.setString(&example1);
	char* specList = (char*)_specList;

	std::vector<Token> vec = parseTokens(1, &specList);
	normalizeTokenList(&vec);
	itemGroup ig;

	unsigned int index = 0;
	ig.Compile(vec,index);

	StringBuilder sb;

	PSpecString result = NULL;
	if (ig.processDo(sb, ps, NULL, NULL)) {
		result = sb.GetString();
	}

	return result;
}

int main(int argc, char** argv)
{
	ProcessingState ps;
	int errorCount = 0;
	int testCount  = 0;

	VERIFY("w1 1", "The"); // Test #1
	VERIFY("7-17 1", "ick brown f"); // Test #2
	VERIFY("2;-2 1", "he quick brown fox jumped over the   lazy do"); // Test #3
	VERIFY("20-* 1", " jumped over the   lazy dog"); // Test #4
	VERIFY("w3-5 1", "brown fox jumped"); // Test #5
	VERIFY("word 3-5 1", "brown fox jumped"); // Test #6
	VERIFY("word 8-9 1", "lazy dog"); // Test #7
	VERIFY("word 8-* 1", "lazy dog"); // Test #8
	VERIFY("w1 1 w3 nw w6 n", "The brownover"); // Test #9
	VERIFY("w1 2 w3 4 w7-8 12", " Thbrown   the   lazy"); // Test #10
	VERIFY("w1 1 w3 nf", "The\tbrown"); // Test #11
	VERIFY("substring 2-4 of word 3 1", "row"); // Test #12
	VERIFY("substring word 3;-2 of 1-* 1", "brown fox jumped over the   lazy"); // Test #13
	VERIFY("substring wordsep e w3 of 1-* 1", "d ov"); // Test #14
	VERIFY("substring fieldsep e w3 of 1-* 1", "brown"); // Test #15
	VERIFY("substring fieldsep / / field 4 of w7-* 1", "lazy"); // Test #16
	VERIFY("w1 1.5 w2 n.5 w3 n.4 w4 n","The  quickbrowfox"); // Test #17
	VERIFY("w1 1.8 right w2 n.8 center w3 n left", "     The quick  brown"); // Test #18
	VERIFY("33.6 strip 1.6 right", "   the"); // Test #19

	if (errorCount) {
		std::cout << '\n' << errorCount << '/' << testCount << " tests failed.\n";
	} else {
		std::cout << "\nAll tests passed.\n";
	}

	return (errorCount==0) ? 0 : 4;
}
