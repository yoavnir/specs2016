#ifndef SPECS2016__UTILS__ALU_FUNCTIONS_H
#define SPECS2016__UTILS__ALU_FUNCTIONS_H

#include <unordered_map>
#include "utils/SpecString.h"
#include "utils/alu.h"

#define ALUFUNC_REGULAR      0x00
#define ALUFUNC_STATISTICAL  0x01
#define ALUFUNC_FREQUENCY    0x02

// function name, number of arguments, whether it needs lines from input
#define ALU_FUNCTION_LIST            \
	X(abs,            1, ALUFUNC_REGULAR,     false)  \
	X(pow,            2, ALUFUNC_REGULAR,     false)  \
	X(sqrt,           1, ALUFUNC_REGULAR,     false)  \
	X(frombin,        1, ALUFUNC_REGULAR,     false)  \
	X(tobine,         2, ALUFUNC_REGULAR,     false)  \
	X(tobin,          1, ALUFUNC_REGULAR,     false)  \
	X(len,            1, ALUFUNC_REGULAR,     false)  \
	X(first,          0, ALUFUNC_REGULAR,     false)  \
	X(recno,          0, ALUFUNC_REGULAR,      true)  \
	X(iterno,         0, ALUFUNC_REGULAR,      true)  \
	X(eof,            0, ALUFUNC_REGULAR,     false)  \
	X(record,         0, ALUFUNC_REGULAR,      true)  \
	X(wordcount,      0, ALUFUNC_REGULAR,      true)  \
	X(wordindex,      1, ALUFUNC_REGULAR,      true)  \
	X(wordlength,     1, ALUFUNC_REGULAR,      true)  \
	X(wordend,        1, ALUFUNC_REGULAR,      true)  \
	X(word,           1, ALUFUNC_REGULAR,      true)  \
	X(words,          2, ALUFUNC_REGULAR,      true)  \
	X(fieldcount,     0, ALUFUNC_REGULAR,      true)  \
	X(fieldindex,     1, ALUFUNC_REGULAR,      true)  \
	X(fieldlength,    1, ALUFUNC_REGULAR,      true)  \
	X(fieldend,       1, ALUFUNC_REGULAR,      true)  \
	X(field,          1, ALUFUNC_REGULAR,      true)  \
	X(fields,         2, ALUFUNC_REGULAR,      true)  \
	X(range,          2, ALUFUNC_REGULAR,      true)  \
	X(tf2d,           2, ALUFUNC_REGULAR,     false)  \
	X(d2tf,           2, ALUFUNC_REGULAR,     false)  \
	X(substr,         3, ALUFUNC_REGULAR,     false)  \
	X(pos,            2, ALUFUNC_REGULAR,     false)  \
	X(rpos,           2, ALUFUNC_REGULAR,     false)  \
	X(includes,       2, ALUFUNC_REGULAR,     false)  \
	X(left,           2, ALUFUNC_REGULAR,     false)  \
	X(right,          2, ALUFUNC_REGULAR,     false)  \
	X(center,         2, ALUFUNC_REGULAR,     false)  \
	X(centre,         2, ALUFUNC_REGULAR,     false)  \
	X(conf,           1, ALUFUNC_REGULAR,     false)  \
	X(x2d,            1, ALUFUNC_REGULAR,     false)  \
	X(d2x,            1, ALUFUNC_REGULAR,     false)  \
	X(x2ch,           1, ALUFUNC_REGULAR,     false)  \
	X(c2x,            1, ALUFUNC_REGULAR,     false)  \
	X(ucase,          1, ALUFUNC_REGULAR,     false)  \
	X(lcase,          1, ALUFUNC_REGULAR,     false)  \
	X(bswap,          1, ALUFUNC_REGULAR,     false)  \
	X(break,          1, ALUFUNC_REGULAR,     false)  \
	X(sum,            1, ALUFUNC_STATISTICAL, false)  \
	X(min,            1, ALUFUNC_STATISTICAL, false)  \
	X(max,            1, ALUFUNC_STATISTICAL, false)  \
	X(avg,            1, ALUFUNC_STATISTICAL, false)  \
	X(rand,           1, ALUFUNC_REGULAR,     false)  \
	X(frand,          0, ALUFUNC_REGULAR,     false)  \
	X(floor,          1, ALUFUNC_REGULAR,     false)  \
	X(round,          1, ALUFUNC_REGULAR,     false)  \
	X(roundd,         2, ALUFUNC_REGULAR,     false)  \
	X(ceil,           1, ALUFUNC_REGULAR,     false)  \
	X(sin,            1, ALUFUNC_REGULAR,     false)  \
	X(cos,            1, ALUFUNC_REGULAR,     false)  \
	X(tan,            1, ALUFUNC_REGULAR,     false)  \
	X(arcsin,         1, ALUFUNC_REGULAR,     false)  \
	X(arccos,         1, ALUFUNC_REGULAR,     false)  \
	X(arctan,         1, ALUFUNC_REGULAR,     false)  \
	X(dsin,           1, ALUFUNC_REGULAR,     false)  \
	X(dcos,           1, ALUFUNC_REGULAR,     false)  \
	X(dtan,           1, ALUFUNC_REGULAR,     false)  \
	X(arcdsin,        1, ALUFUNC_REGULAR,     false)  \
	X(arcdcos,        1, ALUFUNC_REGULAR,     false)  \
	X(arcdtan,        1, ALUFUNC_REGULAR,     false)  \
	X(fmap_nelem,     1, ALUFUNC_FREQUENCY,   false)  \
	X(fmap_nsamples,  1, ALUFUNC_FREQUENCY,   false)  \
	X(fmap_count,     2, ALUFUNC_FREQUENCY,   false)  \
	X(fmap_frac,      2, ALUFUNC_FREQUENCY,   false)  \
	X(fmap_pct,       2, ALUFUNC_FREQUENCY,   false)  \

#define ALU_PSEUDO_FUNCTION_LIST     \
	X(break)                         \
	X(sum)                           \
	X(min)                           \
	X(max)                           \
	X(avg)                           \
	X(fmap_nelem)                    \
	X(fmap_nsamples)                 \
	X(fmap_count)                    \
	X(fmap_frac)                     \
	X(fmap_pct)                      \

#define ALUFUNC0(nm)	ALUValue* AluFunc_##nm();
#define ALUFUNC1(nm)	ALUValue* AluFunc_##nm(ALUValue*);
#define ALUFUNC2(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*);
#define ALUFUNC3(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*, ALUValue*);
#define ALUFUNC4(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*, ALUValue*), ALUValue*);

#define X(fn,argc,flags,rl) ALUFUNC##argc(fn)
ALU_FUNCTION_LIST
#undef X

typedef ALUValue* (*AluFunc0)();
typedef ALUValue* (*AluFunc1)(ALUValue* op1);
typedef ALUValue* (*AluFunc2)(ALUValue* op1, ALUValue* op2);
typedef ALUValue* (*AluFunc3)(ALUValue* op1, ALUValue* op2, ALUValue* op3);
typedef ALUValue* (*AluFunc4)(ALUValue* op1, ALUValue* op2, ALUValue* op3, ALUValue* op4);

// typedef std::unordered_map<std::string,ALUInt>  frequencyMap;

class frequencyMap {
public:
	void             note(std::string& s);
	ALUInt           nelem()     { return map.size(); }
	ALUInt           operator[](std::string& s) {return map[s];}
	ALUInt           count()     { return counter; }
private:
	std::unordered_map<std::string,ALUInt> map;
	ALUInt                                 counter;
};
typedef frequencyMap *PFrequencyMap;

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
	virtual PFrequencyMap  getFrequencyMap(char id) = 0;
};

void setStateQueryAgent(stateQueryAgent* qa);

#endif
