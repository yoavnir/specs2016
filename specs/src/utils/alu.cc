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

unsigned int getCountOperands(AluUnitType t)
{
	switch (t) {
	case UT_LiteralNumber:
	case UT_Counter:
	case UT_FieldIdentifier:
		return 0;
	case UT_UnaryOp:
	case UT_AssignmentOp:
		return 1;
	case UT_BinaryOp:
	case UT_RelOp:
		return 2;
	default:
	{
		std::string err = "Invalid unit type: " + std::to_string(t);
		MYTHROW(err);
	}
	}
}

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
	switch (m_op) {
	ALU_UOP_LIST
	default:
		MYTHROW("Invalid unary operand");
	}
}
#undef X

ALUCounter* 	AluUnitUnaryOperator::computeNot(ALUCounter* operand)
{
	if (true==operand->getBool()) {
		return new ALUCounter(ALUInt(0));
	} else {
		return new ALUCounter(ALUInt(1));
	}
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
