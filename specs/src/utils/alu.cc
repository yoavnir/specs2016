/*
 * The Arithmetic / Logic Unit
 * ===========================
 *
 * This contains all the code needed for storing variables, parsing expressions,
 * and evaluating expressions. These are needed to implement the PRINT and SET
 * directives of specs.
 */

#include <sstream>
#include "ErrorReporting.h"
#include "alu.h"

void ALUCounter::set(std::string& s)
{
	m_value = s;
	m_type = counterType__Str;
}

void ALUCounter::set(const char* st)
{
	std::string s(st);
	set(s);
}

void ALUCounter::set(ALUInt l)
{
	m_value = std::to_string(l);
	m_type = counterType__Int;
}

void ALUCounter::set(ALUFloat f)
{
    std::ostringstream ost;
    ost.precision(ALUFloatPrecision);
    ost << f;
	m_value = ost.str();
	m_type = counterType__Float;
}

ALUInt ALUCounter::getInt() const
{
	return (counterType__None==m_type) ? 0 : std::stoll(m_value);
}

ALUInt ALUCounter::getHex() const
{
	return (counterType__None==m_type) ? 0 : std::stoll(m_value, NULL, 16);
}

ALUFloat ALUCounter::getFloat() const
{
	return (counterType__None==m_type) ? 0.0 : std::stold(m_value);
}

bool ALUCounter::getBool() const
{
	return (m_value!="0" && m_value!="0.0");
}

bool ALUCounter::isWholeNumber() const
{
	return (ALUFloat(getInt())==getFloat());
}

bool ALUCounter::isNumeric() const
{
	switch (m_type) {
	case counterType__None:
		return false;
	case counterType__Int:
	case counterType__Float:
		return true;
	default: {
		std::size_t pos;
		std::stold(m_value, &pos);
		return m_value.length() == pos;

	}
	}
}

ALUCounterType ALUCounter::getDivinedType()
{
	if (m_type != counterType__Str) {
		return m_type;
	}

	if (!isNumeric()) {
		return counterType__Str;
	}

	if (isWholeNumber()) {
		return counterType__Int;
	}

	return counterType__Float;
}

class AluCounterDeleter {
public:
	AluCounterDeleter(ALUCounter* p) 	{p_ctr = p;}
	~AluCounterDeleter()				{delete p_ctr;}
private:
	ALUCounter* p_ctr;
};


ALUCounter* AluUnit::compute()
{
	std::string err = _identify() + " should not be called with no operands";
	MYTHROW(err);
}

ALUCounter* AluUnit::compute(ALUCounter* op)
{
	std::string err = _identify() + " should not be called with one operand";
	MYTHROW(err);
}

ALUCounter* AluUnit::compute(ALUCounter* op1, ALUCounter* op2)
{
	std::string err = _identify() + " should not be called with two operands";
	MYTHROW(err);
}


void AluUnitLiteral::_serialize(std::ostream& os) const
{
	os << '(' << m_literal.getFloat() << ')';
}

std::string AluUnitLiteral::_identify()
{
	return std::string("A literal (") + m_literal.getStr() + ")";
}

ALUCounter* AluUnitLiteral::compute()
{
	return new ALUCounter(m_literal);
}


void AluUnitCounter::_serialize(std::ostream& os) const
{
	os << '#' << m_ctrNumber;
}

std::string AluUnitCounter::_identify()
{
	return std::string("A counter (") + std::to_string(m_ctrNumber) + ")";
}

ALUCounter* AluUnitCounter::compute()
{
	return new ALUCounter(*(m_ctrs->getPointer(m_ctrNumber)));
}

static fieldIdentifierGetter* g_fieldIdentifierGetter = NULL;

void setFieldIdentifierGetter(fieldIdentifierGetter* getter)
{
	g_fieldIdentifierGetter = getter;
}

void AluUnitFieldIdentifier::_serialize(std::ostream& os) const
{
	os << m_id;
}

std::string AluUnitFieldIdentifier::_identify()
{
	return std::string("fieldIdentifier(") + std::to_string(m_id) + ")";
}

ALUCounter* AluUnitFieldIdentifier::compute()
{
	if (!g_fieldIdentifierGetter) {
		MYTHROW("Field Identifier Getter is not set")
	}
	std::string content = g_fieldIdentifierGetter->Get(m_id);
	return new ALUCounter(content);
}

#define X(nm,st)	if (s==st) {m_op = UnaryOp__##nm; return;}
AluUnitUnaryOperator::AluUnitUnaryOperator(std::string& s)
{
	ALU_UOP_LIST
	std::string err = "Invalid unary operand: <"+s+">";
	MYTHROW(err);
}
#undef X

#define X(nm,st)	case UnaryOp__##nm: os << st; break;
void AluUnitUnaryOperator::_serialize(std::ostream& os) const
{
	switch(m_op) {
	ALU_UOP_LIST
	default:
		MYTHROW("Invalid unary operand");
	};
}
#undef X

#define X(nm,st)	case UnaryOp__##nm: return ret + st; break;
std::string AluUnitUnaryOperator::_identify()
{
	std::string ret = std::string("Unary Operator ");
	switch (m_op) {
	ALU_UOP_LIST
	default:
		MYTHROW("Invalid unary operand");
		return ""; // prevent warning
	}
}
#undef X

#define X(nm,st)	case UnaryOp__##nm: return compute##nm(operand);
ALUCounter*		AluUnitUnaryOperator::compute(ALUCounter* operand)
{
	AluCounterDeleter _op(operand);
	if (counterType__None==operand->getType()) {
		return new ALUCounter();
	}
	switch (m_op) {
	ALU_UOP_LIST
	default:
		MYTHROW("Invalid unary operand");
	}
}
#undef X

#define RETURN_FALSE return new ALUCounter(ALUInt(0))
#define RETURN_TRUE  return new ALUCounter(ALUInt(1))

#define RETURN_COND(cond) { if ((cond)) { RETURN_TRUE; } else { RETURN_FALSE; } }

ALUCounter* 	AluUnitUnaryOperator::computeNot(ALUCounter* operand)
{
	RETURN_COND(false==operand->getBool());
}

ALUCounter* 	AluUnitUnaryOperator::computePlus(ALUCounter* operand)
{
	switch (operand->getType()) {
	case counterType__Float:
	case counterType__Int:
		return new ALUCounter(*operand);
	case counterType__Str:
		if (operand->isWholeNumber()) {
			return new ALUCounter(operand->getInt());
		} else {
			return new ALUCounter(operand->getFloat());
		}
	default:
		MYTHROW("Invalid operand type");
	}
}

ALUCounter* 	AluUnitUnaryOperator::computeMinus(ALUCounter* operand)
{
	switch (operand->getType()) {
	case counterType__Float:
		return new ALUCounter(-operand->getFloat());
	case counterType__Int:
		return new ALUCounter(-operand->getInt());
	case counterType__Str:
		if (operand->isWholeNumber()) {
			return new ALUCounter(-operand->getInt());
		} else {
			return new ALUCounter(-operand->getFloat());
		}
	default:
		MYTHROW("Invalid operand type");
	}
}



#define X(nm,st)	if (s==st) {m_op = BinaryOp__##nm; return;}
AluBinaryOperator::AluBinaryOperator(std::string& s)
{
	ALU_BOP_LIST
	std::string err = "Invalid binary operand: <"+s+">";
	MYTHROW(err);
}
#undef X

#define X(nm,st)	case BinaryOp__##nm: os << st; break;
void AluBinaryOperator::_serialize(std::ostream& os) const
{
	switch(m_op) {
	ALU_BOP_LIST
	default:
		MYTHROW("Invalid binary operand");
	};
}
#undef X

#define X(nm,st)	case BinaryOp__##nm: return ret + st; break;
std::string AluBinaryOperator::_identify()
{
	std::string ret = std::string("Binary Operator ");
	switch (m_op) {
	ALU_BOP_LIST
	default:
		MYTHROW("Invalid binary operand");
		return ""; // prevent warning
	}
}
#undef X

#define X(nm,st)	case BinaryOp__##nm: return compute##nm(op1, op2);
ALUCounter*		AluBinaryOperator::compute(ALUCounter* op1, ALUCounter* op2)
{
	AluCounterDeleter _op1(op1);
	AluCounterDeleter _op2(op2);
	if (counterType__None==op1->getType() || counterType__None==op2->getType()) {
		return new ALUCounter();
	}

	switch (m_op) {
	ALU_BOP_LIST
	default:
		MYTHROW("Invalid unary operand");
	}
}
#undef X

// Simple floating point or integer addition
ALUCounter*		AluBinaryOperator::computeAdd(ALUCounter* op1, ALUCounter* op2)
{
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return new ALUCounter(op1->getFloat() + op2->getFloat());
	}
	if (counterType__Int==op1->getType() || counterType__Int==op2->getType()) {
		return new ALUCounter(op1->getInt() + op2->getInt());
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return new ALUCounter(op1->getInt() + op2->getInt());
	}
	return new ALUCounter(op1->getFloat() + op2->getFloat());
}

// Simple floating point or integer subtraction
ALUCounter*		AluBinaryOperator::computeSub(ALUCounter* op1, ALUCounter* op2)
{
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return new ALUCounter(op1->getFloat() - op2->getFloat());
	}
	if (counterType__Int==op1->getType() || counterType__Int==op2->getType()) {
		return new ALUCounter(op1->getInt() - op2->getInt());
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return new ALUCounter(op1->getInt() - op2->getInt());
	}
	return new ALUCounter(op1->getFloat() - op2->getFloat());
}

// Floating point or integer multiplication
ALUCounter*		AluBinaryOperator::computeMult(ALUCounter* op1, ALUCounter* op2)
{
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return new ALUCounter(op1->getFloat() * op2->getFloat());
	}
	if (counterType__Int==op1->getType() || counterType__Int==op2->getType()) {
		return new ALUCounter(op1->getInt() * op2->getInt());
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return new ALUCounter(op1->getInt() * op2->getInt());
	}
	return new ALUCounter(op1->getFloat() * op2->getFloat());
}

// Numeric division. The result quotient may be floating point even when the
// dividend and divisor are both integers.
ALUCounter*		AluBinaryOperator::computeDiv(ALUCounter* op1, ALUCounter* op2)
{
	// guard against divide-by-zero: return NaN
	if (counterType__None!=op2->getType() && 0.0==op2->getFloat()) {
		return new ALUCounter();
	}
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return new ALUCounter(op1->getFloat() / op2->getFloat());
	}
	if (counterType__Int==op1->getType() || counterType__Int==op2->getType()) {
		if (0==(op1->getInt() % op2->getInt())) {
			return new ALUCounter(op1->getInt() / op2->getInt());
		} else {
			return new ALUCounter(op1->getFloat() / op2->getFloat());
		}
	}
	if (op1->isWholeNumber() && op2->isWholeNumber() && (0==(op1->getInt() % op2->getInt()))) {
		return new ALUCounter(op1->getInt() / op2->getInt());
	}
	return new ALUCounter(op1->getFloat() / op2->getFloat());
}

// String concatenation ||
ALUCounter*		AluBinaryOperator::computeAppnd(ALUCounter* op1, ALUCounter* op2)
{
	std::string ret = op1->getStr() + op2->getStr();
	return new ALUCounter(ret);
}

// Integer division. Divides the integer form of both numbers: 19.5 % 2.001 = 9
ALUCounter*		AluBinaryOperator::computeIntDiv(ALUCounter* op1, ALUCounter* op2)
{
	// guard against divide-by-zero: return NaN
	if (counterType__None!=op2->getType() && 0.0==op2->getFloat()) {
		return new ALUCounter();
	}

	return new ALUCounter(op1->getInt() / op2->getInt());
}

// Remainder in integer division. The operator is //
ALUCounter*		AluBinaryOperator::computeRemDiv(ALUCounter* op1, ALUCounter* op2)
{
	// guard against divide-by-zero: return NaN
	if (counterType__None!=op2->getType() && 0.0==op2->getFloat()) {
		return new ALUCounter();
	}

	return new ALUCounter(op1->getInt() % op2->getInt());
}

// Logical AND. Always returns zero or one.
ALUCounter*		AluBinaryOperator::computeAND(ALUCounter* op1, ALUCounter* op2)
{
	RETURN_COND(op1->getBool() && op2->getBool());
}

// Logical OR. Always returns zero or one.
ALUCounter*		AluBinaryOperator::computeOR(ALUCounter* op1, ALUCounter* op2)
{
	RETURN_COND(op1->getBool() || op2->getBool());
}

// Simple floating point or string comparison
ALUCounter*		AluBinaryOperator::computeEQ(ALUCounter* op1, ALUCounter* op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() == op2->getFloat());
	}

	RETURN_COND(op1->getStr() == op2->getStr());
}

// Simple floating point or string comparison
ALUCounter*		AluBinaryOperator::computeNE(ALUCounter* op1, ALUCounter* op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() != op2->getFloat());
	}

	RETURN_COND(op1->getStr() != op2->getStr());
}

// Simple floating point or string comparison
ALUCounter*		AluBinaryOperator::computeGT(ALUCounter* op1, ALUCounter* op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() > op2->getFloat());
	}

	RETURN_COND(op1->getStr().compare(op2->getStr()) > 0);
}

// Simple floating point or string comparison
ALUCounter*		AluBinaryOperator::computeGE(ALUCounter* op1, ALUCounter* op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() >= op2->getFloat());
	}

	RETURN_COND(op1->getStr().compare(op2->getStr()) >= 0);
}

// Simple floating point or string comparison
ALUCounter*		AluBinaryOperator::computeLT(ALUCounter* op1, ALUCounter* op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() < op2->getFloat());
	}

	RETURN_COND(op1->getStr().compare(op2->getStr()) < 0);
}

// Simple floating point or string comparison
ALUCounter*		AluBinaryOperator::computeLE(ALUCounter* op1, ALUCounter* op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() <= op2->getFloat());
	}

	RETURN_COND(op1->getStr().compare(op2->getStr()) <= 0);
}

// Strict floating point or string comparison
ALUCounter*		AluBinaryOperator::computeSEQ(ALUCounter* op1, ALUCounter* op2)
{
	RETURN_COND(op1->getStr() == op2->getStr());
}

// Strict floating point or string comparison
ALUCounter*		AluBinaryOperator::computeSNE(ALUCounter* op1, ALUCounter* op2)
{
	RETURN_COND(op1->getStr() != op2->getStr());
}

// Strict floating point or string comparison
ALUCounter*		AluBinaryOperator::computeSGT(ALUCounter* op1, ALUCounter* op2)
{
	RETURN_COND(op1->getStr().compare(op2->getStr()) > 0);
}

// Strict floating point or string comparison
ALUCounter*		AluBinaryOperator::computeSLT(ALUCounter* op1, ALUCounter* op2)
{
	RETURN_COND(op1->getStr().compare(op2->getStr()) < 0);
}

// Strict floating point or string comparison
ALUCounter*		AluBinaryOperator::computeSGTE(ALUCounter* op1, ALUCounter* op2)
{
	RETURN_COND(op1->getStr().compare(op2->getStr()) >= 0);
}

// Strict floating point or string comparison
ALUCounter*		AluBinaryOperator::computeSLTE(ALUCounter* op1, ALUCounter* op2)
{
	RETURN_COND(op1->getStr().compare(op2->getStr()) <= 0);
}

