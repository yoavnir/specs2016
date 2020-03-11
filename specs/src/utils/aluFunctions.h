#ifndef SPECS2016__UTILS__ALU_FUNCTIONS_H
#define SPECS2016__UTILS__ALU_FUNCTIONS_H

#include <unordered_map>
#include "utils/SpecString.h"
#include "utils/alu.h"

#define ALUFUNC_REGULAR      0x00
#define ALUFUNC_STATISTICAL  0x01
#define ALUFUNC_FREQUENCY    0x02
#define ALUFUNC_EXTERNAL     0x80

// function name, number of arguments, whether it needs lines from input
#define ALU_FUNCTION_LIST            \
	X(abs,            1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns the absolute value of x","Will return an int or a float depending on the type of the argument.") \
	X(pow,            2, ALUFUNC_REGULAR,     false,  \
			"(x,y) - Returns x to the power of y","Will return an int if both arguments are whole numbers, or a float otherwise.\nThe return value will be int even if the arguments were calculated as floats.") \
	X(sqrt,           1, ALUFUNC_REGULAR,     false,  \
			"(x) - Returns the square root of x","Always returns a float.") \
	X(c2u,            1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns the value of the string s re-cast as an unsigned integer.","A length of 1-8 characters is supported.\nA longer length will cause a runtime error.") \
	X(c2f,            1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns the value of the string s re-cast as a floating point number","The supported lengths are those of the float, double, and long double types.\nAny other length will cause a runtime error.") \
	X(c2d,            1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns the value of the string s re-cast as a signed integer.","A length of 1-8 characters is supported.\nA longer length will cause a runtime error.") \
	X(frombin,        1, ALUFUNC_REGULAR,     false,  \
			"(op) - Re-casts op as a 64-bit number.","A length of 1-8 characters is supported.\nA longer length will cause a runtime error.") \
	X(tobine,         2, ALUFUNC_REGULAR,     false,  \
			"(op,bits) - Returns a binary representation of op.","The unsigned integer in op is converted.\nThe output field has length of bits bits.\nValid values for bits are 8, 16, 32, and 64.") \
	X(tobin,          1, ALUFUNC_REGULAR,     false,  \
			"(op) - Returns  a binary representation of the unsigned integer in x.","The field length is automatically determined by the value of x,\nbut will be 1, 2, 4, or 8 characters in length.") \
	X(length,         1, ALUFUNC_REGULAR,     false,  \
			"(s) - Returns the length of the string s","") \
	X(first,          0, ALUFUNC_REGULAR,     false,  \
			"() - Returns TRUE (1) if this is the first line.","") \
	X(recno,          0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the record number of the current record.","Increments with every READ or READSTOP.") \
	X(number,         0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the number of times this specification has restarted","Does not increment with READ or READSTOP. Otherwise similar to recno().") \
	X(eof,            0, ALUFUNC_REGULAR,     false,  \
			"() - Returns TRUE (1) if this is the run-out phase.","") \
	X(record,         0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the entire record.","Equivalent to the @@ pseudo-variable.") \
	X(wordcount,      0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the number of words in the current record.","Result depends on the current word separator.") \
	X(wordstart,      1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the position in the current record where word i begins.","") \
	X(wordlen,        1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the length of word i in the current record.","") \
	X(wordend,        1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the position in the current record where word i ends.","") \
	X(word,           1, ALUFUNC_REGULAR,      true,  \
			"(i) - Returns the i-th word in the current record.","") \
	X(wordrange,      2, ALUFUNC_REGULAR,      true,  \
			"(i,j) - Returns the substring or current record from the start of the i-th word to the end of the j-th word.","") \
	X(fieldcount,     0, ALUFUNC_REGULAR,      true,  \
			"() - Returns the number of fields in the current record.","Result depends on the current field separator.") \
	X(fieldindex,     1, ALUFUNC_REGULAR,      true,  \
			"","") \
	X(fieldlength,    1, ALUFUNC_REGULAR,      true,  \
			"","") \
	X(fieldend,       1, ALUFUNC_REGULAR,      true,  \
			"","") \
	X(field,          1, ALUFUNC_REGULAR,      true,  \
			"","") \
	X(fieldrange,     2, ALUFUNC_REGULAR,      true,  \
			"","") \
	X(range,          2, ALUFUNC_REGULAR,      true,  \
			"","") \
	X(tf2mcs,         2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(mcs2tf,         2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(tf2s,           2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(s2tf,           2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(substr,         3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(pos,            2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(lastpos,        2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(includes,       5, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(includesall,    5, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(rmatch,         3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(rsearch,        3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(rreplace,       4, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(left,           2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(right,          2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(center,         2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(centre,         2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(conf,           2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(x2d,            2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(d2x,            1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(x2ch,           1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(c2x,            1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(ucase,          1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(lcase,          1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(bswap,          1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(break,          1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(sum,            1, ALUFUNC_STATISTICAL, false,  \
			"","") \
	X(min,            1, ALUFUNC_STATISTICAL, false,  \
			"","") \
	X(max,            1, ALUFUNC_STATISTICAL, false,  \
			"","") \
	X(average,        1, ALUFUNC_STATISTICAL, false,  \
			"","") \
	X(variance,       1, ALUFUNC_STATISTICAL, false,  \
			"","") \
	X(stddev,         1, ALUFUNC_STATISTICAL, false,  \
			"","") \
	X(stderrmean,     1, ALUFUNC_STATISTICAL, false,  \
			"","") \
	X(present,        1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(rand,           1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(floor,          1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(round,          2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(ceil,           1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(sin,            1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(cos,            1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(tan,            1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(arcsin,         1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(arccos,         1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(arctan,         1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(dsin,           1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(dcos,           1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(dtan,           1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(arcdsin,        1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(arcdcos,        1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(arcdtan,        1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(fmap_nelem,     1, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(fmap_nsamples,  1, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(fmap_count,     2, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(fmap_frac,      2, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(fmap_pct,       2, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(fmap_common,    1, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(fmap_rare,      1, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(fmap_sample,    2, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(fmap_dump,      4, ALUFUNC_FREQUENCY,   false,  \
			"","") \
	X(string,         1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(substitute,     4, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(sfield,         3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(sword,          3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(abbrev,         3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(bitand,         2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(bitor,          2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(bitxor,         2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(compare,        3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(copies,         2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(delstr,         3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(delword,        3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(find,           2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(index,          3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(insert,         5, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(justify,        3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(overlay,        5, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(reverse,        1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(sign,           1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(space,          3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(strip,          3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(subword,        3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(translate,      4, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(verify,         4, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(wordindex,      2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(wordlength,     2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(wordpos,        3, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(words,          1, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(xrange,         2, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(fmt,            5, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(next,           0, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(rest,           0, ALUFUNC_REGULAR,     false,  \
			"","") \
	X(defined,        1, ALUFUNC_REGULAR,     false,  \
			"","")

#define ALU_DEBUG_FUNCTION_LIST                       \
	X(testfunc,       4, ALUFUNC_REGULAR,     false,  \
			"","")

#define ALU_PSEUDO_FUNCTION_LIST     \
	X(break)                         \
	X(sum)                           \
	X(min)                           \
	X(max)                           \
	X(average)                       \
	X(variance)                      \
	X(stddev)                        \
	X(stderrmean)                    \
	X(present)                       \
	X(fmap_nelem)                    \
	X(fmap_nsamples)                 \
	X(fmap_count)                    \
	X(fmap_frac)                     \
	X(fmap_pct)                      \
	X(fmap_common)                   \
	X(fmap_rare)                     \
	X(fmap_sample)                   \
	X(fmap_dump)                     \

#define ALUFUNC0(nm)	ALUValue* AluFunc_##nm();
#define ALUFUNC1(nm)	ALUValue* AluFunc_##nm(ALUValue*);
#define ALUFUNC2(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*);
#define ALUFUNC3(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*, ALUValue*);
#define ALUFUNC4(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*, ALUValue*, ALUValue*);
#define ALUFUNC5(nm)	ALUValue* AluFunc_##nm(ALUValue*, ALUValue*, ALUValue*, ALUValue*, ALUValue*);

#define X(fn,argc,flags,rl,shorthelp,longhelp) ALUFUNC##argc(fn)
ALU_FUNCTION_LIST
#ifdef DEBUG
ALU_DEBUG_FUNCTION_LIST
#endif
#undef X

typedef ALUValue* (*AluFunc0)();
typedef ALUValue* (*AluFunc1)(ALUValue* op1);
typedef ALUValue* (*AluFunc2)(ALUValue* op1, ALUValue* op2);
typedef ALUValue* (*AluFunc3)(ALUValue* op1, ALUValue* op2, ALUValue* op3);
typedef ALUValue* (*AluFunc4)(ALUValue* op1, ALUValue* op2, ALUValue* op3, ALUValue* op4);
typedef ALUValue* (*AluFunc5)(ALUValue* op1, ALUValue* op2, ALUValue* op3, ALUValue* op4, ALUValue* op5);

#define MAX_FUNC_OPERANDS 5

enum fmap_format {
	/* Leave a gap because low numbers are the string width */
	fmap_format__textualJustified,
	fmap_format__textualJustifiedLines = 1024,
	fmap_format__csv,
	fmap_format__json,
};

enum fmap_sortOrder {
	fmap_sortOrder__byStringAscending,
	fmap_sortOrder__byStringDescending,
	fmap_sortOrder__byCountAscending,
	fmap_sortOrder__byCountDescending,
};

typedef std::unordered_map<std::string, ALUInt> freqMapImpl;

typedef std::pair<const std::string, ALUInt> freqMapPair;

class frequencyMap {
public:
	void             note(std::string& s);
	ALUInt           nelem()     { return map.size(); }
	ALUInt           operator[](std::string& s) {return map[s];}
	ALUInt           count()     { return counter; }
	std::string      mostCommon();
	std::string      leastCommon();
	std::string      dump(fmap_format f, fmap_sortOrder o, bool includePercentage);
private:
	freqMapImpl      map;
	ALUInt           counter;
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
	virtual PSpecString currRecord() = 0;
	virtual bool    isRunIn() = 0;
	virtual bool    isRunOut() = 0;
	virtual ALUInt  getRecordCount() = 0;
	virtual ALUInt  getIterationCount() = 0;
	virtual bool    breakEstablished(char id) = 0;
	virtual PAluValueStats valueStatistics(char id) = 0;
	virtual PFrequencyMap  getFrequencyMap(char id) = 0;
	virtual bool    fieldIdentifierIsSet(char id) = 0;
};

class positionGetter {
public:
	virtual size_t pos() = 0;
};

void setStateQueryAgent(stateQueryAgent* qa);
void setPositionGetter(positionGetter* pGetter);

void aluFunc_help_builtin();
bool aluFunc_help_one_builtin(std::string& funcName);

#endif
