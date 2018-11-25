/*
 * The Arithmetic / Logic Unit
 * ===========================
 *
 * This contains all the code needed for storing variables, parsing expressions,
 * and evaluating expressions. These are needed to implement the PRINT and SET
 * directives of specs.
 */

#include <sstream>
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

ALUInt ALUCounter::getInt()
{
	return (counterType__None==m_type) ? 0 : std::stoll(m_value);
}

ALUInt ALUCounter::getHex()
{
	return (counterType__None==m_type) ? 0 : std::stoll(m_value, NULL, 16);
}

ALUFloat ALUCounter::getFloat()
{
	return (counterType__None==m_type) ? 0.0 : std::stold(m_value);
}

bool ALUCounter::isWholeNumber()
{
	return (ALUFloat(getInt())==getFloat());
}

