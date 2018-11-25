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
	return (m_value=="1");
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
