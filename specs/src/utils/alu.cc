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

void ALUCounters::set(unsigned int i, std::string& s)
{
	m_counters[i] = s;
	m_types[i] = counterType__Str;
}

void ALUCounters::set(unsigned int i, const char* st)
{
	std::string s(st);
	set(i,s);
}

void ALUCounters::set(unsigned int i, ALUInt l)
{
	m_counters[i] = std::to_string(l);
	m_types[i] = counterType__Int;
}

void ALUCounters::set(unsigned int i, ALUFloat f)
{
    std::ostringstream ost;
    ost.precision(ALUFloatPrecision);
    ost << f;
	m_counters[i] = ost.str();
	m_types[i] = counterType__Float;
}

ALUCounterType ALUCounters::type(unsigned int i)
{
	return m_types[i];
}

std::string ALUCounters::getStr(unsigned int i)
{
	return (counterType__None==m_types[i]) ? "" : m_counters[i];
}

ALUInt ALUCounters::getInt(unsigned int i)
{
	return (counterType__None==m_types[i]) ? 0 : std::stoll(m_counters[i]);
}

ALUInt ALUCounters::getHex(unsigned int i)
{
	return (counterType__None==m_types[i]) ? 0 : std::stoll(m_counters[i], NULL, 16);
}

ALUFloat ALUCounters::getFloat(unsigned int i)
{
	return (counterType__None==m_types[i]) ? 0.0 : std::stold(m_counters[i]);
}

