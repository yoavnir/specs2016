#include <iostream>
#include <string>
#include "../processing/ProcessingState.h"

#define VERIFYSTRING(s) {     \
	std::cout << "Expecting: <" << s << "> Got: <" << *pRet << "> ... ";  \
	if (pRet->compare(s)==0) {  \
		std::cout << "OK\n";     \
	} else {                     \
		std::cout << "ERROR\n";  \
		errorCount++;            \
	} }

int main(int argc, char** argv)
{
	ProcessingState ps;
	int errorCount = 0;

	std::string example1("The quick brown fox jumped over the   lazy dog");
	ps.setString(&example1);

	// Let's check the subset from 7 to 17. Should be "ick brown f"
	std::string *pRet = ps.getFromTo(7, 17);
	VERIFYSTRING("ick brown f");

	// Now let's try from the second to the penultimate char --> "he quick brown fox jumped over the lazy do"
	pRet = ps.getFromTo(2,-2);
	VERIFYSTRING("he quick brown fox jumped over the   lazy do");

	// third to 5th words --> "brown fox jumped"
	int _from = ps.getWordStart(3);
	int _to = ps.getWordEnd(5);
	pRet = ps.getFromTo(_from, _to);
	VERIFYSTRING("brown fox jumped");


	return (errorCount==0) ? 0 : 4;
}
