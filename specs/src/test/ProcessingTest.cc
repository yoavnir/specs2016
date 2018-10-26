#include <iostream>
#include <string>
#include "processing/ProcessingState.h"
#include "processing/StringBuilder.h"

#define VERIFYSTRING(s) {     \
	testCount++;     \
	if (theOnlyTest==0 || theOnlyTest==testCount) { \
		std::cout << testCount << ". Expecting: <" << s << "> Got: <" << *pRet << "> ... ";  \
		if (pRet->Compare(s)==0) {  \
			std::cout << "OK\n";     \
		} else {                     \
			std::cout << "ERROR\n";  \
			errorCount++;            \
		} } }

#define GET_WORD_RANGE(f,t) ps.getFromTo((ps.getWordStart(f)),(ps.getWordEnd(t)))

int main(int argc, char** argv)
{
	int theOnlyTest = 0;
	ProcessingState ps;
	int errorCount = 0;
	int testCount  = 0;

	if (argc>1) { theOnlyTest = atoi(argv[1]); }

	StdSpecString example1("The quick brown fox jumped over the   lazy dog");
	ps.setString(&example1);

	// Let's check the subset from 7 to 17. Should be "ick brown f"
	PSpecString pRet = ps.getFromTo(7, 17);
	VERIFYSTRING("ick brown f");

	// Now let's try from the second to the penultimate char --> "he quick brown fox jumped over the lazy do"
	pRet = ps.getFromTo(2,-2);
	VERIFYSTRING("he quick brown fox jumped over the   lazy do");

	// third to 5th words --> "brown fox jumped"
	int _from = ps.getWordStart(3);
	int _to = ps.getWordEnd(5);
	pRet = ps.getFromTo(_from, _to);
	VERIFYSTRING("brown fox jumped");

	// Some StringBuilder tests
	StringBuilder sb;

	// w1 1 w3 nw w6 n ==> "The brownover"
	pRet = GET_WORD_RANGE(1,1);
	sb.insert(pRet, 1);
	pRet = GET_WORD_RANGE(3,3);
	sb.insertNextWord(pRet);
	pRet = GET_WORD_RANGE(6,6);
	sb.insertNext(pRet);
	pRet = sb.GetString();
	VERIFYSTRING("The brownover");

	// w1 2 w3 4 w7-8 12 ==> " Thbrown    the   lazy"
	pRet = GET_WORD_RANGE(1,1);
	sb.insert(pRet, 2);
	pRet = GET_WORD_RANGE(3,3);
	sb.insert(pRet, 4);
	pRet = GET_WORD_RANGE(7,8);
	sb.insert(pRet, 12);
	pRet = sb.GetString();
	VERIFYSTRING(" Thbrown   the   lazy");

	// w1 1 w3 nf ==? "The\tbrown"
	pRet = GET_WORD_RANGE(1,1);
	sb.insert(pRet,1);
	pRet = GET_WORD_RANGE(3,3);
	sb.insertNextField(pRet);
	pRet = sb.GetString();
	VERIFYSTRING("The\tbrown");

	return (errorCount==0) ? 0 : 4;
}
