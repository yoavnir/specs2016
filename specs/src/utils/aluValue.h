#ifndef SPECS2016__UTILS__ALU_VALUE__H
#define SPECS2016__UTILS__ALU_VALUE__H

#include <string>
#include <memory>

typedef long long int ALUInt;
typedef long double   ALUFloat;

#define MAX_ALUInt LLONG_MAX

#define ALUInt_SZ sizeof(ALUInt)

enum ALUCounterType {
	counterType__None,
	counterType__Str,
	counterType__Int,
	counterType__Float
};

static std::string ALUCounterType2Str[] = {
		"NaN",
		"Str",
		"Int",
		"Float"
};


class ALUValue {
public:
	ALUValue():m_value(""), m_type(counterType__None) {}
	ALUValue(std::string& s) {set(s);}
	ALUValue(const std::string& s) {set(s);}
	ALUValue(ALUInt i)       {set(i);}
	ALUValue(ALUFloat f)     {set(f);}
	ALUValue(const char* c, int bytes) {
		std::string s(c,bytes);
		set(s);
	}
	ALUCounterType getType()   {return m_type;}
	ALUCounterType getDivinedType() const;
	void           divineType()		{m_type = getDivinedType();}
	std::string    getStr()  const  {return (m_type==counterType__None) ? std::string("NaN") : m_value;}
	std::string*   getStrPtr()      {return (m_type==counterType__None) ? NULL : &m_value;}
	ALUInt         getInt() const;
	ALUInt         getHex() const;
	ALUFloat       getFloat() const;
	bool           getBool() const;
	void           set(std::string& s);
	void           set(const std::string& s);
	void           set(const char* st);
	void           set(ALUInt i);
	void           set(ALUFloat f);
	void           set();
	bool           isWholeNumber() const;
	bool           isFloat() const;
	bool           isNumeric() const;
private:
	std::string    m_value;
	ALUCounterType m_type;
};

typedef std::shared_ptr<ALUValue> PValue;

#define mkValue0()     PValue(new ALUValue())
#define mkValue(x)     PValue(new ALUValue(x))
#define mkValue2(x,y)  PValue(new ALUValue(x,y))

#endif
