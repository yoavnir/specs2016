#include "utils/aluFunctions.h"
#include <cmath>

/*
 *
 *
 * ALU FUNCTIONS
 * =============
 *
 *
 */

ALUValue* AluFunc_abs(ALUValue* op)
{
	if (op->getType()==counterType__Int) {
		ALUInt i = op->getInt();
		if (i<0) i = -i;
		return new ALUValue(i);
	} else {
		ALUFloat f = op->getFloat();
		if (f<0) f = -f;
		return new ALUValue(f);
	}
}

ALUValue* AluFunc_pow(ALUValue* op1, ALUValue* op2)
{
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return new ALUValue(std::pow(op1->getFloat(), op2->getFloat()));
	}
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
		return new ALUValue(ALUInt(std::pow(op1->getInt(), op2->getInt())));
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return new ALUValue(ALUInt(std::pow(op1->getInt(), op2->getInt())));
	}
	return new ALUValue(std::pow(op1->getFloat(), op2->getFloat()));
}

ALUValue* AluFunc_sqrt(ALUValue* op)
{
	return new ALUValue(std::sqrt(op->getFloat()));
}

