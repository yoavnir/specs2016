#ifndef SPECS2016__UTILS__ALU_FUNCTIONS_H
#define SPECS2016__UTILS__ALU_FUNCTIONS_H

#include "utils/SpecString.h"
#include "utils/alu.h"

// function name, number of arguments, whether it needs lines from input
#define ALU_FUNCTION_LIST            \
	X(abs,1,false)                   \
	X(pow,2,false)                   \
	X(sqrt,1,false)                  \
	X(frombin,1,false)               \
	X(tobine,2,false)                \
	X(tobin,1,false)                 \
	X(len,1,false)                   \
	X(first,0,false)                 \
	X(recno,0,true)                  \
	X(iterno,0,true)                 \
	X(eof,0,false)                   \
	X(record,0,true)                 \
	X(wordcount,0,true)              \
	X(wordindex,1,true)              \
	X(wordlength,1,true)             \
	X(wordend,1,true)                \
	X(word,1,true)                   \
	X(words,2,true)                  \
	X(fieldcount,0,true)             \
	X(fieldindex,1,true)             \
	X(fieldlength,1,true)            \
	X(fieldend,1,true)               \
	X(field,1,true)                  \
	X(fields,2,true)                 \
	X(range,2,true)                  \
	X(tf2d,2,false)                  \
	X(d2tf,2,false)                  \
	X(substr,3,false)                \
	X(pos,2,false)                   \
	X(rpos,2,false)                  \
	X(includes,2,false)              \
	X(left,2,false)                  \
	X(right,2,false)                 \
	X(center,2,false)                \
	X(centre,2,false)                \
	X(conf,1,false)                  \
	X(x2d,1,false)                   \
	X(d2x,1,false)                   \
	X(x2ch,1,false)                  \
	X(c2x,1,false)                   \
	X(ucase,1,false)                 \
	X(lcase,1,false)                 \
	X(bswap,1,false)                 \
	X(break,1,false)                 \
	X(sum,1,false)                   \
	X(min,1,false)                   \
	X(max,1,false)                   \
	X(avg,1,false)                   \

#define ALU_PSEUDO_FUNCTION_LIST     \
	X(break)                         \
	X(sum)                           \
	X(min)                           \
	X(max)                           \
	X(avg)                           \

#define ALUFUNC0(nm)	ALUValue* AluFunc_##nm();
#define ALUFUNC1(nm)	ALUValue* AluFunc_##nm(ALUValue*);
#define ALUFUNC2(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*);
#define ALUFUNC3(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*, ALUValue*);
#define ALUFUNC4(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*, ALUValue*), ALUValue*);

#define X(fn,argc,rl) ALUFUNC##argc(fn)
ALU_FUNCTION_LIST
#undef X

typedef ALUValue* (*AluFunc0)();
typedef ALUValue* (*AluFunc1)(ALUValue* op1);
typedef ALUValue* (*AluFunc2)(ALUValue* op1, ALUValue* op2);
typedef ALUValue* (*AluFunc3)(ALUValue* op1, ALUValue* op2, ALUValue* op3);
typedef ALUValue* (*AluFunc4)(ALUValue* op1, ALUValue* op2, ALUValue* op3, ALUValue* op4);

class stateQueryAgent {
public:
	virtual unsigned int getWordCount() = 0;
	virtual unsigned int getFieldCount() = 0;
	virtual int     getWordStart(int idx) = 0;
	virtual int     getWordEnd(int idx) = 0;
	virtual int     getFieldStart(int idx) = 0;
	virtual int     getFieldEnd(int idx) = 0;
	virtual PSpecString getFromTo(int from, int to) = 0;
	virtual bool    isRunIn() = 0;
	virtual bool    isRunOut() = 0;
	virtual ALUInt  getRecordCount() = 0;
	virtual ALUInt  getIterationCount() = 0;
	virtual bool    breakEstablished(char id) = 0;
	virtual PAluValueStats valueStatistics(char id) = 0;
};

void setStateQueryAgent(stateQueryAgent* qa);

#endif
