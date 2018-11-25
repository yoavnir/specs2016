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

class ALUCounter {
public:
	ALUCounter():m_type(counterType__None), m_value("") {}
	ALUCounter(std::string& s) {set(s);}
	ALUCounter(ALUInt i)       {set(i);}
	ALUCounter(ALUFloat f)     {set(f);}
	ALUCounterType getType()   {return m_type;}
	std::string    getStr()    {return m_value;}
	ALUInt         getInt();
	ALUInt         getHex();
	ALUFloat       getFloat();
	void           set(std::string& s);
	void           set(const char* st);
	void           set(ALUInt i);
	void           set(ALUFloat f);
	bool           isWholeNumber();
private:
	std::string    m_value;
	ALUCounterType m_type;
};

class ALUCounters {
public:
	std::string    getStr(unsigned int i)   {return m_map[i].getStr();}
	ALUInt         getInt(unsigned int i)   {return m_map[i].getInt();}
	ALUInt         getHex(unsigned int i)   {return m_map[i].getHex();}
	ALUFloat       getFloat(unsigned int i) {return m_map[i].getFloat();}
	void           set(unsigned int i, std::string& s)  {m_map[i].set(s);}
	void           set(unsigned int i, const char* st)  {m_map[i].set(st);}
	void           set(unsigned int i, ALUInt l)        {m_map[i].set(l);}
	void           set(unsigned int i, ALUFloat f)      {m_map[i].set(f);}
	ALUCounterType type(unsigned int i)     {return m_map[i].getType();}
	bool           isWholeNumber(unsigned int i) {return m_map[i].isWholeNumber();}
private:
	std::map<unsigned int, ALUCounter> m_map;
};

#endif
