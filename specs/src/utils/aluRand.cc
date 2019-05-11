#include "aluRand.h"
#include "platform.h"
#include "ErrorReporting.h"
#include <fstream>

#define MAX_ALURAND_INT 0x3fffffffffffffff

AluRandContext    AluRandCtxBuffer_G;

AluRandSeedType   AluRandSeed_G;

bool              AluRand_RandomSeeded_G = false;

void AluRand_Seed()
{
	if (sizeof(AluRandSeedType) > 8) {
		AluRandSeedType   AluRandSeed;
		std::ifstream     devRandom("/dev/random");
		devRandom.read((char*)&AluRandSeed, sizeof(AluRandSeedType));
	}
	
	AluRandSeed(AluRandSeed_G);
}

#ifdef ALU_RAND_FUNC_WITH_LEN
static ALUInt AluRandGetInt()
{
	ALUInt res;
	AluRandFunc(&res, sizeof(ALUInt));
	return res & MAX_ALURAND_INT;
}
#else
static ALUInt AluRandGetInt()
{
	ALUInt res1, res2;
	AluRandFunc(res1);
	AluRandFunc(res2);
	return (res1 << 31) | res2;
}
#endif


ALUInt AluRandGetIntUpTo(ALUInt limit)
{
	if (!AluRand_RandomSeeded_G) {
		AluRand_Seed();
		AluRand_RandomSeeded_G = true;
	}

	ALUInt rndValue = AluRandGetInt();
	if (!limit) return rndValue;

	while (MAX_ALURAND_INT % limit > rndValue) {
		rndValue = AluRandGetInt();
	}

	return rndValue % limit;
}
