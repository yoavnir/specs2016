#ifndef SPECS2016__UTILS__ALU__H
#define SPECS2016__UTILS__ALU__H

#include <map>
#include <vector>
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

static std::string ALUCounterType2Str[] = {
		"NaN",
		"Str",
		"Int",
		"Float"
};

class ALUCounter {
public:
	ALUCounter():m_type(counterType__None), m_value("") {}
	ALUCounter(std::string& s) {set(s);}
	ALUCounter(ALUInt i)       {set(i);}
	ALUCounter(ALUFloat f)     {set(f);}
	ALUCounterType getType()   {return m_type;}
	ALUCounterType getDivinedType() const;
	void           divineType()		{m_type = getDivinedType();}
	std::string    getStr()  const  {return m_value;}
	ALUInt         getInt() const;
	ALUInt         getHex() const;
	ALUFloat       getFloat() const;
	bool           getBool() const;
	void           set(std::string& s);
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

static std::ostream& operator<< (std::ostream& os, const ALUCounter &c)
{
	os << c.getStr();
    return os;
}


typedef unsigned int ALUCounterKey;

class ALUCounters {
public:
	std::string		getStr(ALUCounterKey i)		{return m_map[i].getStr();}
	ALUInt			getInt(ALUCounterKey i)		{return m_map[i].getInt();}
	ALUInt			getHex(ALUCounterKey i)		{return m_map[i].getHex();}
	ALUFloat		getFloat(ALUCounterKey i)		{return m_map[i].getFloat();}
	bool			getBool(ALUCounterKey i)		{return m_map[i].getBool();}
	ALUCounter*		getPointer(ALUCounterKey i)	{return &m_map[i];}
	void			set(ALUCounterKey i, std::string& s)  {m_map[i].set(s);}
	void			set(ALUCounterKey i, const char* st)  {m_map[i].set(st);}
	void			set(ALUCounterKey i, ALUInt l)        {m_map[i].set(l);}
	void			set(ALUCounterKey i, ALUFloat f)      {m_map[i].set(f);}
	void			set(ALUCounterKey i)				  {m_map[i].set();}
	ALUCounterType	type(ALUCounterKey i)     {return m_map[i].getType();}
	ALUCounterType	divinedType(ALUCounterKey i)     {return m_map[i].getDivinedType();}
	bool         	isWholeNumber(ALUCounterKey i) {return m_map[i].isWholeNumber();}
	bool          	isNumeric(ALUCounterKey i) {return m_map[i].isNumeric();}
private:
	std::map<ALUCounterKey, ALUCounter> m_map;
};

#define ALU_UOP_LIST	\
	X(Plus,"+")		\
	X(Minus,"-")	\
	X(Not,"!")		\

#define ALU_BOP_LIST	\
	X(Add,    "+",   8)	\
	X(Sub,    "-",   8)	\
	X(Mult,   "*",   9)	\
	X(Div,    "/",   9) \
	X(IntDiv, "//",  9)	\
	X(RemDiv, "%",   9)	\
	X(Appnd,  "||",  7)	\
	X(LT,     "<",   6)	\
	X(LE,     "<=",  6)	\
	X(GT,     ">",   6)	\
	X(GE,     ">=",  6)	\
	X(SLT,    "<<",  6)	\
	X(SLTE,   "<<=", 6)	\
	X(SGT,    ">>",  6)	\
	X(SGTE,   ">>=", 6)	\
	X(EQ,     "=",   5)	\
	X(SEQ,    "==",  5)	\
	X(NE,     "!=",  5)	\
	X(SNE,    "!==", 5)	\
	X(AND,    "&",   4)	\
	X(OR,     "|",   3)	\

#define ALU_ASSOP_LIST \
	X(Let,":=")		\
	X(Add,"+=")		\
	X(Sub,"-=")		\
	X(Mult,"*=")	\
	X(Div,"/=")		\
	X(IntDiv,"%=")	\
	X(RemDiv,"//=")	\
	X(Appnd,"||=")	\


enum AluUnitType {
	UT_Invalid,
	UT_None,
	UT_OpenParenthesis,
	UT_ClosingParenthesis,
	UT_Comma,
	UT_Identifier,
	UT_LiteralNumber,
	UT_Counter,
	UT_FieldIdentifier,
	UT_UnaryOp,
	UT_BinaryOp,
	UT_AssignmentOp,
};

unsigned int getCountOperands(AluUnitType t);

// This abstract class represents anything in an expression - a number, a counter,
// a field identifier, an operation
class AluUnit {
public:
	AluUnit()			{}
	virtual ~AluUnit()	{}
	virtual unsigned int    countOperands()		{return 0;}
	virtual void   			_serialize(std::ostream& os) const = 0;
	virtual std::string     _identify() = 0;
	virtual AluUnitType		type()	{return UT_Invalid;}
	virtual ALUCounter*		evaluate();
	virtual ALUCounter*		compute(ALUCounter* op);
	virtual ALUCounter*		compute(ALUCounter* op1, ALUCounter* op2);
	virtual ALUCounter*		compute(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3);
	virtual ALUCounter*		compute(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3, ALUCounter* op4);
};

class AluUnitLiteral : public AluUnit {
public:
	AluUnitLiteral(std::string& s, bool hintNumerical=false):m_literal(s),m_hintNumerical(hintNumerical)	{}
	virtual ~AluUnitLiteral()					{}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify();
	virtual AluUnitType			type()			{return UT_LiteralNumber;}
	virtual ALUCounter*			evaluate();
private:
	ALUCounter	m_literal;
	bool        m_hintNumerical;
};

class AluUnitCounter : public AluUnit {
public:
	AluUnitCounter(ALUCounterKey ctrNumber):m_ctrNumber(ctrNumber) {};
	~AluUnitCounter()			{}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify();
	virtual AluUnitType			type()			{return UT_Counter;}
	virtual ALUCounter*			compute(ALUCounters* pCtrs);
	ALUCounterKey				getKey()		{return m_ctrNumber;}
private:
	ALUCounterKey m_ctrNumber;
};

class fieldIdentifierGetter {
public:
	virtual ~fieldIdentifierGetter() {}
	virtual std::string Get(char id) = 0;
};

void setFieldIdentifierGetter(fieldIdentifierGetter* getter);

class AluUnitFieldIdentifier : public AluUnit {
public:
	AluUnitFieldIdentifier(char _fId):m_id(_fId) {};
	~AluUnitFieldIdentifier()			{}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify();
	virtual AluUnitType			type()			{return UT_FieldIdentifier;}
	virtual ALUCounter*			evaluate();
private:
	char         m_id;
};

#define X(nm,str) UnaryOp__##nm,
enum ALU_UnaryOperator {
	ALU_UOP_LIST
};
#undef X

#define X(nm,str) ALUCounter* compute##nm(ALUCounter* operand);
class AluUnitUnaryOperator : public AluUnit {
public:
	AluUnitUnaryOperator(std::string& s);
	AluUnitUnaryOperator(const char* str);
	virtual ~AluUnitUnaryOperator()			{}
	virtual unsigned int	countOperands()		{return 1;}
	virtual void			_serialize(std::ostream& os) const;
	virtual std::string		_identify();
	virtual AluUnitType		type()			{return UT_UnaryOp;}
	virtual ALUCounter*		compute(ALUCounter* operand);
private:
	void               setOpByName(std::string& s);
	ALU_UOP_LIST
	ALU_UnaryOperator  m_op;
};
#undef X

#define X(nm,str,prio) BinaryOp__##nm,
enum ALU_BinaryOperator {
	ALU_BOP_LIST
};
#undef X

#define X(nm,str,prio) ALUCounter* compute##nm(ALUCounter* op1, ALUCounter* op2);
class AluBinaryOperator : public AluUnit {
public:
	AluBinaryOperator(std::string& s);
	AluBinaryOperator(const char* str);
	virtual ~AluBinaryOperator()			{}
	virtual unsigned int	countOperands()		{return 2;}
	virtual void			_serialize(std::ostream& os) const;
	virtual std::string		_identify();
	virtual AluUnitType		type()			{return UT_BinaryOp;}
	virtual ALUCounter*		compute(ALUCounter* op1, ALUCounter* op2);
	unsigned int			priority()	{return m_priority;}
private:
	void				setOpByName(std::string& s);
	unsigned int		m_priority;
	ALU_BOP_LIST
	ALU_BinaryOperator  m_op;
};
#undef X

#define X(nm,str) AssnOp__##nm,
enum ALU_AssignmentOperator {
	ALU_ASSOP_LIST
};
#undef X

#define X(nm,str) ALUCounter* compute##nm(ALUCounter* operand, ALUCounter* prevValue);
class AluAssnOperator : public AluUnit {
public:
	AluAssnOperator()				{m_op = AssnOp__Let;}
	AluAssnOperator(std::string& s);
	AluAssnOperator(const char* str);
	virtual ~AluAssnOperator()			{}
	virtual unsigned int		countOperands()	{return 1;}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string 		_identify();
	virtual AluUnitType			type()			{return UT_AssignmentOp;}
	void	perform(ALUCounterKey ctrNumber, ALUCounters* ctrs, ALUCounter* operand);
private:
	void               setOpByName(std::string& s);
	ALU_ASSOP_LIST
	ALU_AssignmentOperator	m_op;
};
#undef X

class AluFunction : public AluUnit {
public:
	AluFunction(std::string& s);
	virtual ~AluFunction()		{}
	virtual unsigned int		countOperands()		{return m_ArgCount;}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify()	{return "FUNC("+m_FuncName+")";}
	virtual AluUnitType			type()		{return UT_Identifier;}
	virtual ALUCounter*			evaluate();
	virtual ALUCounter*			compute(ALUCounter* op);
	virtual ALUCounter*			compute(ALUCounter* op1, ALUCounter* op2);
	virtual ALUCounter*			compute(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3);
	virtual ALUCounter*			compute(ALUCounter* op1, ALUCounter* op2, ALUCounter* op3, ALUCounter* op4);
private:
	std::string		m_FuncName;
	void*			mp_Func;
	unsigned int	m_ArgCount;
};

class AluOtherToken : public AluUnit {
public:
	AluOtherToken(AluUnitType _t)		{m_type = _t;}
	virtual ~AluOtherToken()			{}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string 		_identify();
	virtual AluUnitType			type()	{return m_type;}
private:
	AluUnitType		m_type;
};


static std::ostream& operator<< (std::ostream& os, const AluUnit &u)
{
    u._serialize(os);

    return os;
}


typedef std::vector<AluUnit*> AluVec;

bool parseAluExpression(std::string& s, AluVec& vec);

bool parseAluStatement(std::string& s, ALUCounterKey& k, AluAssnOperator* pAss, AluVec& vec);

std::string dumpAluVec(AluVec& vec, bool deleteUnits);

bool isValidExpression(AluVec& vec);  // To Be Added

bool convertAluVecToPostfix(AluVec& source, AluVec& dest, bool clearSource);

ALUCounter* evaluateExpression(AluVec& expr, ALUCounters* pctrs);

#endif
