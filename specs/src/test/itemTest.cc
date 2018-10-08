#include <iostream>
#include "../cli/tokens.h"
#include "../specitems/specItems.h"

int main(int argc, char** argv)
{
	std::vector<Token> vec = parseTokens(argc-1, argv+1); // skipping the program name
	normalizeTokenList(&vec);

	for (int i=0; i<vec.size(); i++) {
		int digits = (std::to_string(i)).length();
		std::cout << i << ". " << vec[i].Debug(digits) << "\n";
	}

	return 0;
}
