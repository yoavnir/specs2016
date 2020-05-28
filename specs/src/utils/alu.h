#ifndef SPECS2016__UTILS__ALU__H
#define SPECS2016__UTILS__ALU__H

#include <map>
#include <vector>
#include <climits>
#include <string>
#include <memory>
#include "utils/platform.h"
#include "utils/PythonIntf.h"
#include "utils/aluValue.h"

std::ostream& operator<< (std::ostream& os, const ALUValue &c);

typedef unsigned int ALUCounterKey;

class ALUCounters {
public:
	std::string		getStr(ALUCounterKey i)		{return m_map[i].getStr();}
	ALUInt			getInt(ALUCounterKey i)		{return m_map[i].getInt();}
	ALUInt			getHex(ALUCounterKey i)		{return m_map[i].getHex();}
	ALUFloat		getFloat(ALUCounterKey i)		{return m_map[i].getFloat();}
	bool			getBool(ALUCounterKey i)		{return m_map[i].getBool();}
	PValue		getPointer(ALUCounterKey i)	{
		if (0 == m_map.count(i)) m_map[i].set(ALUInt(0));
		return mkValue(m_map[i]);
	}
	void			set(ALUCounterKey i, std::string& s)  {m_map[i].set(s);}
	void			set(ALUCounterKey i, const char* st)  {m_map[i].set(st);}
	void			set(ALUCounterKey i, ALUInt l)        {m_map[i].set(l);}
	void			set(ALUCounterKey i, ALUFloat f)      {m_map[i].set(f);}
	void			set(ALUCounterKey i)				  {m_map[i].set();}
	ALUCounterType	type(ALUCounterKey i)     {return m_map[i].getType();}
	ALUCounterType	divinedType(ALUCounterKey i)     {return m_map[i].getDivinedType();}
	bool         	isWholeNumber(ALUCounterKey i) {return m_map[i].isWholeNumber();}
	bool          	isNumeric(ALUCounterKey i) {return m_map[i].isNumeric();}
	void            clearAll() { m_map.clear(); }
private:
	std::map<ALUCounterKey, ALUValue> m_map;
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
	X(RemDiv,"%=")	\
	X(IntDiv,"//=")	\
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
	UT_InputRecord,
	UT_Null,
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
	virtual PValue		evaluate();
	virtual PValue		compute(PValue op);
	virtual PValue		compute(PValue op1, PValue op2);
	virtual PValue		compute(PValue op1, PValue op2, PValue op3);
	virtual PValue		compute(PValue op1, PValue op2, PValue op3, PValue op4);
	virtual PValue		compute(PValue op1, PValue op2, PValue op3, PValue op4, PValue op5);
	virtual bool            requiresRead() {return false;} // true if this unit requires lines to be read
};

typedef std::shared_ptr<AluUnit> PUnit;

class AluUnitLiteral : public AluUnit {
public:
	AluUnitLiteral(std::string& s, bool hintNumerical=false):m_literal(s),m_hintNumerical(hintNumerical)	{}
	virtual ~AluUnitLiteral()					{}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify();
	virtual AluUnitType			type()			{return UT_LiteralNumber;}
	virtual PValue			evaluate();
private:
	ALUValue	m_literal;
	bool        m_hintNumerical;
};

class AluUnitNull : public AluUnit {
public:
	AluUnitNull() {}
	~AluUnitNull()             {}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify();
	virtual AluUnitType			type()			{return UT_Null;}
	virtual PValue			evaluate();
};

class AluUnitCounter : public AluUnit {
public:
	AluUnitCounter(ALUCounterKey ctrNumber):m_ctrNumber(ctrNumber) {};
	~AluUnitCounter()			{}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify();
	virtual AluUnitType			type()			{return UT_Counter;}
	using AluUnit::compute;  // prevent a warning about overloading
	virtual PValue			compute(ALUCounters* pCtrs);
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
	AluUnitFieldIdentifier(char _fId):m_id(_fId),m_ReturnIdentifier(false) {};
	~AluUnitFieldIdentifier()			{}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify();
	virtual AluUnitType			type()			{return UT_FieldIdentifier;}
	virtual PValue			evaluate();
	void                        setEvaluateToName()  {m_ReturnIdentifier = true;}
private:
	char         m_id;
	bool         m_ReturnIdentifier;
};

#define X(nm,str) UnaryOp__##nm,
enum ALU_UnaryOperator {
	ALU_UOP_LIST
};
#undef X

#define X(nm,str) PValue compute##nm(PValue operand);
class AluUnitUnaryOperator : public AluUnit {
public:
	AluUnitUnaryOperator(std::string& s);
	AluUnitUnaryOperator(const char* str);
	virtual ~AluUnitUnaryOperator()			{}
	virtual unsigned int	countOperands()		{return 1;}
	virtual void			_serialize(std::ostream& os) const;
	virtual std::string		_identify();
	virtual AluUnitType		type()			{return UT_UnaryOp;}
	virtual PValue		compute(PValue operand);
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

#define X(nm,str,prio) PValue compute##nm(PValue op1, PValue op2);
class AluBinaryOperator : public AluUnit {
public:
	AluBinaryOperator(std::string& s);
	AluBinaryOperator(const char* str);
	virtual ~AluBinaryOperator()			{}
	virtual unsigned int	countOperands()		{return 2;}
	virtual void			_serialize(std::ostream& os) const;
	virtual std::string		_identify();
	virtual AluUnitType		type()			{return UT_BinaryOp;}
	virtual PValue		compute(PValue op1, PValue op2);
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

#define X(nm,str) PValue compute##nm(PValue operand, PValue prevValue);
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
	void	perform(ALUCounterKey ctrNumber, ALUCounters* ctrs, PValue operand);
private:
	void               setOpByName(std::string& s);
	ALU_ASSOP_LIST
	ALU_AssignmentOperator	m_op;
};
#undef X

typedef std::shared_ptr<AluAssnOperator> POperator;

class AluFunction : public AluUnit {
public:
	AluFunction(std::string& s);
	virtual ~AluFunction()		{}
	virtual unsigned int		countOperands()		{return m_ArgCount;}
	virtual void				_serialize(std::ostream& os) const;
	virtual std::string			_identify()	{return "FUNC("+m_FuncName+")";}
	virtual AluUnitType			type()		{return UT_Identifier;}
	virtual PValue			evaluate();
	virtual PValue			compute(PValue op);
	virtual PValue			compute(PValue op1, PValue op2);
	virtual PValue			compute(PValue op1, PValue op2, PValue op3);
	virtual PValue			compute(PValue op1, PValue op2, PValue op3, PValue op4);
	virtual PValue			compute(PValue op1, PValue op2, PValue op3, PValue op4, PValue op5);
	virtual bool                requiresRead()  { return m_reliesOnInput; }
	std::string&                getName()       { return m_FuncName; }
	static unsigned char        functionTypes() { return m_flags; }
private:
	// m_flags is static because it's a bitstring that describes all of the functions
	// used in a particular specification.
	static unsigned char m_flags;
	std::string		     m_FuncName;
	void*			     mp_Func;
	unsigned int	     m_ArgCount;
	bool                 m_reliesOnInput;
	ExternalFunctionRec* m_pExternalFunc;
};

class AluInputRecord : public AluUnit {
public:
	AluInputRecord() 					{}
	virtual ~AluInputRecord()			{}
	virtual void   			_serialize(std::ostream& os) const;
	virtual std::string     _identify()	{return "@@";}
	virtual AluUnitType		type()	{return UT_InputRecord;}
	virtual PValue		evaluate();
	virtual bool            requiresRead() {return true;}
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


std::ostream& operator<< (std::ostream& os, const AluUnit &u);

class AluValueStats {
public:
	AluValueStats();
	AluValueStats(char id);
	void         AddValue(char id);

	PValue    sum();
	PValue    sumi();
	PValue    sumf();
	PValue    _min();
	PValue    mini();
	PValue    minf();
	PValue    _max();
	PValue    maxi();
	PValue    maxf();
	PValue    average();
	PValue    variance();
	PValue    stddev();
	PValue    stderrmean();

private:
	void         initialize();
	unsigned int m_intCount;
	unsigned int m_floatCount;
	unsigned int m_totalCount;
	ALUInt       m_sumInt;
	ALUFloat     m_sumFloat;
	ALUInt       m_minInt;
	ALUFloat     m_minFloat;
	ALUInt       m_maxInt;
	ALUFloat     m_maxFloat;
	ALUFloat     m_runningAverage;
	ALUFloat     m_runningSn;  // Sn is the variance multiplied by n
};

typedef std::shared_ptr<AluValueStats> PAluValueStats;

typedef std::vector<PUnit> AluVec;

bool expressionForcesRunoutCycle(AluVec& vec);

bool expressionIsAssignment(AluVec& vec);

bool parseAluExpression(std::string& s, AluVec& vec);

bool parseAluStatement(std::string& s, ALUCounterKey& k, AluAssnOperator* pAss, AluVec& vec);

std::string dumpAluVec(AluVec& vec, bool deleteUnits);

void cleanAluVec(AluVec& vec);

bool isValidExpression(AluVec& vec);  // To Be Added

bool convertAluVecToPostfix(AluVec& source, AluVec& dest, bool clearSource);

bool breakAluVecByComma(AluVec& source, AluVec& dest);

PValue evaluateExpression(AluVec& expr, ALUCounters* pctrs);

void ALUPerformAssignment(ALUCounterKey& k, POperator pAss, AluVec& expr, ALUCounters* pctrs);

bool AluExpressionReadsLines(AluVec& vec);

#endif
