#ifndef SPECS2016__UTILS__ALU__H
#define SPECS2016__UTILS__ALU__H

#include <map>
#include <string>

typedef long long int ALUInt;
typedef long double   ALUFloat;
#define ALUFloatPrecision 16

enum ALUCounterType {
	counterType__None,
	counterType__Str,
	counterType__Int,
	counterType__Float
};

class ALUCounters {
public:
	std::string    getStr(unsigned int i);
	ALUInt         getInt(unsigned int i);
	ALUInt         getHex(unsigned int i);
	ALUFloat       getFloat(unsigned int i);
	void           set(unsigned int i, std::string& s);
	void           set(unsigned int i, const char* st);
	void           set(unsigned int i, ALUInt l);
	void           set(unsigned int i, ALUFloat f);
	ALUCounterType type(unsigned int i);
private:
	std::map<unsigned int, std::string> m_counters;
	std::map<unsigned int, ALUCounterType> m_types;
};

#endif
