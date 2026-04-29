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
	ALUValue():m_value(""), m_type(counterType__None), m_exact(false) {}
	ALUValue(std::string& s) {set(s);}
	ALUValue(const std::string& s) {set(s);}
	ALUValue(ALUInt i)       {set(i);}
	ALUValue(ALUInt i, bool exact)       {set(i); m_exact = exact;}
	ALUValue(ALUFloat f)     {set(f);}
	ALUValue(ALUFloat f, bool exact)     {set(f); m_exact = exact;}
	ALUValue(const char* c, int bytes) {
		std::string s(c,bytes);
		set(s);
	}
	ALUCounterType getType()   {return m_type;}
	ALUCounterType getDivinedType() const;
	void           divineType()		{m_type = getDivinedType();}
	std::string    getStr()  const  {return (m_type==counterType__None) ? std::string("NaN") : m_value;}
	std::string*   getStrPtr()      {return (m_type==counterType__None) ? nullptr : &m_value;}
	std::shared_ptr<std::string> getStrSptr()     {return std::make_shared<std::string>(getStr()); }
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
	bool           isExact() const    {return m_exact;}
	void           setExact(bool e)   {m_exact = e;}
private:
	std::string    m_value;
	ALUCounterType m_type;
	bool           m_exact = true;
};

typedef std::shared_ptr<ALUValue> PValue;

#define mkValue0()     std::make_shared<ALUValue>()
#define mkValue(x)     std::make_shared<ALUValue>(x)
#define mkValueE(x,e)  std::make_shared<ALUValue>(x,e)
#define mkValue2(x,y)  std::make_shared<ALUValue>(x,y)

#define IS_EXACT_VALUE(op, v) ((op)->isExact() \
	&& ((counterType__Int==(op)->getType() && (v)==(op)->getInt()) \
	 || ((op)->isWholeNumber() && (v)==(op)->getInt())))

#define IS_EXACT_ZERO(op) IS_EXACT_VALUE(op, 0)

#endif
