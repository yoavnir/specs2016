/*
 * The Arithmetic / Logic Unit
 * ===========================
 *
 * This contains all the code needed for storing variables, parsing expressions,
 * and evaluating expressions. These are needed to implement the PRINT and SET
 * directives of specs.
 */

#include <sstream>
#include <cmath>
#include <stack>
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

void ALUCounter::set()
{
	m_type = counterType__None;
}

ALUInt ALUCounter::getInt() const
{
	try {
		switch (m_type) {
		case counterType__None:
			return 0;
		case counterType__Int:
			break;
		case counterType__Float:
			return ALUInt(std::stold(m_value));
		default:
			if (counterType__Float == getDivinedType()) {
				return ALUInt(std::stold(m_value));
			}
		}
		return std::stoll(m_value);
	} catch (std::invalid_argument& e) {
		return 0;
	}
}

ALUInt ALUCounter::getHex() const
{
	try {
		return (counterType__None==m_type) ? 0 : std::stoll(m_value, NULL, 16);
	} catch (std::invalid_argument& e) {
		return 0;
	}
}

ALUFloat ALUCounter::getFloat() const
{
	try {
		return (counterType__None==m_type) ? 0.0 : std::stold(m_value);
	} catch (std::invalid_argument& e) {
		return 0;
	}
}

bool ALUCounter::getBool() const
{
	return (m_value!="0" && m_value!="0.0");
}

bool ALUCounter::isWholeNumber() const
{
	if (!isNumeric()) return false;
	ALUFloat f = getFloat();
	return (f==std::floor(f));
}

bool ALUCounter::isFloat() const
{
	return isNumeric() && (!isWholeNumber() || m_value.find('.')!=std::string::npos);
}

bool ALUCounter::isNumeric() const
{
	switch (m_type) {
	case counterType__None:
		return false;
	case counterType__Int:
	case counterType__Float:
		return true;
	default: try {
		std::size_t pos;
		std::stold(m_value, &pos);
		return m_value.length() == pos;
	} catch (std::invalid_argument& e) {
		return false;
	}
	}
}

ALUCounterType ALUCounter::getDivinedType() const
{
	if (m_type != counterType__Str) {
		return m_type;
	}

	if (!isNumeric()) {
		return counterType__Str;
	}

	if (isWholeNumber() && (std::string::npos==m_value.find('.'))) {
		return counterType__Int;
	}

	return counterType__Float;
}

class ALUCounterDeleter {
public:
	ALUCounterDeleter(ALUCounter* p) 	{p_ctr = p;}
	~ALUCounterDeleter()				{if (p_ctr) delete p_ctr;}
private:
	ALUCounter* p_ctr;
};


ALUCounter* AluUnit::evaluate()
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

ALUCounter* AluUnit::compute(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3)
{
	std::string err = _identify() + " should not be called with three operands";
	MYTHROW(err);
}

ALUCounter* AluUnit::compute(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3, ALUCounter* op4)
{
	std::string err = _identify() + " should not be called with four operands";
	MYTHROW(err);
}


void AluUnitLiteral::_serialize(std::ostream& os) const
{
	os << '(' << m_literal.getFloat() << ')';
}

std::string AluUnitLiteral::_identify()
{
	if (m_hintNumerical) {
		return std::string("Number(") + m_literal.getStr() + ")";
	} else {
		return std::string("Literal(") + m_literal.getStr() + ")";
	}
}

ALUCounter* AluUnitLiteral::evaluate()
{
	ALUCounter* ret = new ALUCounter(m_literal);
	if (m_hintNumerical) ret->divineType();
	return ret;
}


void AluUnitCounter::_serialize(std::ostream& os) const
{
	os << '#' << m_ctrNumber;
}

std::string AluUnitCounter::_identify()
{
	return std::string("Counter(") + std::to_string(m_ctrNumber) + ")";
}

ALUCounter* AluUnitCounter::compute(ALUCounters* pCtrs)
{
	return new ALUCounter(*(pCtrs->getPointer(m_ctrNumber)));
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
	return std::string("FI(") + m_id + ")";
}

ALUCounter* AluUnitFieldIdentifier::evaluate()
{
	if (!g_fieldIdentifierGetter) {
		MYTHROW("Field Identifier Getter is not set")
	}
	std::string content = g_fieldIdentifierGetter->Get(m_id);
	return new ALUCounter(content);
}

#define X(nm,st)	if (s==st) {m_op = UnaryOp__##nm; return;}
void AluUnitUnaryOperator::setOpByName(std::string& s)
{
	ALU_UOP_LIST
	std::string err = "Invalid unary operand: <"+s+">";
	MYTHROW(err);
}
#undef X

AluUnitUnaryOperator::AluUnitUnaryOperator(std::string& s)
{
	setOpByName(s);
}

AluUnitUnaryOperator::AluUnitUnaryOperator(const char* str)
{
	std::string s(str);
	setOpByName(s);
}

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

#define X(nm,st)	case UnaryOp__##nm: return ret + st + ")"; break;
std::string AluUnitUnaryOperator::_identify()
{
	std::string ret = std::string("UOP(");
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
	ALUCounterDeleter _op(operand);
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
		if (operand->isFloat()) {
			return new ALUCounter(operand->getFloat());
		} else {
			return new ALUCounter(operand->getInt());
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
		if (operand->isFloat()) {
			return new ALUCounter(-operand->getFloat());
		} else {
			return new ALUCounter(-operand->getInt());
		}
	default:
		MYTHROW("Invalid operand type");
	}
}



#define X(nm,st,prio)	if (s==st) {m_op = BinaryOp__##nm; m_priority = prio; return;}
void AluBinaryOperator::setOpByName(std::string& s)
{
	ALU_BOP_LIST
	std::string err = "Invalid binary operand: <"+s+">";
	MYTHROW(err);
}
#undef X

AluBinaryOperator::AluBinaryOperator(std::string& s)
{
	setOpByName(s);
}

AluBinaryOperator::AluBinaryOperator(const char* str)
{
	std::string s(str);
	setOpByName(s);
}

#define X(nm,st,prio)	case BinaryOp__##nm: os << st; break;
void AluBinaryOperator::_serialize(std::ostream& os) const
{
	switch(m_op) {
	ALU_BOP_LIST
	default:
		MYTHROW("Invalid binary operand");
	};
}
#undef X

#define X(nm,st,prio)	case BinaryOp__##nm: return ret + st + ")"; break;
std::string AluBinaryOperator::_identify()
{
	std::string ret = std::string("BOP(");
	switch (m_op) {
	ALU_BOP_LIST
	default:
		MYTHROW("Invalid binary operand");
		return ""; // prevent warning
	}
}
#undef X

#define X(nm,st,prio)	case BinaryOp__##nm: return compute##nm(op1, op2);
ALUCounter*		AluBinaryOperator::compute(ALUCounter* op1, ALUCounter* op2)
{
	ALUCounterDeleter _op1(op1);
	ALUCounterDeleter _op2(op2);
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
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
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
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
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
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
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
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
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



#define X(nm,st)	if (s==st) {m_op = AssnOp__##nm; return;}
void AluAssnOperator::setOpByName(std::string& s)
{
	ALU_ASSOP_LIST
	std::string err = "Invalid assignment operator: <"+s+">";
	MYTHROW(err);
}
#undef X

AluAssnOperator::AluAssnOperator(std::string& s)
{
	setOpByName(s);
}

AluAssnOperator::AluAssnOperator(const char* str)
{
	std::string s(str);
	setOpByName(s);
}

#define X(nm,st)	case AssnOp__##nm: os << st; break;
void AluAssnOperator::_serialize(std::ostream& os) const
{
	switch(m_op) {
	ALU_ASSOP_LIST
	default:
		MYTHROW("Invalid assignment operand");
	};
}
#undef X

#define X(nm,st)	case AssnOp__##nm: return ret + st + ")"; break;
std::string AluAssnOperator::_identify()
{
	std::string ret = std::string("ASS(");
	switch (m_op) {
	ALU_ASSOP_LIST
	default:
		MYTHROW("Invalid assignment operand");
		return ""; // prevent warning
	}
}
#undef X

#define X(nm,st)	case AssnOp__##nm: result = compute##nm(operand, prevOp); break;
void		AluAssnOperator::perform(ALUCounterKey ctrNumber, ALUCounters* ctrs, ALUCounter* operand)
{
	ALUCounter* result;
	ALUCounterDeleter _op(operand);

	ALUCounter* prevOp = ctrs->getPointer(ctrNumber);

	// NaN is contagious
	if (counterType__None==operand->getType() ||
			(m_op!=AssnOp__Let && counterType__None==prevOp->getType())) {
		ctrs->set(ctrNumber);
		return;
	}

	switch (m_op) {
	ALU_ASSOP_LIST
	default:
		MYTHROW("Invalid assignment operator");
	}

	ALUCounterDeleter _res((result==operand) ? NULL : result);

	switch (result->getDivinedType()) {
	case counterType__None:
		ctrs->set(ctrNumber);
		break;
	case counterType__Str:
	{
		std::string s = result->getStr();
		ctrs->set(ctrNumber, s);
		break;
	}
	case counterType__Int:
		ctrs->set(ctrNumber, result->getInt());
		break;
	case counterType__Float:
		ctrs->set(ctrNumber, result->getFloat());
		break;
	default:
		MYTHROW("Invalid assignment result");
	}
}
#undef X

ALUCounter* AluAssnOperator::computeLet(ALUCounter* operand, ALUCounter* prevOp)
{
	// Previous value is ignored
	return operand;
}

ALUCounter* AluAssnOperator::computeAdd(ALUCounter* operand, ALUCounter* prevOp)
{
	if (counterType__Float==operand->getType() || counterType__Float==prevOp->getType()) {
		return new ALUCounter(prevOp->getFloat() + operand->getFloat());
	}
	if (counterType__Int==operand->getType() && counterType__Int==prevOp->getType()) {
		return new ALUCounter(prevOp->getInt() + operand->getInt());
	}
	if (operand->isWholeNumber() && prevOp->isWholeNumber()) {
		return new ALUCounter(prevOp->getInt() + operand->getInt());
	}
	return new ALUCounter(prevOp->getFloat() + operand->getFloat());
}

ALUCounter* AluAssnOperator::computeSub(ALUCounter* operand, ALUCounter* prevOp)
{
	if (counterType__Float==operand->getType() || counterType__Float==prevOp->getType()) {
		return new ALUCounter(prevOp->getFloat() - operand->getFloat());
	}
	if (counterType__Int==operand->getType() && counterType__Int==prevOp->getType()) {
		return new ALUCounter(prevOp->getInt() - operand->getInt());
	}
	if (operand->isWholeNumber() && prevOp->isWholeNumber()) {
		return new ALUCounter(prevOp->getInt() - operand->getInt());
	}
	return new ALUCounter(prevOp->getFloat() - operand->getFloat());
}

ALUCounter* AluAssnOperator::computeMult(ALUCounter* operand, ALUCounter* prevOp)
{
	if (counterType__Float==operand->getType() || counterType__Float==prevOp->getType()) {
		return new ALUCounter(prevOp->getFloat() * operand->getFloat());
	}
	if (counterType__Int==operand->getType() && counterType__Int==prevOp->getType()) {
		return new ALUCounter(prevOp->getInt() * operand->getInt());
	}
	if (operand->isWholeNumber() && prevOp->isWholeNumber()) {
		return new ALUCounter(prevOp->getInt() * operand->getInt());
	}
	return new ALUCounter(prevOp->getFloat() * operand->getFloat());
}

ALUCounter* AluAssnOperator::computeDiv(ALUCounter* operand, ALUCounter* prevOp)
{
	// guard against divide-by-zero: return NaN
	if (0.0==operand->getFloat()) {
		return new ALUCounter();
	}
	if (counterType__Float==operand->getType() || counterType__Float==prevOp->getType()) {
		return new ALUCounter(prevOp->getFloat() / operand->getFloat());
	}
	if (counterType__Int==operand->getType() && counterType__Int==prevOp->getType()) {
		if (0==(prevOp->getInt() % operand->getInt())) {
			return new ALUCounter(prevOp->getInt() / operand->getInt());
		} else {
			return new ALUCounter(prevOp->getFloat() / operand->getFloat());
		}
	}
	if (operand->isWholeNumber() && prevOp->isWholeNumber() && (0==(prevOp->getInt() % operand->getInt()))) {
		return new ALUCounter(prevOp->getInt() / operand->getInt());
	}
	return new ALUCounter(prevOp->getFloat() / operand->getFloat());
}

ALUCounter* AluAssnOperator::computeIntDiv(ALUCounter* operand, ALUCounter* prevOp)
{
	// guard against divide-by-zero: return NaN
	if (0.0==operand->getFloat()) {
		return new ALUCounter();
	}

	return new ALUCounter(prevOp->getInt() / operand->getInt());
}

ALUCounter* AluAssnOperator::computeRemDiv(ALUCounter* operand, ALUCounter* prevOp)
{
	// guard against divide-by-zero: return NaN
	if (0.0==operand->getFloat()) {
		return new ALUCounter();
	}

	return new ALUCounter(prevOp->getInt() % operand->getInt());
}

ALUCounter* AluAssnOperator::computeAppnd(ALUCounter* operand, ALUCounter* prevOp)
{
	std::string ret = prevOp->getStr() + operand->getStr();
	return new ALUCounter(ret);
}


void AluOtherToken::_serialize(std::ostream& os) const
{
	switch(m_type) {
	case UT_OpenParenthesis:
		os << '(';
		break;
	case UT_ClosingParenthesis:
		os << ')';
		break;
	case UT_Comma:
		os << ',';
		break;
	default:
		MYTHROW("Unexpected token");
	}
}

std::string AluOtherToken::_identify()
{
	switch(m_type) {
	case UT_OpenParenthesis:
		return "(";
	case UT_ClosingParenthesis:
		return ")";
	case UT_Comma:
		return "COMMA";
	default:
		MYTHROW("Unexpected token");
	}
}

/*
 *
 *
 * ALU FUNCTIONS
 * =============
 *
 *
 */
ALUCounter* AluFunc_abs(ALUCounter* op)
{
	if (op->getType()==counterType__Int) {
		ALUInt i = op->getInt();
		if (i<0) i = -i;
		return new ALUCounter(i);
	} else {
		ALUFloat f = op->getFloat();
		if (f<0) f = -f;
		return new ALUCounter(f);
	}
}

ALUCounter* AluFunc_pow(ALUCounter* op1, ALUCounter* op2)
{
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return new ALUCounter(std::pow(op1->getFloat(), op2->getFloat()));
	}
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
		return new ALUCounter(ALUInt(std::pow(op1->getInt(), op2->getInt())));
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return new ALUCounter(ALUInt(std::pow(op1->getInt(), op2->getInt())));
	}
	return new ALUCounter(std::pow(op1->getFloat(), op2->getFloat()));
}

ALUCounter* AluFunc_sqrt(ALUCounter* op)
{
	return new ALUCounter(std::sqrt(op->getFloat()));
}

#define ALU_FUNCTION_LIST 		\
	X(abs,1)					\
	X(pow,2)					\
	X(sqrt,1)					\

typedef ALUCounter* (*AluFunc0)();
typedef ALUCounter* (*AluFunc1)(ALUCounter* op1);
typedef ALUCounter* (*AluFunc2)(ALUCounter* op1, ALUCounter* op2);
typedef ALUCounter* (*AluFunc3)(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3);
typedef ALUCounter* (*AluFunc4)(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3, ALUCounter* op4);


#define X(nm,cnt)	if (s==#nm) {m_FuncName = s; m_ArgCount = cnt; mp_Func = (void*)AluFunc_##nm; return;}
AluFunction::AluFunction(std::string& _s)
{
	std::string s(_s);
	std::transform(s.begin(), s.end(),s.begin(), ::tolower);
	ALU_FUNCTION_LIST
	std::string err = "Unrecognized function "+_s;
	MYTHROW(err);
}
#undef X

void AluFunction::_serialize(std::ostream& os) const
{
	os << m_FuncName;
}

ALUCounter* AluFunction::evaluate()
{
	if (0 != countOperands()) return AluUnit::evaluate();
	return (AluFunc0(mp_Func))();
}

ALUCounter* AluFunction::compute(ALUCounter* op1)
{
	if (1 != countOperands()) return AluUnit::compute(op1);
	return (AluFunc1(mp_Func))(op1);
}

ALUCounter* AluFunction::compute(ALUCounter* op1, ALUCounter* op2)
{
	if (2 != countOperands()) return AluUnit::compute(op1,op2);
	return (AluFunc2(mp_Func))(op1,op2);
}

ALUCounter* AluFunction::compute(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3)
{
	if (3 != countOperands()) return AluUnit::compute(op1,op2,op3);
	return (AluFunc3(mp_Func))(op1,op2,op3);
}

ALUCounter* AluFunction::compute(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3, ALUCounter* op4)
{
	if (4 != countOperands()) return AluUnit::compute(op1,op2,op3,op4);
	return (AluFunc4(mp_Func))(op1,op2,op3,op4);
}


/*
 * Code for parsing expressions and assignments
 */

static bool isDigit(char c) {
	return (c>='0' && c<='9');
}

static bool isLetter(char c) {
	return ((c>='a' && c<='z') || (c>='A' && c<='Z'));
}

#define X(nm,st) if (s==st) return new AluUnitUnaryOperator(s);
static AluUnit* getUnaryOperator(std::string& s)
{
	ALU_UOP_LIST
	return NULL;
}
#undef X

#define X(nm,st,prio) if (s==st) return new AluBinaryOperator(s);
static AluUnit* getBinaryOperator(std::string& s)
{
	ALU_BOP_LIST
	return NULL;
}
#undef X

static AluAssnOperator* getAssnOperator(std::string& s)
{
#define X(nm,st) if (s==st) return new AluAssnOperator(s);
	ALU_ASSOP_LIST
#undef X
	return NULL;
}

bool parseAluExpression(std::string& s, AluVec& vec)
{
	char* c = (char*)s.c_str();
	char* cEnd = c + s.length();
	AluUnit* pUnit;
	AluUnitType prevUnitType = UT_None;

	if (!vec.empty()){
		MYTHROW("Entered with non-empty vec.");
	}

	while (c<cEnd) {
		// skip over whitespace (if any)
		while (c<cEnd && *c<=32) c++;

		if (c==cEnd) break;

		// First character a digit, a dot, or a minus sign?
		if (*c=='.' || (*c=='-' && (c+1 < cEnd) && isDigit(*(c+1))) || isDigit(*c)) {
			unsigned int countDots = (*c=='.') ? 1 : 0;
			char* tokEnd = c+1;
			while (tokEnd < cEnd) {
				if (*tokEnd=='.' && countDots==0) {
					countDots++;
				} else if (*tokEnd>='0' && *tokEnd<='9') {
					// nothing
				} else {
					break;
				}
				tokEnd++;
			}
			std::string num(c,(tokEnd-c));
			pUnit = new AluUnitLiteral(num,true);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			continue;
		}

		// hash-sign followed by a number is a counter
		if (*c=='#') {
			c++;
			char* tokEnd = c;
			while (tokEnd<cEnd && *tokEnd>='0' && *tokEnd<='9') tokEnd++;
			std::string num(c,(tokEnd-c));
			pUnit = new AluUnitCounter(std::stoi(num));
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			continue;
		}

		// string of letters and digits. May be function or field identifier
		if (isLetter(*c)) {
			char *tokEnd = c+1;
			while (tokEnd<cEnd && (isLetter(*tokEnd) || isDigit(*tokEnd))) tokEnd++;
			if (tokEnd == c+1) {
				pUnit = new AluUnitFieldIdentifier(*c);
			} else {
				std::string identifier(c,(tokEnd-c));
				pUnit = new AluFunction(identifier);
			}
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			continue;
		}

		// parenthesis
		if (*c=='(') {
			pUnit = new AluOtherToken(UT_OpenParenthesis);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c++;
			continue;
		}

		if (*c==')') {
			pUnit = new AluOtherToken(UT_ClosingParenthesis);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c++;
			continue;
		}

		if (*c==',') {
			pUnit = new AluOtherToken(UT_Comma);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c++;
			continue;
		}

		// string of operator characters
		static std::string operatorChars = "=+-*/|!<>:&%";
		if (operatorChars.find(*c)!=std::string::npos) {
			char* tokEnd = c+1;
			while (tokEnd<cEnd && std::string::npos!=operatorChars.find(*tokEnd)) tokEnd++;
			std::string operatr(c,(tokEnd-c));
			if (prevUnitType==UT_None || prevUnitType==UT_OpenParenthesis || prevUnitType==UT_BinaryOp) {
				pUnit = getUnaryOperator(operatr);
			} else {
				pUnit = getBinaryOperator(operatr);
			}
			if (!pUnit) {
				pUnit = getAssnOperator(operatr);
			}
			if (!pUnit) {
				std::string err = "Operator '"+operatr+"' is invalid.";
				MYTHROW(err);
			}
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			continue;
		}

		// Anything else is a string literal
		if (1) {
			char* tokEnd = c+1;
			while (tokEnd < cEnd && *tokEnd!=*c) tokEnd++;
			if (*tokEnd!=*c) {
				std::string err = "Error parsing expression <" + std::string(c) + ">";
				MYTHROW(err);
			}
			std::string sLiteral(c+1, (tokEnd-c-1));
			pUnit = new AluUnitLiteral(sLiteral);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			continue;
		}
	}
	return true;
}

std::string dumpAluVec(AluVec& vec, bool deleteUnits)
{
	std::string ret;
	if (deleteUnits) {
		while (!vec.empty()) {
			if (!ret.empty()) ret += ";";
			ret += vec[0]->_identify();
			vec.erase(vec.begin());
		}
	} else {
		for (int i=0; i<vec.size(); i++) {
			if (!ret.empty()) ret += ";";
			ret += vec[i]->_identify();
		}
	}
	return ret;
}

bool parseAluStatement(std::string& s, ALUCounterKey& k, AluAssnOperator* pAss, AluVec& vec)
{
	if (false==parseAluExpression(s, vec)) {
		return false;
	}

	// First item in the vector must be a counter key, the second is an assignment operator
	// Need to remove them both.

	if (vec.size() < 3) {
		std::string err = "Expression <" + s + "> is not a valid ALU assignment";
		MYTHROW(err);
	}

	if (UT_Counter!=vec[0]->type()) {
		std::string err = "ALU assignment statements must begin with a counter. Got " + vec[0]->_identify() + " instead.";
		MYTHROW(err);
	}

	if (UT_AssignmentOp!=vec[1]->type()) {
		std::string err = "ALU assignment statements must have an assignment operator as the second element. Got " + vec[1]->_identify() + " instead.";
		MYTHROW(err);
	}

	AluUnitCounter* ctr = dynamic_cast<AluUnitCounter*>(vec[0]);
	*pAss = *(dynamic_cast<AluAssnOperator*>(vec[1]));
	k = ctr->getKey();

	vec.erase(vec.begin());
	vec.erase(vec.begin());
	delete ctr;

	// Check that we don't have something terrible like an assignment operator in the
	// expression. Like this is C or something
	for (AluUnit* pUnit : vec) {
		if (UT_AssignmentOp==pUnit->type()) {
			MYTHROW("ALU expression should not contain an assignment operator");
		}
	}
	return true;
}

bool isHigherPrecedenceBinaryOp(AluUnit* op1, AluUnit* op2)
{
	AluBinaryOperator *bop1 = dynamic_cast<AluBinaryOperator*>(op1);
	AluBinaryOperator *bop2 = dynamic_cast<AluBinaryOperator*>(op2);

	if (!bop1 || !bop2) {
		MYTHROW("Received two units. Not both are binary operators");
	}

	return bop1->priority() >= bop2->priority();
}

/*
 * Function: convertAluVecToPostfix
 * Implements the Shunting-Yard algorithm to convert an infix expression
 * to a postfix expression for easier calculation later on.
 */
bool convertAluVecToPostfix(AluVec& source, AluVec& dest, bool clearSource)
{
	std::stack<AluUnit*> operatorStack;
	unsigned int availableOperands = 0;

	if (!dest.empty()){
		MYTHROW("Entered with non-empty vec.");
	}

	for (AluUnit* pUnit : source) {
		switch (pUnit->type()) {
		case UT_Comma:
			if (clearSource) delete pUnit;
			break;
		case UT_AssignmentOp:
			MYTHROW("Assignment operator used in expression");
			break;
		case UT_None:
		case UT_Invalid:
			MYTHROW("None or Invalid - internal logic error");
		case UT_LiteralNumber:
		case UT_Counter:
		case UT_FieldIdentifier:
			dest.push_back(pUnit);
			availableOperands++;
			break;
		case UT_Identifier: 	// a function
			operatorStack.push(pUnit);
			break;
		case UT_UnaryOp: { // Unary operator - higher precedence than any binary but not a function
			AluUnitType topType = operatorStack.empty() ? UT_None : operatorStack.top()->type();
			while (topType==UT_Identifier) {
				MYASSERT_WITH_MSG(operatorStack.top()->countOperands() <= availableOperands, "Not enough operands for operator");
				availableOperands = availableOperands + 1 - operatorStack.top()->countOperands();
				dest.push_back(operatorStack.top());
				operatorStack.pop();
				topType = operatorStack.empty() ? UT_None : operatorStack.top()->type();
			}
			operatorStack.push(pUnit);
			break;
		}
		case UT_BinaryOp: { // Binary operator - lower than unary and an interesting rank among them
			AluUnitType topType = operatorStack.empty() ? UT_None : operatorStack.top()->type();
			while (topType==UT_Identifier || topType==UT_UnaryOp ||
					(topType==UT_BinaryOp && isHigherPrecedenceBinaryOp(operatorStack.top(),pUnit))) {
				MYASSERT_WITH_MSG(operatorStack.top()->countOperands() <= availableOperands, "Not enough operands for operator");
				availableOperands = availableOperands + 1 - operatorStack.top()->countOperands();
				dest.push_back(operatorStack.top());
				operatorStack.pop();
				topType = operatorStack.empty() ? UT_None : operatorStack.top()->type();
			}
			operatorStack.push(pUnit);
			break;
		}
		case UT_OpenParenthesis:
			operatorStack.push(pUnit);
			break;
		case UT_ClosingParenthesis:
			while (!operatorStack.empty() && UT_OpenParenthesis!=operatorStack.top()->type()) {
				MYASSERT_WITH_MSG(operatorStack.top()->countOperands() <= availableOperands, "Not enough operands for operator");
				availableOperands = availableOperands + 1 - operatorStack.top()->countOperands();
				dest.push_back(operatorStack.top());
				operatorStack.pop();
			}
			if (operatorStack.empty()) {
				MYTHROW("Mismatched parenthesis -- too many closing");
			} else {
				if (clearSource) {
					delete pUnit;
					delete operatorStack.top();
				}
				operatorStack.pop(); // Pop off the opening parenthesis
			}
		}
	}

	while (!operatorStack.empty()) {
		if (operatorStack.top()->type()==UT_OpenParenthesis) {
			MYTHROW("Mismatched parenthesis -- too many opening");
		}
		AluUnit* pTopUnit = operatorStack.top();
		operatorStack.pop();
		MYASSERT_WITH_MSG(pTopUnit->countOperands()<=dest.size(), "Not enough operands for operator");
		dest.push_back(pTopUnit);
	}

	if (clearSource) {
		source.clear();
	}

	return true;
}

ALUCounter* evaluateExpression(AluVec& expr, ALUCounters* pctrs)
{
	std::stack<ALUCounter*> computeStack;
	ALUCounter* arg1;
	ALUCounter* arg2;
	ALUCounter* arg3;
	ALUCounter* arg4;

	for (AluUnit* pUnit : expr) {
		switch (pUnit->type()) {
		case UT_LiteralNumber:
		case UT_FieldIdentifier:
			computeStack.push(pUnit->evaluate());
			break;
		case UT_Counter: {
			AluUnitCounter* pCtr = dynamic_cast<AluUnitCounter*>(pUnit);
			computeStack.push(pCtr->compute(pctrs));
			break;
		}
		case UT_UnaryOp:
			MYASSERT(computeStack.size()>=1);
			arg1 = computeStack.top();
			computeStack.pop();
			computeStack.push(pUnit->compute(arg1));
			break;
		case UT_BinaryOp:
			MYASSERT(computeStack.size()>=2);
			arg2 = computeStack.top();
			computeStack.pop();
			arg1 = computeStack.top();
			computeStack.pop();
			computeStack.push(pUnit->compute(arg1,arg2));
			break;
		case UT_Identifier: {
			MYASSERT(computeStack.size() >= pUnit->countOperands());
			MYASSERT(pUnit->countOperands() <= 4);

			switch (pUnit->countOperands()) {
			case 4:
				arg4 = computeStack.top();
				computeStack.pop();
			case 3:
				arg3 = computeStack.top();
				computeStack.pop();
			case 2:
				arg2 = computeStack.top();
				computeStack.pop();
			case 1:
				arg1 = computeStack.top();
				computeStack.pop();
			default:
				break;
			}

			switch (pUnit->countOperands()) {
			case 0:
				computeStack.push(pUnit->evaluate());
				break;
			case 1:
				computeStack.push(pUnit->compute(arg1));
				delete arg1;
				break;
			case 2:
				computeStack.push(pUnit->compute(arg1, arg2));
				delete arg1;
				delete arg2;
				break;
			case 3:
				computeStack.push(pUnit->compute(arg1, arg2, arg3));
				delete arg1;
				delete arg2;
				delete arg3;
				break;
			case 4:
				computeStack.push(pUnit->compute(arg1, arg2, arg3, arg4));
				delete arg1;
				delete arg2;
				delete arg3;
				delete arg4;
				break;
			default:
				break;
			}
			break;
			default:
				MYTHROW("Logic error, should not have gotten this type of Unit");
		}
		}
	}

	MYASSERT(computeStack.size() == 1);
	ALUCounter* ret = computeStack.top();
	computeStack.pop();
	return ret;
}

void ALUPerformAssignment(ALUCounterKey& k, AluAssnOperator* pAss, AluVec& expr, ALUCounters* pctrs)
{
	ALUCounter* exprResult = evaluateExpression(expr, pctrs);

	pAss->perform(k, pctrs, exprResult);
}
