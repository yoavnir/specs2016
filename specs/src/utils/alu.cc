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
#include <algorithm>
#include <iomanip>
#include "ErrorReporting.h"
#include "alu.h"
#include "aluFunctions.h"
#include "processing/Config.h"  // for configured literals

extern stateQueryAgent* g_pStateQueryAgent;

void ALUValue::set(std::string& s)
{
	m_value = s;
	m_type = counterType__Str;
}

void ALUValue::set(const std::string& s)
{
	m_value = s;
	m_type = counterType__Str;
}

void ALUValue::set(const char* st)
{
	std::string s(st);
	set(s);
}

void ALUValue::set(ALUInt l)
{
	m_value = std::to_string(l);
	m_type = counterType__Int;
}

void ALUValue::set(ALUFloat f)
{
	if (std::isnan(f)) {
		m_type = counterType__None;  /* NaN */
		return;
	}
    std::ostringstream ost;
    ost.precision(ALUFloatPrecision);
    ost << f;
	m_value = ost.str();
	m_type = counterType__Float;
}

void ALUValue::set()
{
	m_type = counterType__None;
}

ALUInt ALUValue::getInt() const
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
	} catch (std::out_of_range& e) {
		std::string err = "Out of range trying to convert " + m_value + " to Int";
		MYTHROW(err);
	}
}

ALUInt ALUValue::getHex() const
{
	try {
		return (counterType__None==m_type) ? 0 : std::stoll(m_value, nullptr, 16);
	} catch (std::invalid_argument& e) {
		return 0;
	}
}

ALUFloat ALUValue::getFloat() const
{
	try {
		return (counterType__None==m_type) ? 0.0 : std::stold(m_value);
	} catch (std::invalid_argument& e) {
		return 0;
	}
}

bool ALUValue::getBool() const
{
	return (counterType__None!=m_type && m_value!="" && m_value!="0" && m_value!="0.0");
}

bool ALUValue::isWholeNumber() const
{
	if (!isNumeric()) return false;
	ALUFloat f = getFloat();
	return (f==std::floor(f));
}

bool ALUValue::isFloat() const
{
	return isNumeric() && (!isWholeNumber() || m_value.find('.')!=std::string::npos);
}

bool ALUValue::isNumeric() const
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
	} catch (std::out_of_range& e) {
		return false;
	} catch (std::invalid_argument& e) {
		return false;
	}
	}
}

ALUCounterType ALUValue::getDivinedType() const
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

PValue AluUnit::evaluate()
{
	std::string err = _identify() + " should not be called with no operands";
	MYTHROW(err);
}

PValue AluUnit::compute(PValue op)
{
	std::string err = _identify() + " should not be called with one operand";
	MYTHROW(err);
}

PValue AluUnit::compute(PValue op1, PValue op2)
{
	std::string err = _identify() + " should not be called with two operands";
	MYTHROW(err);
}

PValue AluUnit::compute(PValue op1, PValue op2, PValue op3)
{
	std::string err = _identify() + " should not be called with three operands";
	MYTHROW(err);
}

PValue AluUnit::compute(PValue op1, PValue op2, PValue op3, PValue op4)
{
	std::string err = _identify() + " should not be called with four operands";
	MYTHROW(err);
}

PValue AluUnit::compute(PValue op1, PValue op2, PValue op3, PValue op4, PValue op5)
{
	std::string err = _identify() + " should not be called with five operands";
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

PValue AluUnitLiteral::evaluate()
{
	PValue ret = mkValue(m_literal);
	if (m_hintNumerical) ret->divineType();
	return ret;
}

void AluUnitNull::_serialize(std::ostream& os) const
{
	os << "dummy";
}

std::string AluUnitNull::_identify()
{
	return std::string("Dummy");
}

PValue AluUnitNull::evaluate()
{
	return nullptr;
}


void AluUnitCounter::_serialize(std::ostream& os) const
{
	os << '#' << m_ctrNumber;
}

std::string AluUnitCounter::_identify()
{
	return std::string("Counter(") + std::to_string(m_ctrNumber) + ")";
}

PValue AluUnitCounter::compute(ALUCounters* pCtrs)
{
	return mkValue(*(pCtrs->getPointer(m_ctrNumber)));
}

static fieldIdentifierGetter* g_fieldIdentifierGetter = nullptr;

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
	if (m_ReturnIdentifier) {
		return std::string("FI(") + m_id + " - by name)";
	}
	return std::string("FI(") + m_id + ")";
}

PValue AluUnitFieldIdentifier::evaluate()
{
	if (m_ReturnIdentifier) {
		return mkValue(ALUInt(m_id));
	}
	if (!g_fieldIdentifierGetter) {
		MYTHROW("Field Identifier Getter is not set")
	}
	return mkValue(g_fieldIdentifierGetter->Get(m_id));
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
PValue		AluUnitUnaryOperator::compute(PValue operand)
{
	if (counterType__None==operand->getType()) {
		return mkValue0();
	}
	switch (m_op) {
	ALU_UOP_LIST
	default:
		MYTHROW("Invalid unary operand");
	}
}
#undef X

#define RETURN_FALSE return mkValue(ALUInt(0))
#define RETURN_TRUE  return mkValue(ALUInt(1))

#define RETURN_COND(cond) { if ((cond)) { RETURN_TRUE; } else { RETURN_FALSE; } }

PValue 	AluUnitUnaryOperator::computeNot(PValue operand)
{
	RETURN_COND(false==operand->getBool());
}

PValue 	AluUnitUnaryOperator::computePlus(PValue operand)
{
	switch (operand->getType()) {
	case counterType__Float:
	case counterType__Int:
		return mkValue(*operand);
	case counterType__Str:
		if (operand->isFloat()) {
			return mkValue(operand->getFloat());
		} else {
			return mkValue(operand->getInt());
		}
	default:
		MYTHROW("Invalid operand type");
	}
}

PValue 	AluUnitUnaryOperator::computeMinus(PValue operand)
{
	switch (operand->getType()) {
	case counterType__Float:
		return mkValue(-operand->getFloat());
	case counterType__Int:
		return mkValue(-operand->getInt());
	case counterType__Str:
		if (operand->isFloat()) {
			return mkValue(-operand->getFloat());
		} else {
			return mkValue(-operand->getInt());
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
PValue		AluBinaryOperator::compute(PValue op1, PValue op2)
{
	if (counterType__None==op1->getType() || counterType__None==op2->getType()) {
		return mkValue0();
	}

	switch (m_op) {
	ALU_BOP_LIST
	default:
		MYTHROW("Invalid unary operand");
	}
}
#undef X

// Simple floating point or integer addition
PValue		AluBinaryOperator::computeAdd(PValue op1, PValue op2)
{
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return mkValue(op1->getFloat() + op2->getFloat());
	}
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
		return mkValue(op1->getInt() + op2->getInt());
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return mkValue(op1->getInt() + op2->getInt());
	}
	return mkValue(op1->getFloat() + op2->getFloat());
}

// Simple floating point or integer subtraction
PValue		AluBinaryOperator::computeSub(PValue op1, PValue op2)
{
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return mkValue(op1->getFloat() - op2->getFloat());
	}
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
		return mkValue(op1->getInt() - op2->getInt());
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return mkValue(op1->getInt() - op2->getInt());
	}
	return mkValue(op1->getFloat() - op2->getFloat());
}

// Floating point or integer multiplication
PValue		AluBinaryOperator::computeMult(PValue op1, PValue op2)
{
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return mkValue(op1->getFloat() * op2->getFloat());
	}
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
		return mkValue(op1->getInt() * op2->getInt());
	}
	if (op1->isWholeNumber() && op2->isWholeNumber()) {
		return mkValue(op1->getInt() * op2->getInt());
	}
	return mkValue(op1->getFloat() * op2->getFloat());
}

// Numeric division. The result quotient may be floating point even when the
// dividend and divisor are both integers.
PValue		AluBinaryOperator::computeDiv(PValue op1, PValue op2)
{
	// guard against divide-by-zero: return NaN
	if (counterType__None!=op2->getType() && 0.0==op2->getFloat()) {
		return mkValue0();
	}
	if (counterType__Float==op1->getType() || counterType__Float==op2->getType()) {
		return mkValue(op1->getFloat() / op2->getFloat());
	}
	if (counterType__Int==op1->getType() && counterType__Int==op2->getType()) {
		if (0==(op1->getInt() % op2->getInt())) {
			return mkValue(op1->getInt() / op2->getInt());
		} else {
			return mkValue(op1->getFloat() / op2->getFloat());
		}
	}
	if (op1->isWholeNumber() && op2->isWholeNumber() && (0==(op1->getInt() % op2->getInt()))) {
		return mkValue(op1->getInt() / op2->getInt());
	}
	return mkValue(op1->getFloat() / op2->getFloat());
}

// String concatenation ||
PValue		AluBinaryOperator::computeAppnd(PValue op1, PValue op2)
{
	return mkValue(op1->getStr() + op2->getStr());
}

// Integer division. Divides the integer form of both numbers: 19.5 % 2.001 = 9
PValue		AluBinaryOperator::computeIntDiv(PValue op1, PValue op2)
{
	// guard against divide-by-zero: return NaN
	if (counterType__None!=op2->getType() && 0.0==op2->getFloat()) {
		return mkValue0();
	}

	return mkValue(op1->getInt() / op2->getInt());
}

// Remainder in integer division. The operator is //
PValue		AluBinaryOperator::computeRemDiv(PValue op1, PValue op2)
{
	// guard against divide-by-zero: return NaN
	if (counterType__None!=op2->getType() && 0.0==op2->getFloat()) {
		return mkValue0();
	}

	return mkValue(op1->getInt() % op2->getInt());
}

// Logical AND. Always returns zero or one.
PValue		AluBinaryOperator::computeAND(PValue op1, PValue op2)
{
	RETURN_COND(op1->getBool() && op2->getBool());
}

// Logical OR. Always returns zero or one.
PValue		AluBinaryOperator::computeOR(PValue op1, PValue op2)
{
	RETURN_COND(op1->getBool() || op2->getBool());
}

// Simple floating point or string comparison
PValue		AluBinaryOperator::computeEQ(PValue op1, PValue op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() == op2->getFloat());
	}

	RETURN_COND(op1->getStr() == op2->getStr());
}

// Simple floating point or string comparison
PValue		AluBinaryOperator::computeNE(PValue op1, PValue op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() != op2->getFloat());
	}

	RETURN_COND(op1->getStr() != op2->getStr());
}

// Simple floating point or string comparison
PValue		AluBinaryOperator::computeGT(PValue op1, PValue op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() > op2->getFloat());
	}

	RETURN_COND(op1->getStr().compare(op2->getStr()) > 0);
}

// Simple floating point or string comparison
PValue		AluBinaryOperator::computeGE(PValue op1, PValue op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() >= op2->getFloat());
	}

	RETURN_COND(op1->getStr().compare(op2->getStr()) >= 0);
}

// Simple floating point or string comparison
PValue		AluBinaryOperator::computeLT(PValue op1, PValue op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() < op2->getFloat());
	}

	RETURN_COND(op1->getStr().compare(op2->getStr()) < 0);
}

// Simple floating point or string comparison
PValue		AluBinaryOperator::computeLE(PValue op1, PValue op2)
{
	if (op1->isNumeric() || op2->isNumeric()) {
		RETURN_COND(op1->getFloat() <= op2->getFloat());
	}

	RETURN_COND(op1->getStr().compare(op2->getStr()) <= 0);
}

// Strict floating point or string comparison
PValue		AluBinaryOperator::computeSEQ(PValue op1, PValue op2)
{
	RETURN_COND(op1->getStr() == op2->getStr());
}

// Strict floating point or string comparison
PValue		AluBinaryOperator::computeSNE(PValue op1, PValue op2)
{
	RETURN_COND(op1->getStr() != op2->getStr());
}

// Strict floating point or string comparison
PValue		AluBinaryOperator::computeSGT(PValue op1, PValue op2)
{
	RETURN_COND(op1->getStr().compare(op2->getStr()) > 0);
}

// Strict floating point or string comparison
PValue		AluBinaryOperator::computeSLT(PValue op1, PValue op2)
{
	RETURN_COND(op1->getStr().compare(op2->getStr()) < 0);
}

// Strict floating point or string comparison
PValue		AluBinaryOperator::computeSGTE(PValue op1, PValue op2)
{
	RETURN_COND(op1->getStr().compare(op2->getStr()) >= 0);
}

// Strict floating point or string comparison
PValue		AluBinaryOperator::computeSLTE(PValue op1, PValue op2)
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
void		AluAssnOperator::perform(ALUCounterKey ctrNumber, ALUCounters* ctrs, PValue operand)
{
	PValue result;

	PValue prevOp = ctrs->getPointer(ctrNumber);

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

PValue AluAssnOperator::computeLet(PValue operand, PValue prevOp)
{
	// Previous value is ignored
	return operand;
}

PValue AluAssnOperator::computeAdd(PValue operand, PValue prevOp)
{
	if (counterType__Float==operand->getType() || counterType__Float==prevOp->getType()) {
		return mkValue(prevOp->getFloat() + operand->getFloat());
	}
	if (counterType__Int==operand->getType() && counterType__Int==prevOp->getType()) {
		return mkValue(prevOp->getInt() + operand->getInt());
	}
	if (operand->isWholeNumber() && prevOp->isWholeNumber()) {
		return mkValue(prevOp->getInt() + operand->getInt());
	}
	return mkValue(prevOp->getFloat() + operand->getFloat());
}

PValue AluAssnOperator::computeSub(PValue operand, PValue prevOp)
{
	if (counterType__Float==operand->getType() || counterType__Float==prevOp->getType()) {
		return mkValue(prevOp->getFloat() - operand->getFloat());
	}
	if (counterType__Int==operand->getType() && counterType__Int==prevOp->getType()) {
		return mkValue(prevOp->getInt() - operand->getInt());
	}
	if (operand->isWholeNumber() && prevOp->isWholeNumber()) {
		return mkValue(prevOp->getInt() - operand->getInt());
	}
	return mkValue(prevOp->getFloat() - operand->getFloat());
}

PValue AluAssnOperator::computeMult(PValue operand, PValue prevOp)
{
	if (counterType__Float==operand->getType() || counterType__Float==prevOp->getType()) {
		return mkValue(prevOp->getFloat() * operand->getFloat());
	}
	if (counterType__Int==operand->getType() && counterType__Int==prevOp->getType()) {
		return mkValue(prevOp->getInt() * operand->getInt());
	}
	if (operand->isWholeNumber() && prevOp->isWholeNumber()) {
		return mkValue(prevOp->getInt() * operand->getInt());
	}
	return mkValue(prevOp->getFloat() * operand->getFloat());
}

PValue AluAssnOperator::computeDiv(PValue operand, PValue prevOp)
{
	// guard against divide-by-zero: return NaN
	if (0.0==operand->getFloat()) {
		return mkValue0();
	}
	if (counterType__Float==operand->getType() || counterType__Float==prevOp->getType()) {
		return mkValue(prevOp->getFloat() / operand->getFloat());
	}
	if (counterType__Int==operand->getType() && counterType__Int==prevOp->getType()) {
		if (0==(prevOp->getInt() % operand->getInt())) {
			return mkValue(prevOp->getInt() / operand->getInt());
		} else {
			return mkValue(prevOp->getFloat() / operand->getFloat());
		}
	}
	if (operand->isWholeNumber() && prevOp->isWholeNumber() && (0==(prevOp->getInt() % operand->getInt()))) {
		return mkValue(prevOp->getInt() / operand->getInt());
	}
	return mkValue(prevOp->getFloat() / operand->getFloat());
}

PValue AluAssnOperator::computeIntDiv(PValue operand, PValue prevOp)
{
	// guard against divide-by-zero: return NaN
	if (0.0==operand->getFloat()) {
		return mkValue0();
	}

	return mkValue(prevOp->getInt() / operand->getInt());
}

PValue AluAssnOperator::computeRemDiv(PValue operand, PValue prevOp)
{
	// guard against divide-by-zero: return NaN
	if (0.0==operand->getFloat()) {
		return mkValue0();
	}

	return mkValue(prevOp->getInt() % operand->getInt());
}

PValue AluAssnOperator::computeAppnd(PValue operand, PValue prevOp)
{
	std::string ret = prevOp->getStr() + operand->getStr();
	return mkValue(ret);
}

void AluInputRecord::_serialize(std::ostream& os) const
{
	os << "@@";
}

PValue AluInputRecord::evaluate()
{
	PSpecString ps = g_pStateQueryAgent->getFromTo(1,-1);
	PValue ret;
	if (ps) {
		ret = mkValue2(ps->data(), ps->length());
	} else {
		ret = mkValue("");
	}
	return ret;
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


// AluFunction class

unsigned char AluFunction::m_flags = ALUFUNC_REGULAR;

#define X(nm,cnt,flags,rl,shorthelp,longhelp)	if (s==#nm) {  \
		m_FuncName = s; m_ArgCount = cnt;  \
		m_reliesOnInput = rl;              \
		mp_Func = (void*)AluFunc_##nm;     \
		m_flags |= flags;                  \
		return;                            \
	}
#define H(hdr,len)
AluFunction::AluFunction(std::string& _s)
{
	std::string s(_s);
	std::transform(s.begin(), s.end(),s.begin(), ::tolower);

	mp_Func = nullptr;
	m_pExternalFunc = nullptr;

	ALU_FUNCTION_LIST
#ifdef DEBUG
	ALU_DEBUG_FUNCTION_LIST
#endif

#ifndef SPECS_NO_PYTHON
	// Internal function not found - try external unless they're disabled
	if (EXTERNAL_FUNC_OFF != g_pythonFuncs) {
		if (!p_gExternalFunctions->IsInitialized()) {
			try {
				p_gExternalFunctions->Initialize(getFullSpecPath());
			} catch (const SpecsException& e) {
				std::cerr << "Python Interface: " << e.what(!g_bVerbose) << "\n";
				exit(0);
			}
		}
		MYASSERT(p_gExternalFunctions->IsInitialized());
		m_pExternalFunc = p_gExternalFunctions->GetFunctionByName(_s);
	}
#endif

	if (!m_pExternalFunc) {
		std::string err = "Unrecognized function "+_s;
		MYTHROW(err);
	}
	m_FuncName = _s;
	m_ArgCount = (unsigned int)(m_pExternalFunc->GetArgCount());
	m_reliesOnInput = false;
	mp_Func = nullptr;
	m_flags |= ALUFUNC_EXTERNAL;
}
#undef X
#undef H

void AluFunction::_serialize(std::ostream& os) const
{
	os << m_FuncName;
}

PValue AluFunction::evaluate()
{
	if (0 != countOperands()) return AluUnit::evaluate();
	if (nullptr != m_pExternalFunc) {
		m_pExternalFunc->ResetArgs();
		return m_pExternalFunc->Call();
	}
	return (AluFunc0(mp_Func))();
}

PValue AluFunction::compute(PValue op1)
{
	if (1 != countOperands()) return AluUnit::compute(op1);
	if (nullptr != m_pExternalFunc) {
		m_pExternalFunc->ResetArgs();
		m_pExternalFunc->setArgValue(0,op1);
		return m_pExternalFunc->Call();
	}
	return (AluFunc1(mp_Func))(op1);
}

PValue AluFunction::compute(PValue op1, PValue op2)
{
	if (2 != countOperands()) return AluUnit::compute(op1,op2);
	if (nullptr != m_pExternalFunc) {
		m_pExternalFunc->ResetArgs();
		m_pExternalFunc->setArgValue(0,op1);
		m_pExternalFunc->setArgValue(1,op2);
		return m_pExternalFunc->Call();
	}
	return (AluFunc2(mp_Func))(op1,op2);
}

PValue AluFunction::compute(PValue op1, PValue op2, PValue op3)
{
	if (3 != countOperands()) return AluUnit::compute(op1,op2,op3);
	if (nullptr != m_pExternalFunc) {
		m_pExternalFunc->ResetArgs();
		m_pExternalFunc->setArgValue(0,op1);
		m_pExternalFunc->setArgValue(1,op2);
		m_pExternalFunc->setArgValue(2,op3);
		return m_pExternalFunc->Call();
	}
	return (AluFunc3(mp_Func))(op1,op2,op3);
}

PValue AluFunction::compute(PValue op1, PValue op2, PValue op3, PValue op4)
{
	if (4 != countOperands()) return AluUnit::compute(op1,op2,op3,op4);
	if (nullptr != m_pExternalFunc) {
		m_pExternalFunc->ResetArgs();
		m_pExternalFunc->setArgValue(0,op1);
		m_pExternalFunc->setArgValue(1,op2);
		m_pExternalFunc->setArgValue(2,op3);
		m_pExternalFunc->setArgValue(3,op4);
		return m_pExternalFunc->Call();
	}
	return (AluFunc4(mp_Func))(op1,op2,op3,op4);
}

PValue AluFunction::compute(PValue op1, PValue op2, PValue op3, PValue op4, PValue op5)
{
	if (5 != countOperands()) return AluUnit::compute(op1,op2,op3,op4,op5);
	if (nullptr != m_pExternalFunc) {
		m_pExternalFunc->ResetArgs();
		m_pExternalFunc->setArgValue(0,op1);
		m_pExternalFunc->setArgValue(1,op2);
		m_pExternalFunc->setArgValue(2,op3);
		m_pExternalFunc->setArgValue(3,op4);
		m_pExternalFunc->setArgValue(4,op5);
		return m_pExternalFunc->Call();
	}
	return (AluFunc5(mp_Func))(op1,op2,op3,op4,op5);
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

static bool isCharInIdentifier(char c) {
	return isLetter(c) || isDigit(c) || c=='_';
}

static bool isFirstCharInIdentifier(char c) {
	return isLetter(c) || c=='_';
}

#define X(nm,st) if (s==st) return std::make_shared<AluUnitUnaryOperator>(s);
static PUnit getUnaryOperator(std::string& s)
{
	ALU_UOP_LIST
	return nullptr;
}
#undef X

#define X(nm,st,prio) if (s==st) return std::make_shared<AluBinaryOperator>(s);
static PUnit getBinaryOperator(std::string& s)
{
	ALU_BOP_LIST
	return nullptr;
}
#undef X

static PUnit getAssnOperator(std::string& s)
{
#define X(nm,st) if (s==st) return std::make_shared<AluAssnOperator>(s);
	ALU_ASSOP_LIST
#undef X
	return nullptr;
}

void dumpAluVec(const char* title, AluVec& vec, int pointer = -1)
{
	std::cerr << title << ": ALU Vector at " << &vec << " with " << vec.size() << " items:\n";
	int index = 0;
	for (PUnit pUnit : vec) {
		if (index==pointer) {
			std::cerr << "   ==> | ";
		} else {
			std::cerr << "       | ";
		}
		std::cerr << std::setw(40) << std::left << pUnit->_identify() << "|" << std::endl;
		index++;
	}
	std::cerr << "       +" << std::setw(42) << std::setfill('-') <<
		std::right << "+" << std::endl << std::setw(0) << std::setfill(' ') << std::endl;
}

void dumpAluStack(const char* title, std::stack<PValue>& stk)
{
	std::cerr << title << ": ALU Stack at " << &stk << " with " << stk.size() << " items:\n";
	std::stack<PValue> tmp;
	while (!stk.empty()) {
		PValue v = stk.top();
		stk.pop();
		std::cerr << "   > " << (v ? v->getStr() : "(nil)") << std::endl;
		tmp.push(v);
	}
	std::cerr << std::endl;
	while (!tmp.empty()) {
		PValue v = tmp.top();
		tmp.pop();
		stk.push(v);
	}
}

void dumpAluStack(const char* title, std::stack<PUnit>& stk)
{
	std::cerr << title << ": ALU Unit Stack at " << &stk << " with " << stk.size() << " items:\n";
	std::stack<PUnit> tmp;
	while (!stk.empty()) {
		PUnit v = stk.top();
		stk.pop();
		std::cerr << "   > " << v->_identify() << std::endl;
		tmp.push(v);
	}
	std::cerr << std::endl;
	while (!tmp.empty()) {
		PUnit v = tmp.top();
		tmp.pop();
		stk.push(v);
	}
}

bool expressionIsAssignment(AluVec& vec)
{
	return (vec.size() > 2 &&
			UT_Counter == vec[0]->type() &&
			UT_AssignmentOp == vec[1]->type());
}

static char nextNonBlankChar(char* pc)
{
	while (*pc==' ') pc++;
	return *pc;
}

bool parseAluExpression(std::string& s, AluVec& vec)
{
	char* c = (char*)s.c_str();
	char* cEnd = c + s.length();
	PUnit pUnit;
	AluUnitType prevUnitType = UT_None;

	if (!vec.empty()){
		MYTHROW("Entered with non-empty vec.");
	}

#ifdef ALU_DUMP
	if (g_bDebugAluCompile) {
		std::cerr << __FUNCTION__ << ": Parsing Expression: " << s << std::endl;
	}
#endif

	bool mayBeStart = true; // will be false after an expression that is not followed by whitespace

	while (c<cEnd) {
		// skip over whitespace (if any)
		while (c<cEnd && *c<=32) {
			c++;
			mayBeStart = true;
		}

		if (c==cEnd) break;

		// First character a digit, a dot, or a minus sign?
		if (*c=='.' ||
				(*c=='-' && mayBeStart && (c+1 < cEnd) && isDigit(*(c+1))) ||
				isDigit(*c)) {
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
			pUnit = std::make_shared<AluUnitLiteral>(num,true);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			mayBeStart = false;
			continue;
		}

		// First character a single or double quote?
		// Only one is really possible because the other is used to delimit
		// the expression, but we don't know that inside...
		if (*c=='\'' || *c=='"') {
			std::string tok;
			char delimiter = *c++;
			char* tokEnd = c;
			while (tokEnd<cEnd && *tokEnd!=delimiter) {
				if (*tokEnd=='\\' && tokEnd < (cEnd-1)) {
					tokEnd++;
				}
				tok += *tokEnd++;
			}
			pUnit = std::make_shared<AluUnitLiteral>(tok);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd+1;
			mayBeStart = false;
			continue;
		}

		// Also a configured string
		if (*c=='@' && isFirstCharInIdentifier(c[1])) {
			char* tokEnd = ++c;
			while (isCharInIdentifier(*tokEnd) && (tokEnd<cEnd)) {
				tokEnd++;
			}
			std::string key = std::string(c,tokEnd-c);
			if (!configSpecLiteralExists(key)) {
				std::string err = "Key '" + key + "' not found in expression.";
				MYTHROW(err);
			}
			pUnit = std::make_shared<AluUnitLiteral>(configSpecLiteralGet(key));
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			mayBeStart = false;
			continue;
		}

		// A special string @@ representing the entire input record
		if (*c=='@' && c[1]=='@')  {
			c+=2;
			pUnit = std::make_shared<AluInputRecord>();
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			mayBeStart = false;
			continue;
		}

		// hash-sign followed by a number is either a counter or a persistent variable
		if (*c=='#') {
			c++;
			char* tokEnd = c;
			if (!isDigit(*c)) {
				// Equivalent to pget(name)
				while (tokEnd<cEnd && (isLetter(*tokEnd) || isDigit(*tokEnd) || *tokEnd=='_')) tokEnd++;
				std::string name(c,(tokEnd-c));
				static std::string pget_identifier("pget");
				pUnit = std::make_shared<AluFunction>(pget_identifier);
				vec.push_back(pUnit);
				pUnit = std::make_shared<AluOtherToken>(UT_OpenParenthesis);
				vec.push_back(pUnit);
				pUnit = std::make_shared<AluUnitLiteral>(name);
				vec.push_back(pUnit);
				pUnit = std::make_shared<AluOtherToken>(UT_ClosingParenthesis);
				vec.push_back(pUnit);
			} else {
				// a counter
				while (tokEnd<cEnd && *tokEnd>='0' && *tokEnd<='9') tokEnd++;
				std::string num(c,(tokEnd-c));
				if ((num.length() == 0) || (num.length() > 3)) {
					std::string err = "Invalid counter <#" + num + *tokEnd + "> in expression";
					MYTHROW(err);
				}
				pUnit = std::make_shared<AluUnitCounter>(std::stoi(num));
				vec.push_back(pUnit);
			}
			prevUnitType = pUnit->type();
			c = tokEnd;
			mayBeStart = false;
			continue;
		}

		// string of letters and digits. May be function or field identifier
		if (isLetter(*c)) {
			char *tokEnd = c+1;
			while (tokEnd<cEnd && (isLetter(*tokEnd) || isDigit(*tokEnd) || *tokEnd=='_')) tokEnd++;
			if (tokEnd == c+1) {
				pUnit = std::make_shared<AluUnitFieldIdentifier>(*c);
			} else if (nextNonBlankChar(tokEnd)=='(') {
				std::string identifier(c,(tokEnd-c));
				pUnit = std::make_shared<AluFunction>(identifier);
			} else {
				std::string lit(c,(tokEnd-c));
				pUnit = std::make_shared<AluUnitLiteral>(lit);
			}
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			mayBeStart = false;
			continue;
		}

		// parenthesis
		if (*c=='(') {
			pUnit = std::make_shared<AluOtherToken>(UT_OpenParenthesis);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c++;
			mayBeStart = true;
			continue;
		}

		if (*c==')') {
			pUnit = std::make_shared<AluOtherToken>(UT_ClosingParenthesis);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c++;
			mayBeStart = false;
			continue;
		}

		if (*c==',') {
			pUnit = std::make_shared<AluOtherToken>(UT_Comma);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c++;
			mayBeStart = true;
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
			mayBeStart = true;
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
			pUnit = std::make_shared<AluUnitLiteral>(sLiteral);
			vec.push_back(pUnit);
			prevUnitType = pUnit->type();
			c = tokEnd;
			mayBeStart = false;
			continue;
		}
	}
#ifdef ALU_DUMP
	if (g_bDebugAluCompile) dumpAluVec("Parsed Expression", vec);
#endif
	return true;
}

void cleanAluVec(AluVec& vec)
{
	while (!vec.empty()) {
		PUnit pUnit = vec[0];
		vec.erase(vec.begin());
		pUnit = nullptr;
	}
}

std::string dumpAluVec(AluVec& vec, bool deleteUnits)
{
	std::string ret;
	if (deleteUnits) {
		while (!vec.empty()) {
			PUnit pUnit = vec[0];
			if (!ret.empty()) ret += ";";
			ret += pUnit->_identify();
			vec.erase(vec.begin());
			pUnit = nullptr;
		}
	} else {
		for (size_t i=0; i<vec.size(); i++) {
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

	auto ctr = std::dynamic_pointer_cast<AluUnitCounter>(vec[0]);
	auto pAssnOp = std::dynamic_pointer_cast<AluAssnOperator>(vec[1]);
	*pAss = *pAssnOp;
	k = ctr->getKey();

	vec.erase(vec.begin());
	vec.erase(vec.begin());
	ctr = nullptr;
	pAssnOp = nullptr;

	// Check that we don't have something terrible like an assignment operator in the
	// expression. Like this is C or something
	for (PUnit pUnit : vec) {
		if (UT_AssignmentOp==pUnit->type()) {
			MYTHROW("ALU expression should not contain an assignment operator");
		}
	}
	return true;
}

bool isHigherPrecedenceBinaryOp(PUnit op1, PUnit op2)
{
	auto bop1 = std::dynamic_pointer_cast<AluBinaryOperator>(op1);
	auto bop2 = std::dynamic_pointer_cast<AluBinaryOperator>(op2);

	if (!bop1 || !bop2) {
		MYTHROW("Received two units. Not both are binary operators");
	}

	return bop1->priority() >= bop2->priority();
}

bool breakAluVecByComma(AluVec& source, AluVec& dest)
{
	int countOpens = 0;
	int countUnits = 0;
	bool foundComma = false;

	if (!dest.empty()){
		MYTHROW("Entered with non-empty destination vec.");
	}


	for (PUnit pUnit : source) {
		countUnits++;
		switch(pUnit->type()) {
		case UT_OpenParenthesis:
			countOpens++;
			break;
		case UT_ClosingParenthesis:
			MYASSERT(countOpens>0);
			countOpens--;
			break;
		case UT_Comma:
			if (0==countOpens) foundComma = true;
			break;
		default:
			break;
		}

		if (foundComma) {
			pUnit = nullptr;
			break;
		}
		else {
			dest.push_back(pUnit);
		}
	}

	for (int i=0 ; i < countUnits ; i++) {
		source.erase(source.begin());
	}

	MYASSERT(foundComma || source.empty());

	return true;
}

#define X(fn,argc,flags,rl) if (s==##fn) { argCount = argc; return; }
struct functionArgcountPair {
	functionArgcountPair(std::string& s, unsigned int argc, unsigned int avail) {
		funcName = s;
		argCount = argc;
		availableOperands = avail;
	}
	std::string  funcName;
	unsigned int argCount;
	unsigned int availableOperands;
};
#undef X

class functionNestingStack {
public:
	functionNestingStack()  {}
	~functionNestingStack() {
		// MYASSERT_WITH_MSG(m_stack.empty(), "Function nesting stack not empty");
	}
	void   push(std::string& s, unsigned int argc, unsigned int availableOperands) {
		m_stack.push(functionArgcountPair(s,argc,availableOperands));
	}
	void   pop() {
		m_stack.pop();
	}
	std::string fname() {
		return m_stack.top().funcName;
	}
	unsigned int argc() {
		return m_stack.top().argCount;
	}
	unsigned int availBefore() {
		return m_stack.top().availableOperands;
	}
private:
	std::stack<functionArgcountPair> m_stack;
};

bool isPseudoFunctionName(std::string& fn)
{
#define X(nm) if (#nm==fn) return true;
	ALU_PSEUDO_FUNCTION_LIST
	return false;
}

/*
 * Function: convertAluVecToPostfix
 * Implements the Shunting-Yard algorithm to convert an infix expression
 * to a postfix expression for easier calculation later on.
 *
 * This function also finds and fixes instances of the break() pseudo-function
 */
enum pseudoFunctionFindingState {
	PFFS_lookingForPseudoFunction,
	PFFS_lookingForOpen,
	PFFS_lookingForFI,
	PFFS_lookingForClose
};
bool convertAluVecToPostfix(AluVec& source, AluVec& dest, bool clearSource)
{
	std::stack<PUnit> operatorStack;
	unsigned int availableOperands = 0;

	if (!dest.empty()){
		MYTHROW("Entered with non-empty vec.");
	}

	// Handle the special case of pseudo-functions.
	// The parameter should be a field identifier and it is converted to a number
	// representing the ASCII value of the character
	pseudoFunctionFindingState pffs = PFFS_lookingForPseudoFunction;
	for (PUnit pUnit : source) {
		switch (pffs) {
		case PFFS_lookingForPseudoFunction: {
			if (pUnit->type() != UT_Identifier) continue;
			auto func = std::dynamic_pointer_cast<AluFunction>(pUnit);
			if (!isPseudoFunctionName(func->getName())) continue;
			pffs = PFFS_lookingForOpen;
			break;
		}
		case PFFS_lookingForOpen: {
			if (pUnit->type() != UT_OpenParenthesis) {
				pffs = PFFS_lookingForPseudoFunction;
				continue;
			}
			pffs = PFFS_lookingForFI;
			break;
		}
		case PFFS_lookingForFI: {
			if (pUnit->type() != UT_FieldIdentifier) {
				MYTHROW("Pseudo-functions' first argument may only be a field identifier");
			}
			auto pFI = std::dynamic_pointer_cast<AluUnitFieldIdentifier>(pUnit);
			pFI->setEvaluateToName();
			pffs = PFFS_lookingForClose;
			break;
		}
		case PFFS_lookingForClose: {
			if (pUnit->type() != UT_ClosingParenthesis) {
				continue;
			}
			pffs = PFFS_lookingForPseudoFunction;
			break;
		}
		}
	}

#ifdef ALU_DUMP
	if (g_bDebugAluCompile) {
		dumpAluVec("Expression to Convert to RPN", source);
		if (g_bVerbose) {
			clearSource = false;
		}
	}
#endif

	int stepNumber = 0;
	bool     bExpectNullArgument = false;
	functionNestingStack fstack;

	for (auto pUnit : source) {
#ifdef ALU_DUMP
		if (g_bDebugAluCompile && g_bVerbose) {
			std::cerr << "\n\n\nStep #" << stepNumber << " - available operands: " << availableOperands << std::endl;
			dumpAluVec("Source", source, stepNumber);
			dumpAluVec("Dest", dest);
			dumpAluStack("operator stack",operatorStack);
		}
#endif
		stepNumber++;
		switch (pUnit->type()) {
		case UT_Comma:
			while (!operatorStack.empty() && UT_OpenParenthesis!=operatorStack.top()->type()) {
				MYASSERT_WITH_MSG(operatorStack.top()->countOperands() <= availableOperands, "Not enough operands for operator (3)");
				availableOperands = availableOperands + 1 - operatorStack.top()->countOperands();
				dest.push_back(operatorStack.top());
				operatorStack.pop();
			}
			if (bExpectNullArgument) {
				dest.push_back(std::make_shared<AluUnitNull>());
				availableOperands++;
			}
			bExpectNullArgument = true;
			break;
		case UT_AssignmentOp:
			MYTHROW("Assignment operator used in expression");
			break;
		case UT_None:
		case UT_Invalid:
		case UT_Null:
			MYTHROW("None or Invalid - internal logic error");
		case UT_LiteralNumber:
		case UT_Counter:
		case UT_FieldIdentifier:
		case UT_InputRecord:
			dest.push_back(pUnit);
			availableOperands++;
			bExpectNullArgument = false;
			break;
		case UT_Identifier: {	// a function
			auto pFunc = std::dynamic_pointer_cast<AluFunction>(pUnit);
			operatorStack.push(pUnit);
			fstack.push(pFunc->getName(), pFunc->countOperands(), availableOperands);
			bExpectNullArgument = false;
			break;
		}
		case UT_UnaryOp: { // Unary operator - higher precedence than any binary but not a function
			AluUnitType topType = operatorStack.empty() ? UT_None : operatorStack.top()->type();
			while (topType==UT_Identifier) {
				MYASSERT_WITH_MSG(operatorStack.top()->countOperands() <= availableOperands, "Not enough operands for operator (1)");
				availableOperands = availableOperands + 1 - operatorStack.top()->countOperands();
				dest.push_back(operatorStack.top());
				operatorStack.pop();
				topType = operatorStack.empty() ? UT_None : operatorStack.top()->type();
			}
			operatorStack.push(pUnit);
			bExpectNullArgument = false;
			break;
		}
		case UT_BinaryOp: { // Binary operator - lower than unary and an interesting rank among them
			AluUnitType topType = operatorStack.empty() ? UT_None : operatorStack.top()->type();
			while (topType==UT_Identifier || topType==UT_UnaryOp ||
					(topType==UT_BinaryOp && isHigherPrecedenceBinaryOp(operatorStack.top(),pUnit))) {
				MYASSERT_WITH_MSG(operatorStack.top()->countOperands() <= availableOperands, "Not enough operands for operator (2)");
				availableOperands = availableOperands + 1 - operatorStack.top()->countOperands();
				dest.push_back(operatorStack.top());
				operatorStack.pop();
				topType = operatorStack.empty() ? UT_None : operatorStack.top()->type();
			}
			operatorStack.push(pUnit);
			bExpectNullArgument = false;
			break;
		}
		case UT_OpenParenthesis:
			operatorStack.push(pUnit);
			bExpectNullArgument = true;
			break;
		case UT_ClosingParenthesis:
			while (!operatorStack.empty() && UT_OpenParenthesis!=operatorStack.top()->type()) {
				MYASSERT_WITH_MSG(operatorStack.top()->countOperands() <= availableOperands, "Not enough operands for operator (3)");
				availableOperands = availableOperands + 1 - operatorStack.top()->countOperands();
				dest.push_back(operatorStack.top());
				operatorStack.pop();
			}
			if (operatorStack.empty()) {
				MYTHROW("Mismatched parenthesis -- too many closing");
			} else {
				operatorStack.pop(); // Pop off the opening parenthesis
				// issue #37 - if what remains is a function, it should also go
				if (!operatorStack.empty() && UT_Identifier==operatorStack.top()->type()) {
					if ((availableOperands) > (fstack.availBefore() + fstack.argc())) {
						std::string err = "Too many operands (" +
								std::to_string(availableOperands - fstack.availBefore()) +
								") for function " + fstack.fname() + " (accepts " +
								std::to_string(fstack.argc()) + ")";
						MYTHROW(err);
					}
					while (availableOperands < (fstack.availBefore() + fstack.argc())) {
						dest.push_back(std::make_shared<AluUnitNull>());
						availableOperands++;
					}
					availableOperands = availableOperands + 1 - operatorStack.top()->countOperands();
					dest.push_back(operatorStack.top());
					operatorStack.pop();
					fstack.pop();
				}
			}
			bExpectNullArgument = false;
		}
	}

	while (!operatorStack.empty()) {
		if (operatorStack.top()->type()==UT_OpenParenthesis) {
			MYTHROW("Mismatched parenthesis -- too many opening");
		}
		PUnit pTopUnit = operatorStack.top();
		operatorStack.pop();
		MYASSERT_WITH_MSG(pTopUnit->countOperands()<=dest.size(), "Not enough operands for operator (4)");
		dest.push_back(pTopUnit);
	}

	if (clearSource) {
		while (!source.empty()) {
			source.erase(source.begin());
		}
	}

#ifdef ALU_DUMP
	if (g_bDebugAluCompile) dumpAluVec("RPN Expression", dest);
#endif

	return true;
}

PValue evaluateExpression(AluVec& expr, ALUCounters* pctrs)
{
	std::stack<PValue> computeStack;
	PValue arg1;
	PValue arg2;
	PValue arg3;
	PValue arg4;
	PValue arg5;

#ifdef ALU_DUMP
	if (g_bDebugAluRun) {
		std::cerr << "\n============= " << __FUNCTION__ << " ==============\n";
	}
	int index = 0;
#endif

	for (PUnit pUnit : expr) {
#ifdef ALU_DUMP
		if (g_bDebugAluRun) {
			dumpAluVec("Expression Progress", expr, index);
			dumpAluStack("Execution Stack", computeStack);
			std::cerr << std::endl << std::endl;
		}
		index++;
#endif
		switch (pUnit->type()) {
		case UT_LiteralNumber:
		case UT_FieldIdentifier:
		case UT_InputRecord:
		case UT_Null:
			computeStack.push(pUnit->evaluate());
			break;
		case UT_Counter: {
			auto pCtr = std::dynamic_pointer_cast<AluUnitCounter>(pUnit);
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
			MYASSERT(pUnit->countOperands() <= MAX_FUNC_OPERANDS);

			switch (pUnit->countOperands()) {
			case 5:
				arg5 = computeStack.top();
				computeStack.pop();
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
				try {
					computeStack.push(pUnit->compute(arg1));
				}
				catch (const SpecsException& e) {
					throw;
				}
				break;
			case 2:
				try {
					computeStack.push(pUnit->compute(arg1, arg2));
				}
				catch (const SpecsException& e) {
					throw;
				}
				break;
			case 3:
				try {
					computeStack.push(pUnit->compute(arg1, arg2, arg3));
				}
				catch (const SpecsException& e) {
					throw;
				}
				break;
			case 4:
				try {
					computeStack.push(pUnit->compute(arg1, arg2, arg3, arg4));
				}
				catch (const SpecsException& e) {
					throw;
				}
				break;
			case 5:
				try {
					computeStack.push(pUnit->compute(arg1, arg2, arg3, arg4, arg5));
				}
				catch (const SpecsException& e) {
					throw;
				}
				break;
			default:
				break;
			}
			break;
			default:
				std::string err = "Logic error, should not have gotten this type of Unit: " + pUnit->_identify();
				MYTHROW(err);
			}
		}
	}

	MYASSERT(computeStack.size() == 1);
#ifdef ALU_DUMP
		if (g_bDebugAluRun) {
			dumpAluStack("Final Stack", computeStack);
		}
#endif
	PValue ret = computeStack.top();
	computeStack.pop();
	return ret;
}

void ALUPerformAssignment(ALUCounterKey& k, POperator pAss, AluVec& expr, ALUCounters* pctrs)
{
	PValue exprResult = evaluateExpression(expr, pctrs);

	pAss->perform(k, pctrs, exprResult);
}

bool AluExpressionReadsLines(AluVec& vec)
{
	for (PUnit unit : vec) {
		if (unit->requiresRead()) {
			return true;
		}
	}
	return false;
}

bool expressionForcesRunoutCycle(AluVec& vec)
{
	for (PUnit unit : vec) {
		switch (unit->type()) {
		case UT_Identifier:
		{
			auto pFunction = std::dynamic_pointer_cast<AluFunction>(unit);
			MYASSERT(nullptr!=pFunction);
			return ("eof" == pFunction->getName());
		}
		default:
			break;
		}
	}
	return false;
}

void AluValueStats::initialize()
{
	m_intCount = 0;
	m_floatCount = 0;
	m_totalCount = 0;
	m_sumInt = 0;
	m_sumFloat = 0.0;
	m_minInt = 0;
	m_minFloat = 0.0;
	m_maxInt = 0;
	m_maxFloat = 0.0;
	m_runningAverage = 0.0;
	m_runningSn = 0.0;
}

AluValueStats::AluValueStats()
{
	initialize();
}

AluValueStats::AluValueStats(char id)
{
	initialize();
	AddValue(id);
}

void AluValueStats::AddValue(char id)
{
	ALUValue v(g_fieldIdentifierGetter->Get(id));
	auto type = v.getDivinedType();
	switch (type) {
	case counterType__Int:
	{
		ALUInt value = v.getInt();
		m_intCount++;
		if (1 == m_intCount) {
			m_sumInt = value;
			m_minInt = value;
			m_maxInt = value;
		} else {
			if (value > m_maxInt) m_maxInt = value;
			if (value < m_minInt) m_minInt = value;
			m_sumInt += value;
		}
		/* intentional fall-through */
	}
	case counterType__Float:
	{
		ALUFloat value = v.getFloat();

		// Collect running average and variance
		if (0 == m_totalCount++) {
			m_runningAverage = value;
			m_runningSn = 0.0;
		} else {
			ALUFloat diffFromPreviousAverage = value - m_runningAverage;
			m_runningAverage += (diffFromPreviousAverage / m_totalCount);
			m_runningSn += (diffFromPreviousAverage * (value - m_runningAverage));
		}

		if (type != counterType__Float) break;
		m_floatCount++;
		if (1 == m_floatCount) {
			m_sumFloat = value;
			m_minFloat = value;
			m_maxFloat = value;
		} else {
			if (value > m_maxFloat) m_maxFloat = value;
			if (value < m_minFloat) m_minFloat = value;
			m_sumFloat += value;
		}
		break;
	}
	default:
		break;
	}
}


PValue AluValueStats::sum()
{
	if (0 < m_floatCount) {
		return mkValue(m_sumFloat + m_sumInt);
	} else {
		return mkValue(m_sumInt);
	}
}

PValue AluValueStats::sumi()
{
	return mkValue(m_sumInt);
}

PValue AluValueStats::sumf()
{
	return mkValue(m_sumFloat);
}

PValue AluValueStats::_min()
{
	if (0 < m_floatCount) {
		if (0 < m_intCount && m_minInt < m_minFloat) {
			return mkValue(ALUFloat(m_minInt));
		} else {
			return mkValue(m_minFloat);
		}
	} else {
		return mini();
	}
}

PValue AluValueStats::mini()
{
	if (0 < m_intCount) {
		return mkValue(m_minInt);
	} else {
		return mkValue0(); /* returns NaN */
	}
}

PValue AluValueStats::minf()
{
	if (0 < m_floatCount) {
		return mkValue(m_minFloat);
	} else {
		return mkValue0(); /* returns NaN */
	}
}

PValue AluValueStats::_max()
{
	if (0 < m_floatCount) {
		if (0 < m_intCount && m_maxInt < m_maxFloat) {
			return mkValue(ALUFloat(m_maxInt));
		} else {
			return mkValue(m_maxFloat);
		}
	} else {
		return maxi();
	}
}

PValue AluValueStats::maxi()
{
	if (0 < m_intCount) {
		return mkValue(m_maxInt);
	} else {
		return mkValue0(); /* returns NaN */
	}
}

PValue AluValueStats::maxf()
{
	if (0 < m_floatCount) {
		return mkValue(m_maxFloat);
	} else {
		return mkValue0(); /* returns NaN */
	}
}

PValue AluValueStats::average()
{
	if (m_totalCount==0) {
		return mkValue0(); /* returns NaN */
	}

	return mkValue(m_runningAverage);
}

PValue AluValueStats::variance()
{
	if (m_totalCount==0) {
		return mkValue0(); /* returns NaN */
	}

	return mkValue(m_runningSn / m_totalCount);
}

PValue AluValueStats::stddev()
{
	if (m_totalCount==0) {
		return mkValue0(); /* returns NaN */
	}

	return mkValue(std::sqrt(m_runningSn / m_totalCount));
}

PValue AluValueStats::stderrmean()
{
	if (m_totalCount<=1) {
		return mkValue0(); /* returns NaN */
	}

	return mkValue(std::sqrt(m_runningSn / m_totalCount) / (m_totalCount-1));
}

std::ostream& operator<< (std::ostream& os, const ALUValue &c)
{
	os << c.getStr();
    return os;
}

std::ostream& operator<< (std::ostream& os, const AluUnit &u)
{
    u._serialize(os);

    return os;
}

