#include <iostream>
#include "../cli/tokens.h"
#include "../specitems/specItems.h"

int main(int argc, char** argv)
{
	std::vector<Token> vec = parseTokens(argc-1, argv+1); // skipping the program name
	normalizeTokenList(&vec);
	itemGroup ig;

	unsigned int index = 0;
	ig.parse(vec, index);

	std::cout << "After parsing, index = " << index << "/" << vec.size() << "\n";

	std::cout << ig.Debug();

	return 0;
}
