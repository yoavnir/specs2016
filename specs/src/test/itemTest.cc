#include <iostream>
#include "../cli/tokens.h"
#include "../specitems/specItems.h"
#include "../processing/StringBuilder.h"
#include "../processing/Reader.h"
#include "../processing/Writer.h"

std::string Jabberwocky[] = {
	    std::string("’Twas brillig, and the slithy toves "),
	    std::string("      Did gyre and gimble in the wabe: "),
	    std::string("All mimsy were the borogoves, "),
	    std::string("      And the mome raths outgrabe. "),
	    std::string(""),
	    std::string("“Beware the Jabberwock, my son! "),
	    std::string("      The jaws that bite, the claws that catch! "),
	    std::string("Beware the Jubjub bird, and shun "),
	    std::string("      The frumious Bandersnatch!” "),
	    std::string(""),
	    std::string("He took his vorpal sword in hand; "),
	    std::string("      Long time the manxome foe he sought— "),
	    std::string("So rested he by the Tumtum tree "),
	    std::string("      And stood awhile in thought. "),
	    std::string(""),
	    std::string("And, as in uffish thought he stood, "),
	    std::string("      The Jabberwock, with eyes of flame, "),
	    std::string("Came whiffling through the tulgey wood, "),
	    std::string("      And burbled as it came! "),
	    std::string(""),
	    std::string("One, two! One, two! And through and through "),
	    std::string("      The vorpal blade went snicker-snack! "),
	    std::string("He left it dead, and with its head "),
	    std::string("      He went galumphing back. "),
	    std::string(""),
	    std::string("“And hast thou slain the Jabberwock? "),
	    std::string("      Come to my arms, my beamish boy! "),
	    std::string("O frabjous day! Callooh! Callay!” "),
	    std::string("      He chortled in his joy. "),
	    std::string(""),
	    std::string("’Twas brillig, and the slithy toves "),
	    std::string("      Did gyre and gimble in the wabe: "),
	    std::string("All mimsy were the borogoves, "),
	    std::string("      And the mome raths outgrabe."),
	    std::string("")
};

int main(int argc, char** argv)
{
	std::vector<Token> vec = parseTokens(argc-1, argv+1); // skipping the program name
	normalizeTokenList(&vec);
	itemGroup ig;
	StringBuilder sb;
	ProcessingState ps;
	TestReader *pRd;
	SimpleWriter *pWr;

	unsigned int index = 0;
	ig.parse(vec, index);

	std::cout << "After parsing, index = " << index << "/" << vec.size() << "\n";

	std::cout << ig.Debug();

	pRd = new TestReader(Jabberwocky, sizeof(Jabberwocky)/sizeof(std::string));
	pWr = new SimpleWriter;

	pRd->begin();
	pWr->Begin();

	ig.process(sb, ps, *pRd, *pWr);

	delete pRd;
	pWr->End();
	delete pWr;
	vec.clear();

	return 0;
}
