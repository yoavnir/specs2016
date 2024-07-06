#include <iostream>
#include <iomanip>
#include <vector>
#include <limits>  // For std::numeric_limits<ALUFloat>::quiet_NaN()
#include "utils/platform.h"  // For put_time and get_time vs strftime and strptime
#include "utils/ErrorReporting.h"
#include "utils/alu.h"
#include "utils/aluRegex.h"
#include "utils/TimeUtils.h"
#include "processing/ProcessingState.h"
#include "processing/persistent.h"

ALUCounters counters;
ProcessingState g_ps;

std::string counterTypeNames[]= {"None", "Str", "Int", "Float"};

extern void setRegexType(std::string& s);
extern bool g_bWarnAboutGrammars;

#define INC_TEST_INDEX if (++testIndex!=onlyTest && onlyTest!=0) break;
#define INC_TEST_INDEX2 if (++testIndex==onlyTest || onlyTest==0)

#define VERIFY_TYPE(i,t) do {\
	INC_TEST_INDEX;				\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": type[" << i << "]=="#t": "; \
	if (counters.type(i)==counterType__##t) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counterTypeNames[counters.type(i)] << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} } while(0);

#define VERIFY_INT(i,val) do { \
	INC_TEST_INDEX;				\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": #" << i << "==" << val << ": "; \
	if (counters.getInt(i)==val) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counters.getInt(i) << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} } while(0);

#define VERIFY_WHOLE(i,b) do { \
	INC_TEST_INDEX;				\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": #" << i << (b ? " is whole: ":" isn't whole: "); \
	if (counters.isWholeNumber(i)==b) {  \
		std::cout << "OK (" << counters.getFloat(i) << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << counters.getFloat(i) << ")\n";  \
		countFailures++;  failedTests.push_back(testIndex); \
	} } while(0);

#define VERIFY_HEX(i,val) do { \
	INC_TEST_INDEX;            \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": #" << i << "==" << val << ": "; \
	if (counters.getHex(i)==val) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counters.getHex(i) << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} } while(0);

#define VERIFY_FLOAT(i,val) do { \
	INC_TEST_INDEX; \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": #" << i << "==" << std::setprecision(ALUFloatPrecision) << val << ": "; \
	if (counters.getFloat(i)==val) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counters.getFloat(i) << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} } while(0);

#define VERIFY_STR(i,val)  do { \
	INC_TEST_INDEX;				\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": #" << i << "==" << val << ": "; \
	if (counters.getStr(i)==val) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counters.getStr(i) << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} } while(0);

#define VERIFY_NUMERIC(i,b)	 do { \
	INC_TEST_INDEX;				\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": #" << i << (b ? " is numeric: ":" isn't numeric: "); \
	if (counters.isNumeric(i)==b) {  \
		std::cout << "OK (";    \
	} else {     \
		std::cout << "*** NOT OK *** (";  \
		countFailures++;  failedTests.push_back(testIndex); \
	}  \
	std::cout << counters.getStr(i) << ")\n";  \
} while(0);

#define VERIFY_DIVINED_TYPE(i,t)  do { \
	INC_TEST_INDEX;				\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": #" << i << " is "<< ALUCounterType2Str[counterType__##t] <<": "; \
	if (counters.divinedType(i)==counterType__##t) { \
		std::cout << "OK (" << counters.getStr(i) << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << counters.getStr(i) << " - " <<   \
				ALUCounterType2Str[counters.divinedType(i)] << ")\n";  \
		countFailures++;  failedTests.push_back(testIndex); \
	} } while(0);

#define UNIT_DIVINED_TYPE(u,t) do { \
	INC_TEST_INDEX;				\
	PValue ctr = u.evaluate(); \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": "<< #u << " is "<< ALUCounterType2Str[counterType__##t] <<": "; \
	if (ctr->getDivinedType()==counterType__##t) { \
		std::cout << "OK (" << ctr->getStr() << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << ctr->getStr() << " - " <<   \
				ALUCounterType2Str[ctr->getDivinedType()] << ")\n";  \
		countFailures++;  failedTests.push_back(testIndex); \
	} \
} while(0);

#define VERIFY_UNIT_ST(u,s) do { \
	INC_TEST_INDEX;				\
	PValue ctr = u.compute(&counters); \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": "<< #u << " is \""<< s <<"\": "; \
	if (ctr->getStr()==s) { \
		std::cout << "OK.\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << ctr->getStr() << ")\n";  \
		countFailures++;  failedTests.push_back(testIndex); \
	} \
} while(0);

#define VERIFY_UNIT_INT(u,i) do { \
	INC_TEST_INDEX;				\
	PValue ctr = u.compute(&counters); \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": "<< #u << " is "<< i <<": "; \
	if (ctr->getInt()==i) { \
		std::cout << "OK (" << ctr->getStr() << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << ctr->getStr() << " - " << ctr->getInt() << ")\n";  \
		countFailures++;  failedTests.push_back(testIndex); \
	} \
} while(0);

#define VERIFY_UNIT_F(u,f) do { \
	INC_TEST_INDEX;				\
	PValue ctr = u.compute(&counters); \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": "<< #u << " is "<< f <<": "; \
	if (ctr->getFloat()==f) { \
		std::cout << "OK (" << ctr->getStr() << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << ctr->getStr() << " - " << ctr->getFloat() << ")\n";  \
		countFailures++;  failedTests.push_back(testIndex); \
	} \
} while(0);

#define VERIFY_UNARY(u,o,t,s) do { \
	INC_TEST_INDEX;				\
	AluUnitCounter ctr(o);	\
	PValue _op = ctr.compute(&counters);	\
	PValue _res = u.compute(_op);	\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": "<< #u << "(" << #t << ") is \""<< s <<"\": "; \
	if (counterType__##t!=_res->getType()) { \
		std::cout << "*** NOT OK *** (type=" << ALUCounterType2Str[_res->getType()] << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} else if ((_res->getType()!=counterType__None) && (_res->getStr() != s)) {	\
		std::cout << "*** NOT OK *** (" << _res->getStr() << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} else { \
		std::cout << "OK.\n"; \
	} \
} while(0);

#define VERIFY_BINARY(u,o1,o2,t,s) do { \
	INC_TEST_INDEX;				\
	AluUnitCounter ctr1(o1);	\
	AluUnitCounter ctr2(o2);	\
	PValue _op1 = ctr1.compute(&counters);	\
	PValue _op2 = ctr2.compute(&counters);	\
	PValue _res = u.compute(_op1,_op2);	\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": "<< #u << "(" << #t << ") is \""<< s <<"\": "; \
	if (counterType__##t!=_res->getType()) { \
		std::cout << "*** NOT OK *** (type=" << ALUCounterType2Str[_res->getType()] << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} else if ((_res->getType()!=counterType__None) && (_res->getStr() != s)) {	\
		std::cout << "*** NOT OK *** (" << _res->getStr() << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} else { \
		std::cout << "OK.\n"; \
	} \
} while(0);

#define VERIFY_ASSN(u,p,o,t,s) do {		\
	INC_TEST_INDEX;				\
	AluUnitCounter 	ctr(o);	\
	PValue		op = ctr.compute(&counters);	\
	u.perform(p,&counters,op);			\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": "<< #u << "(" << #t << ") is \""<< s <<"\": "; \
	if (counterType__##t!=counters.type(p)) { \
		std::cout << "*** NOT OK *** (type=" << ALUCounterType2Str[counters.type(p)] << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} else if ((counters.type(p)!=counterType__None) && (counters.getStr(p) != s)) {	\
		std::cout << "*** NOT OK *** (" << counters.getStr(p) << ")\n"; \
		countFailures++;  failedTests.push_back(testIndex); \
	} else { \
		std::cout << "OK.\n"; \
	} \
} while(0);

#define VERIFY_EXPR(s,e) do {					\
	INC_TEST_INDEX;								\
	std::string _expr(s);						\
	parseAluExpression(_expr,vec);				\
	std::string _dump = dumpAluVec(vec, true);	\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": <"<< s << "> ==> \"" << e << "\": "; 	\
	if (_dump==e) std::cout << "OK\n";			\
	else {										\
		std::cout << "*** NOT OK *** - " << _dump << "\n";	\
		countFailures++;  failedTests.push_back(testIndex);	\
	}											\
} while(0);

#define VERIFY_ASSNMENT(s,ex) do {							\
	INC_TEST_INDEX;											\
	std::string _expr(s);									\
	ALUCounterKey	k;										\
	AluAssnOperator op;										\
	std::string	actual;										\
	bool _res;												\
	try {													\
		_res = parseAluStatement(_expr,k,&op,vec);			\
		if (_res) {											\
			std::string _dump = dumpAluVec(vec, true);		\
			_res = (_dump==ex);								\
			actual = _dump;									\
		}													\
	} catch (SpecsException& e) {							\
		_res =  (e.what(true)==std::string(ex));			\
		actual = e.what(true);								\
		cleanAluVec(vec);									\
	}														\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
	": <"<< s << "> ==> \"" << ex << "\": "; 				\
	if (_res) std::cout << "OK\n";							\
	else {													\
		std::cout << "*** NOT OK *** - " << actual << "\n";	\
		countFailures++;  failedTests.push_back(testIndex);	\
	}														\
} while(0);

#define VERIFY_RPN(s,ex) do {								\
		INC_TEST_INDEX;										\
		std::string _expr(s);								\
		AluVec rpnVec;										\
		std::string _dump;									\
		bool _res;											\
		try {												\
			_res = parseAluExpression(_expr,vec);			\
			_res = convertAluVecToPostfix(vec, rpnVec,true);\
			_dump = dumpAluVec(rpnVec, true);				\
		} catch (SpecsException& e) {						\
			_dump = e.what(true);							\
		}													\
		_res = (_dump==ex);									\
		std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": <"<< s << "> ==> \"" << ex << "\": ";			\
		if (_res) std::cout << "OK\n";						\
		else {												\
			std::cout << "*** NOT OK *** - " << _dump << "\n";	\
			countFailures++;  failedTests.push_back(testIndex);	\
		}													\
	} while(0);


#define VERIFY_EXPR_RES(s,res) 								\
		INC_TEST_INDEX2 {									\
		_expr = s;											\
		_res2 = true;										\
		_result = nullptr;									\
		g_ps.Reset();										\
		_res = parseAluExpression(_expr,vec);				\
		if (_res) {                                         \
			if (expressionIsAssignment(vec)) {              \
				ALUCounterKey	k;							\
				AluAssnOperator op;							\
				cleanAluVec(vec);							\
				_res = parseAluStatement(_expr,k,&op,vec);  \
				if (_res) _res = convertAluVecToPostfix(vec, rpnVec,true); \
				if (_res) {									\
					POperator pop = std::make_shared<AluAssnOperator>(op); \
					ALUPerformAssignment(k,pop,rpnVec,&counters); \
					_res2 = (counters.getStr(k)==res);		\
					cleanAluVec(rpnVec);					\
				}											\
			} else {                                        \
				_res2 = true;								\
				_res = convertAluVecToPostfix(vec, rpnVec,true);	\
				if (_res) try {                             \
					_result = evaluateExpression(rpnVec, &counters);	\
				} catch(SpecsException& e) {                \
					_result = mkValue(e.what(true));        \
				}                                           \
				cleanAluVec(rpnVec);						\
				_res = (_result!=nullptr) && (_result->getStr()==res);	\
			}                                               \
		}                                                   \
		std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": "<< s << " ==> " << res << ": ";					\
		if (_res && _res2) std::cout << "OK\n";						\
		else {												\
			std::cout << "*** NOT OK *** - " << *_result << "\n";	\
			countFailures++;  failedTests.push_back(testIndex);		\
		}															\
	}

#define VERIFY_ASSN_RES(s,exp) do {								\
		INC_TEST_INDEX;											\
		std::string _expr(s);									\
		ALUCounterKey	k;										\
		AluAssnOperator op;										\
		AluVec rpnVec;											\
		bool _res, _res2 = false;								\
		_res = parseAluStatement(_expr,k,&op,vec);				\
		if (_res) _res = convertAluVecToPostfix(vec, rpnVec,true); \
		if (_res) {												\
			POperator pop = std::make_shared<AluAssnOperator>(op); \
			ALUPerformAssignment(k,pop,rpnVec,&counters);			\
			_res2 = (counters.getStr(k)==exp);					\
		}														\
		cleanAluVec(rpnVec); 									\
		std::cout << "Test #" << std::setfill('0') << std::setw(3) << testIndex << \
		": <"<< s << "> ==> \"" << exp << "\": "; 				\
		if (_res && _res2) std::cout << "OK\n";					\
		else {													\
			std::cout << "*** NOT OK *** - ";					\
			countFailures++;  failedTests.push_back(testIndex);		\
			if (!_res) std::cout << "Expression did not parse.\n";	\
			else std::cout << "#" << k << " == " << counters.getStr(k) << "\n";	\
		}														\
	} while(0);

void setRegexType(const char* str)
{
	std::string s = str;
	setRegexType(s);
}

class testGetter : public fieldIdentifierGetter {
public:
	virtual ~testGetter() {}
	virtual std::string Get(char id) 	{return m_map[id];}
	void set(char id, std::string s)	{m_map[id] = s;}
private:
	std::map<char,std::string> m_map;
};

/* global variables */
std::vector<unsigned int> failedTests;
unsigned int testIndex = 0;
unsigned int countFailures = 0;
testGetter tg;
AluVec vec;
std::string _expr;
AluVec rpnVec;
bool _res, _res2;
PValue _result;
/* global variables */

int runALUUnitTests(unsigned int onlyTest)
{

	setStateQueryAgent(&g_ps);
	setFieldIdentifierGetter(&tg);

	tg.set('b', "84");
	tg.set('c', "specs");
	tg.set('z', "0.0");
	tg.set('n', "-9.8");

	std::cout << "\nCounter Types and Values\n========================\n\n";

	// All variables are None before they're set
	VERIFY_TYPE(0,None);
	VERIFY_TYPE(5,None);

	counters.set(3,"3.14159265");

	// Set as string, it should be string regardless of content
	VERIFY_TYPE(3,Str);

	// Float values are rounded down for Int
	VERIFY_STR(3, "3.14159265");
	VERIFY_INT(3, 3);
	VERIFY_FLOAT(3, 3.14159265L);
	VERIFY_TYPE(3,Str);   // Reading should not change the type

	// But setting should...
	counters.set(3,3.14159265L);
	VERIFY_TYPE(3,Float)
	VERIFY_STR(3, "3.14159265");
	VERIFY_INT(3, 3);
	VERIFY_FLOAT(3, 3.14159265L);

	long long bigInt = (long long int)(1234567890123456789);
	counters.set(5, bigInt);
	VERIFY_TYPE(5,Int);
	VERIFY_STR(5,"1234567890123456789");
	VERIFY_FLOAT(5,1234567890123456789.0L);
	VERIFY_INT(5, bigInt);

	// is PI whole?
	counters.set(1,"hello");
	counters.set(11,"-8.0");
	counters.set(10,"7.5");
	counters.set(33,ALUInt(3));
	VERIFY_WHOLE(3,false);
	counters.set(2, 948.0L);
	VERIFY_WHOLE(2, true);
	VERIFY_WHOLE(1, false);
	VERIFY_WHOLE(11,true);
	VERIFY_WHOLE(10,false);

	// What is numeric?
	VERIFY_NUMERIC(3,true);
	counters.set(4,"123xl");
	counters.set(6,3.00L);
	counters.set(7,"98.6fever");
	VERIFY_NUMERIC(3,true);
	VERIFY_NUMERIC(5,true);
	VERIFY_NUMERIC(1,false);
	VERIFY_NUMERIC(6,true);
	VERIFY_NUMERIC(4,false);
	VERIFY_NUMERIC(7,false);
	VERIFY_INT(4,123);
	counters.set(8,"0.986e+2");
	VERIFY_FLOAT(7,98.6L);
	VERIFY_FLOAT(8,98.6L);

	// What are the divined types
	counters.set(9,"65");
	VERIFY_DIVINED_TYPE(9,Int);
	VERIFY_DIVINED_TYPE(3,Float);
	VERIFY_DIVINED_TYPE(4,Str);
	VERIFY_DIVINED_TYPE(7,Str);
	VERIFY_DIVINED_TYPE(0,None);
	VERIFY_DIVINED_TYPE(11,Float);  // -8.0 should be considered float

	std::cout << "\nALU Units\n=========\n\n";

	// Some ALU Units
	std::string s = "hello";
	AluUnitLiteral uHello(s);
	UNIT_DIVINED_TYPE(uHello,Str)
	s = "2.718281828459";
	AluUnitLiteral uE(s);
	UNIT_DIVINED_TYPE(uE,Float);
	s = "42";
	AluUnitLiteral uMeaning(s);
	UNIT_DIVINED_TYPE(uMeaning,Int);

	AluUnitCounter uCtr(8);
	VERIFY_UNIT_F(uCtr,98.6L);
	VERIFY_UNIT_ST(uCtr,"0.986e+2");
	VERIFY_UNIT_INT(uCtr,98)

	AluUnitUnaryOperator uPlus("+");
	AluUnitUnaryOperator uMinus("-");
	AluUnitUnaryOperator uNot("!");
	VERIFY_UNARY(uPlus,8,Float,"98.6");
	VERIFY_UNARY(uPlus,11,Float,"-8");
	VERIFY_UNARY(uPlus,9,Int,"65");
	VERIFY_UNARY(uPlus,4,Int,"123");
	VERIFY_UNARY(uPlus,1,Int,"0");
	VERIFY_UNARY(uMinus,8,Float,"-98.6");
	VERIFY_UNARY(uMinus,11,Float,"8");
	VERIFY_UNARY(uMinus,9,Int,"-65");
	VERIFY_UNARY(uMinus,4,Int,"-123");
	VERIFY_UNARY(uMinus,1,Int,"0");
	VERIFY_UNARY(uNot,8,Int,"0");
	VERIFY_UNARY(uNot,11,Int,"0");
	VERIFY_UNARY(uNot,9,Int,"0");
	VERIFY_UNARY(uNot,4,Int,"0");
	VERIFY_UNARY(uNot,1,Int,"0");
	counters.set(12,ALUInt(0));
	counters.set(13,0.0L);
	counters.set(0);
	VERIFY_UNARY(uNot,0,None,"0");
	VERIFY_UNARY(uNot,12,Int,"1");
	VERIFY_UNARY(uNot,13,Int,"1");

	return 0;
}

int runALUUnitTests2(unsigned int onlyTest)
{
#define X(nm,st,prio) AluBinaryOperator u##nm(st);
	ALU_BOP_LIST
#undef X
	counters.set(4,"123");
	counters.set(21,ALUInt(-8));
	counters.set(22,-4.5L);
	counters.set(23,"specs");
	counters.set(24,"-8.0");
	VERIFY_BINARY(uAdd,8,11,Float,"90.6");  // 98.6 + (-8)
	VERIFY_BINARY(uAdd,1,3,Float,"3.14159265");  // "hello" + 3.14159265
	VERIFY_BINARY(uAdd,9,3,Float,"68.14159265"); // 65 + 3.14159265
	VERIFY_BINARY(uAdd,9,4,Int,"188"); // 65 + 123
	VERIFY_BINARY(uAdd,9,21,Int,"57"); // 56 + (-8)
	VERIFY_BINARY(uSub,4,9,Int,"58");  // 123 - 65
	VERIFY_BINARY(uSub,9,4,Int,"-58"); // 65 - 123
	VERIFY_BINARY(uSub,9,3,Float,"61.85840735"); // 65 - 3.14159265
	VERIFY_BINARY(uSub,3,1,Float,"3.14159265"); // 3.14159265 - "hello"
	VERIFY_BINARY(uSub,3,22,Float,"7.64159265"); // 3.14159265 - (-4.5)
	VERIFY_BINARY(uMult,8,11,Float,"-788.8");    // 98.6 * (-8)
	VERIFY_BINARY(uMult,9,4,Int,"7995");    // 65*123
	VERIFY_BINARY(uMult,3,9,Float,"204.20352225");	// Pi * 65
#if ALUFloatPrecision == 15
	VERIFY_BINARY(uDiv,4,9,Float,"1.89230769230769");  // 123 / 65 - VS has one less digit of precision
#elif ALUFloatPrecision == 16
	VERIFY_BINARY(uDiv,4,9,Float,"1.892307692307692");  // 123 / 65
#elif ALUFloatPrecision == 18
	VERIFY_BINARY(uDiv,4,9,Float,"1.89230769230769231");  // 123 / 65
#elif ALUFloatPrecision == 33
	VERIFY_BINARY(uDiv,4,9,Float,"1.89230769230769230769230769230769");  // 123 / 65
#else
#warning "Dropping test case because of unsupported precision"
#endif
	VERIFY_BINARY(uDiv,4,33,Int,"41");      // 123 / 3
	VERIFY_BINARY(uDiv,4,99,None,"");       // 123 / 0 = NaN
	VERIFY_BINARY(uIntDiv,4,9,Int,"1");     // 123 % 65 where % is integer division
	VERIFY_BINARY(uIntDiv,7,3,Int,"32");     // 98.6 % 3.14
	VERIFY_BINARY(uIntDiv,4,99,None,"");       // 123 // 0 = NaN
	VERIFY_BINARY(uRemDiv,4,9,Int,"58");     // 123 // 65 where // is modulo
	VERIFY_BINARY(uRemDiv,7,3,Int,"2");     // 98.6 // 3.14 ==> 98 // 3
	VERIFY_BINARY(uRemDiv,4,99,None,"");       // 123 % 0 = NaN
	VERIFY_BINARY(uAppnd,1,3,Str,"hello3.14159265"); // "hello" || "3.14159265"
	VERIFY_BINARY(uAppnd,23,1,Str,"specshello");	 // "specs" || "hello"
	VERIFY_BINARY(uLT,9,4,Int,"1");	// 65 < 123
	VERIFY_BINARY(uLT,4,9,Int,"0");   // 123 < 65
	VERIFY_BINARY(uLT,21,22,Int,"1");	// -8 < -4.5
	VERIFY_BINARY(uLT,21,21,Int,"0");   // -8 < -8
	VERIFY_BINARY(uLT,1,23,Int,"1");	// "hello" < "specs"
	VERIFY_BINARY(uLT,23,1,Int,"0");	// "specs" < "hello"
	VERIFY_BINARY(uLE,9,4,Int,"1");	// 65 <= 123
	VERIFY_BINARY(uLE,4,9,Int,"0");   // 123 <= 65
	VERIFY_BINARY(uLE,21,22,Int,"1");	// -8 <= -4.5
	VERIFY_BINARY(uLE,21,21,Int,"1");   // -8 <= -8
	VERIFY_BINARY(uLE,23,23,Int,"1");   // "specs" <= "specs"
	VERIFY_BINARY(uLE,21,24,Int,"1");	// -8 <= -8.0
	VERIFY_BINARY(uGT,9,4,Int,"0");	// 65 > 123
	VERIFY_BINARY(uGT,4,9,Int,"1");   // 123 > 65
	VERIFY_BINARY(uGT,21,22,Int,"0");	// -8 > -4.5
	VERIFY_BINARY(uGT,21,21,Int,"0");   // -8 > -8
	VERIFY_BINARY(uGT,1,23,Int,"0");	// "hello" > "specs"
	VERIFY_BINARY(uGT,23,1,Int,"1");	// "specs" > "hello"
	VERIFY_BINARY(uGE,9,4,Int,"0");	// 65 >= 123
	VERIFY_BINARY(uGE,4,9,Int,"1");   // 123 >= 65
	VERIFY_BINARY(uGE,21,22,Int,"0");	// -8 >= -4.5
	VERIFY_BINARY(uGE,21,21,Int,"1");   // -8 >= -8
	VERIFY_BINARY(uGE,23,23,Int,"1");   // "specs" >= "specs"
	VERIFY_BINARY(uGE,21,24,Int,"1");	// -8 >= -8.0
	VERIFY_BINARY(uSLT,21,22,Int,"0");	// -8 << -4.5
	VERIFY_BINARY(uSLT,22,21,Int,"1");	// -4.5 << -8
	VERIFY_BINARY(uSGT,21,22,Int,"1");	// -8 >> -4.5
	VERIFY_BINARY(uSGT,22,21,Int,"0");	// -4.5 >> -8
	VERIFY_BINARY(uSLTE,21,22,Int,"0");	// -8 <<= -4.5
	VERIFY_BINARY(uSLTE,22,21,Int,"1");	// -4.5 <<= -8
	VERIFY_BINARY(uSLTE,21,21,Int,"1"); // -8 <<= -8
	VERIFY_BINARY(uSLTE,21,24,Int,"1"); // -8 <<= -8.0  - this is the strict part
	VERIFY_BINARY(uSGTE,21,22,Int,"1");	// -8 >>= -4.5
	VERIFY_BINARY(uSGTE,22,21,Int,"0");	// -4.5 >>= -8
	VERIFY_BINARY(uSGTE,21,21,Int,"1"); // -8 >>= -8
	VERIFY_BINARY(uSGTE,21,24,Int,"0"); // -8 >>= -8.0  - this is the strict part
	VERIFY_BINARY(uEQ,21,21,Int,"1");   // -8 = -8
	VERIFY_BINARY(uEQ,21,24,Int,"1");   // -8 = -8.0
	VERIFY_BINARY(uEQ,23,23,Int,"1");	// "specs" = "specs"
	VERIFY_BINARY(uEQ,21,22,Int,"0");	// -8 = -4.5
	VERIFY_BINARY(uSEQ,21,21,Int,"1");   // -8 == -8
	VERIFY_BINARY(uSEQ,21,24,Int,"0");   // -8 == -8.0
	VERIFY_BINARY(uSEQ,23,23,Int,"1");	// "specs" == "specs"
	VERIFY_BINARY(uSEQ,21,22,Int,"0");	// -8 == -4.5
	VERIFY_BINARY(uNE,21,21,Int,"0");   // -8 = -8
	VERIFY_BINARY(uNE,21,24,Int,"0");   // -8 = -8.0
	VERIFY_BINARY(uNE,23,23,Int,"0");	// "specs" = "specs"
	VERIFY_BINARY(uNE,21,22,Int,"1");	// -8 = -4.5
	VERIFY_BINARY(uSNE,21,21,Int,"0");   // -8 == -8
	VERIFY_BINARY(uSNE,21,24,Int,"1");   // -8 == -8.0
	VERIFY_BINARY(uSNE,23,23,Int,"0");	// "specs" == "specs"
	VERIFY_BINARY(uSNE,21,22,Int,"1");	// -8 == -4.5
	counters.set(30,ALUInt(0));
	counters.set(31,ALUInt(1));
	counters.set(32,0.0L);
	VERIFY_BINARY(uAND,30,30,Int,"0");	// 0 and 0
	VERIFY_BINARY(uAND,30,31,Int,"0");	// 0 and 1
	VERIFY_BINARY(uAND,31,30,Int,"0");	// 1 and 0
	VERIFY_BINARY(uAND,31,31,Int,"1");	// 1 and 1
	VERIFY_BINARY(uAND,31,1,Int,"1");   // 1 and "hello"
	VERIFY_BINARY(uAND,31,3,Int,"1");   // 1 and 3.14159265
	VERIFY_BINARY(uAND,31,32,Int,"0");	// 1 and 0.0
	VERIFY_BINARY(uOR,30,30,Int,"0");	// 0 or 0
	VERIFY_BINARY(uOR,30,31,Int,"1");	// 0 or 1
	VERIFY_BINARY(uOR,31,30,Int,"1");	// 1 or 0
	VERIFY_BINARY(uOR,31,31,Int,"1");	// 1 or 1
	VERIFY_BINARY(uOR,30,1,Int,"1");    // 0 or "hello"
	VERIFY_BINARY(uOR,30,3,Int,"1");    // 0 or 3.14159265
	VERIFY_BINARY(uOR,30,32,Int,"0");	// 0 or 0.0
	return 0;
}

int runALUUnitTests3(unsigned int onlyTest)
{
#define X(nm,st) AluAssnOperator uAss##nm(st);
	ALU_ASSOP_LIST
#undef X

	VERIFY_ASSN(uAssLet,33,3,Float,"3.14159265");
	counters.set(33,3.14159265L);
	VERIFY_ASSN(uAssAdd,33,3,Float,"6.2831853");
	counters.set(33,6.2831853L);
	VERIFY_ASSN(uAssSub,33,3,Float,"3.14159265");
	counters.set(33,3.14159265L);
	VERIFY_ASSN(uAssAppnd,33,3,Str,"3.141592653.14159265");

	// TODO: Many more needed

	std::cout << "\nExpressions\n===========\n\n";

	VERIFY_EXPR("23+45", "Number(23);BOP(+);Number(45)");
	VERIFY_EXPR(" 23 + -8", "Number(23);BOP(+);Number(-8)");

	VERIFY_EXPR("34+b-#2","Number(34);BOP(+);FI(b);BOP(-);Counter(2)");
	VERIFY_EXPR("34+b- -c","Number(34);BOP(+);FI(b);BOP(-);UOP(-);FI(c)");

	VERIFY_EXPR("2*(2+2)","Number(2);BOP(*);(;Number(2);BOP(+);Number(2);)");

	VERIFY_EXPR("5+sqrt(1)-pow(3,4)","Number(5);BOP(+);FUNC(sqrt);(;Number(1););BOP(-);FUNC(pow);(;Number(3);COMMA;Number(4);)");
	VERIFY_EXPR("a>b & b>c","FI(a);BOP(>);FI(b);BOP(&);FI(b);BOP(>);FI(c)");

	VERIFY_EXPR("37%10", "Number(37);BOP(%);Number(10)");

	VERIFY_EXPR("length(5)", "FUNC(length);(;Number(5);)");

	VERIFY_EXPR("length('hello')", "FUNC(length);(;Literal(hello);)");

	VERIFY_EXPR("length(hello)", "FUNC(length);(;Literal(hello);)"); // Issue #141 - arguments without quotes

	// TODO: Yeah, a whole bunch of more expressions

	std::cout << "\nAssignment Statements\n=====================\n\n";

	VERIFY_ASSNMENT("#6 := 2+2","Number(2);BOP(+);Number(2)");
	VERIFY_ASSNMENT("#6 = 2+2","ALU assignment statements must have an assignment operator as the second element. Got BOP(=) instead.");
	VERIFY_ASSNMENT("b := 2+2","ALU assignment statements must begin with a counter. Got FI(b) instead.");
	VERIFY_ASSNMENT("#6 b += 2+2","ALU assignment statements must have an assignment operator as the second element. Got FI(b) instead.");
	VERIFY_ASSNMENT("2+2 := 4","ALU assignment statements must begin with a counter. Got Number(2) instead.");
	VERIFY_ASSNMENT("#0 += a", "FI(a)");

	// TODO: More
	return 0;
}

int runALUUnitTests4(unsigned int onlyTest)
{
	std::cout << "\nInfix to RPN Conversions - Shunting Yard Algorithm\n==================================================\n\n";

	VERIFY_RPN("2+3", "Number(2);Number(3);BOP(+)");
	VERIFY_RPN("-b","FI(b);UOP(-)");
	VERIFY_RPN("-2+3","Number(-2);Number(3);BOP(+)");
	VERIFY_RPN("-(2+3)","Number(2);Number(3);BOP(+);UOP(-)");
	VERIFY_RPN("37%10","Number(37);Number(10);BOP(%)");
	VERIFY_RPN("a+b-c","FI(a);FI(b);BOP(+);FI(c);BOP(-)");
	VERIFY_RPN("a-b+c","FI(a);FI(b);BOP(-);FI(c);BOP(+)")
	VERIFY_RPN("a*b-c","FI(a);FI(b);BOP(*);FI(c);BOP(-)");
	VERIFY_RPN("a+b*c","FI(a);FI(b);FI(c);BOP(*);BOP(+)");
	VERIFY_RPN("(a+b)*c","FI(a);FI(b);BOP(+);FI(c);BOP(*)");
	VERIFY_RPN("2<3","Number(2);Number(3);BOP(<)");
	VERIFY_RPN("a>b","FI(a);FI(b);BOP(>)");
	VERIFY_RPN("a>b & b>c","FI(a);FI(b);BOP(>);FI(b);FI(c);BOP(>);BOP(&)");

	// Issue #37
	VERIFY_RPN("tf2mcs(wordrange(1,2),'%d')", "Number(1);Number(2);FUNC(wordrange);Literal(%d);FUNC(tf2mcs)");

	// TODO: More here as well
	return 0;
}

int runALUUnitTests5(unsigned int onlyTest)
{
	std::cout << "\nEvaluating Expressions\n======================\n\n";

	VERIFY_EXPR_RES("5", "5");
	VERIFY_EXPR_RES("b", "84")
	VERIFY_EXPR_RES("#3", "3.14159265");
	VERIFY_EXPR_RES("-b", "-84");
	VERIFY_EXPR_RES("!b", "0");
	VERIFY_EXPR_RES("2+2", "4");
	VERIFY_EXPR_RES("!b = 0.0", "1");
	VERIFY_EXPR_RES("!b == 0.0", "0");
	VERIFY_EXPR_RES("2+2*2", "6");
	VERIFY_EXPR_RES("(2+2)*2", "8");
	VERIFY_EXPR_RES("9+sqrt(4)", "11");
	VERIFY_EXPR_RES("#3+1", "4.14159265");
	VERIFY_EXPR_RES("1.1*1.1", "1.21");
	VERIFY_EXPR_RES("pow(1.1,2)", "1.21");
	VERIFY_EXPR_RES("1/0", "NaN");
	VERIFY_EXPR_RES("(1/0)+5", "NaN");
	VERIFY_EXPR_RES("#17", "0");  // initial value of all counters

	// Issue #11: sqrt of a negative number was returning the native nan rather than the ALU NaN
	VERIFY_EXPR_RES("sqrt(81)","9");
	VERIFY_EXPR_RES("sqrt(-81)","NaN");
	VERIFY_EXPR_RES("sqrt(0)", "0");

	// Issue #10: conversions from and to binary format
	VERIFY_EXPR_RES("tobin(65)","A");
	counters.set(4,"1545406489378645");
	VERIFY_EXPR_RES("frombin(tobin(#4+3))", "1545406489378648");
	counters.set(4,"AAAA");
	VERIFY_EXPR_RES("frombin(#4)", "1094795585");
	VERIFY_EXPR_RES("tobin(1094795590)","FAAA");

	// Issue #71: tobin/tobine of large input
	VERIFY_EXPR_RES("tobin('1234567890123456789012')", "Out of range trying to convert 1234567890123456789012 to Int")

	// Possible problem with integer division
	VERIFY_EXPR_RES("37/10","3.7");
	VERIFY_EXPR_RES("37//10","3");
	VERIFY_EXPR_RES("37%10","7");

	// length and quoted string
	VERIFY_EXPR_RES("length(0)", "1");
	VERIFY_EXPR_RES("length(5)", "1");
	VERIFY_EXPR_RES("length(512)", "3");
	VERIFY_EXPR_RES("length('hello')", "5");
	VERIFY_EXPR_RES("length(hello)", "5");

	VERIFY_EXPR_RES("sqrt(4)||' by '||sqrt(16)", "2 by 4");
	return 0;
}

int runALUUnitTests6(unsigned int onlyTest)
{
	// The functions that look at the line being processed
	g_ps.setString(std::make_shared<std::string>());
	VERIFY_EXPR_RES("wordcount()", "0");
	VERIFY_EXPR_RES("word(2)", "");
	VERIFY_EXPR_RES("wordstart(3)", "0");
	VERIFY_EXPR_RES("wordend(2)", "0");
	VERIFY_EXPR_RES("wordrange(3,4)", "");

	g_ps.setString(std::make_shared<std::string>("The quick brown foox jumps over the lazy dog"));
	VERIFY_EXPR_RES("wordcount()", "9");
	VERIFY_EXPR_RES("word(2)", "quick");
	VERIFY_EXPR_RES("wordstart(3)", "11");
	VERIFY_EXPR_RES("wordend(2)", "9");
	VERIFY_EXPR_RES("wordlen(2)", "5");
	VERIFY_EXPR_RES("wordrange(3,4)", "brown foox");
	VERIFY_EXPR_RES("@@", "The quick brown foox jumps over the lazy dog");
	VERIFY_EXPR_RES("length(@@)", "44");
	VERIFY_EXPR_RES("splitw()", "The\nquick\nbrown\nfoox\njumps\nover\nthe\nlazy\ndog");
	VERIFY_EXPR_RES("splitw(,3,4)", "foox\njumps");
	VERIFY_EXPR_RES("splitw('m')", "The quick brown foox ju\nps over the lazy dog");
	VERIFY_EXPR_RES("split()", "The quick brown foox jumps over the lazy dog");
	VERIFY_EXPR_RES("split('o')", "The quick br\nwn f\n\nx jumps \nver the lazy d\ng"); 
	VERIFY_EXPR_RES("split('o',2,2)", "\nx jumps ");
	VERIFY_EXPR_RES("wordwith('o')", "brown");
	VERIFY_EXPR_RES("wordwith('oo')", "foox");
	VERIFY_EXPR_RES("wordwith('ooo')", "");
	VERIFY_EXPR_RES("wordwithidx('o')", "3");
	VERIFY_EXPR_RES("wordwithidx('oo')", "4");
	VERIFY_EXPR_RES("wordwithidx('ooo')", "0");

	g_ps.setString(std::make_shared<std::string>("The\tquick brown\tfox jumps\tover the\tlazy dog"));
	VERIFY_EXPR_RES("wordcount()", "9");
	VERIFY_EXPR_RES("wordcount(,'e')", "4");
	VERIFY_EXPR_RES("wordcount('Some sentence with a bunch    of spaces')", "7");
	VERIFY_EXPR_RES("wordcount('Some sentence with a bunch    of spaces','e')", "6");
	VERIFY_EXPR_RES("wordcount()", "9");
	VERIFY_EXPR_RES("word(2)", "quick");
	VERIFY_EXPR_RES("wordstart(3)", "11");
	VERIFY_EXPR_RES("wordend(2)", "9");
	VERIFY_EXPR_RES("wordrange(3,4)", "brown\tfox");
	VERIFY_EXPR_RES("fieldcount()", "5");
	VERIFY_EXPR_RES("fieldcount(,'e')", "4");
	VERIFY_EXPR_RES("fieldcount('Some\tsentence\twith\ta\tbunch\tof\ttabs')", "7");
	VERIFY_EXPR_RES("fieldcount('Some\tsentence\twith\ta\tbunch\tof\ttabs','b')", "3");
	VERIFY_EXPR_RES("fieldcount()", "5");
	VERIFY_EXPR_RES("field(3)", "fox jumps");
	VERIFY_EXPR_RES("fieldindex(2)", "5");
	VERIFY_EXPR_RES("fieldend(3)", "25");
	VERIFY_EXPR_RES("fieldlength(2)", "11"); // length of "quick brown"
	VERIFY_EXPR_RES("fieldrange(2,3)", "quick brown\tfox jumps");
	VERIFY_EXPR_RES("fieldwith('i')", "quick brown");
	VERIFY_EXPR_RES("fieldwithidx('i')", "2");
	VERIFY_EXPR_RES("fieldwith('oo')", "");
	VERIFY_EXPR_RES("fieldwithidx('oo')", "0");
	VERIFY_EXPR_RES("range(5,25)", "quick brown\tfox jumps");
	VERIFY_EXPR_RES("range(41,43)", "dog");
	VERIFY_EXPR_RES("range(41,45)", "dog");
	VERIFY_EXPR_RES("range(44,48)", "");
	VERIFY_EXPR_RES("@@", "The\tquick brown\tfox jumps\tover the\tlazy dog");
	VERIFY_EXPR_RES("record()", "The\tquick brown\tfox jumps\tover the\tlazy dog");
	return 0;
}

int runALUUnitTests7(unsigned int onlyTest)
{
	// time reformat
	VERIFY_EXPR_RES("tf2mcs('2019-01-03 23:23:23','%Y-%m-%d %H:%M:%S')", "1546550603000000");
	VERIFY_EXPR_RES("mcs2tf(1546550663000000,'%Y-%m-%d %H:%M:%S')", "2019-01-03 23:24:23");

	VERIFY_EXPR_RES("tf2s('2019-01-03 23:23:23','%Y-%m-%d %H:%M:%S')", "1546550603");
#if ALUFloatPrecision == 15
	VERIFY_EXPR_RES("tf2s('2019-01-03 23:23:23:123456','%Y-%m-%d %H:%M:%S:%6f')", "1546550603.12346");
#else
	VERIFY_EXPR_RES("tf2s('2019-01-03 23:23:23:123456','%Y-%m-%d %H:%M:%S:%6f')", "1546550603.123456");
#endif
	VERIFY_EXPR_RES("s2tf(1546550663.000000,'%Y-%m-%d %H:%M:%S')", "2019-01-03 23:24:23");

	// Issue #62
	VERIFY_EXPR_RES("tf2mcs('2019/10/01 07:14:01.98531','%Y/%d/%m %H:%M:%S.%6f')", "1547097241985310");    // only 5 digits in the subsecond
	VERIFY_EXPR_RES("tf2mcs('2019/10/01 07:14:01.985317','%Y/%d/%m %H:%M:%S.%6f')", "1547097241985317");   // proper 6 digits in the subsecond
	VERIFY_EXPR_RES("tf2mcs('2019/10/01 07:14:01.9853177','%Y/%d/%m %H:%M:%S.%6f')", "1547097241985317");  // 7 digits in the subsecond

	VERIFY_EXPR_RES("tf2mcs('2019/10/01 07:14:01.','%Y/%d/%m %H:%M:%S.%6f')", "1547097241000000");         // no subsecond digits at all
#ifdef PUT_TIME__SUPPORTED
	VERIFY_EXPR_RES("tf2mcs('2019/10/01 07:14:01','%Y/%d/%m %H:%M:%S.%6f')", "1547097241000000");                 // no subsecond digits and a missing dot!
#else
	VERIFY_EXPR_RES("tf2mcs('2019/10/01 07:14:01','%Y/%d/%m %H:%M:%S.%6f')", "0");                          // no subsecond digits and a missing dot!
#endif
	VERIFY_EXPR_RES("tf2mcs('2019/10/01 07:14:0153','%Y/%d/%m %H:%M:%S.%6f')", "0");                        // This is just weird

	// Issue #38
	VERIFY_EXPR_RES("2-2", "0");
	return 0;
}

int runALUUnitTests8(unsigned int onlyTest)
{
	// String processing functions
	counters.set(9,"Magna Carta");
	VERIFY_EXPR_RES("substr(#9,2,4)", "agna");
	VERIFY_EXPR_RES("substr(#9,5,-1)", "a Carta");
	VERIFY_EXPR_RES("substr(#9,-3,-1)", "rta");
	VERIFY_EXPR_RES("substr(#9,2,-3)", "agna Cart");
	VERIFY_EXPR_RES("substr(#9,-3,999)", "rta");
	VERIFY_EXPR_RES("substr(#9,11,2)", "a");
	VERIFY_EXPR_RES("substr(#9,12,5)", "");
	VERIFY_EXPR_RES("substr(#9,-13,5)", "");
	VERIFY_EXPR_RES("substr(#9,0,3)", "NaN");
	VERIFY_EXPR_RES("substr(#9,3,0)", "");

	VERIFY_EXPR_RES("left(#9,0)", "");
	VERIFY_EXPR_RES("left(#9,1)", "M");
	VERIFY_EXPR_RES("left(#9,7)", "Magna C");
	VERIFY_EXPR_RES("left(#9,11)", "Magna Carta");
	VERIFY_EXPR_RES("left(#9,15)", "Magna Carta    ");
	VERIFY_EXPR_RES("left(#9,-1)", "Magna Carta");
	VERIFY_EXPR_RES("left(#9,-3)", "Magna Car");

	VERIFY_EXPR_RES("right(#9,0)", "");
	VERIFY_EXPR_RES("right(#9,1)", "a");
	VERIFY_EXPR_RES("right(#9,7)", "a Carta");
	VERIFY_EXPR_RES("right(#9,11)", "Magna Carta");
	VERIFY_EXPR_RES("right(#9,15)", "    Magna Carta");
	VERIFY_EXPR_RES("right(#9,-1)", "Magna Carta");
	VERIFY_EXPR_RES("right(#9,-3)", "gna Carta");

	VERIFY_EXPR_RES("center(#9,0)", "");
	VERIFY_EXPR_RES("center(#9,1)", " ");
	VERIFY_EXPR_RES("center(#9,7)", "gna Car");
	VERIFY_EXPR_RES("center(#9,6)", "gna Ca");
	VERIFY_EXPR_RES("center(#9,11)", "Magna Carta");
	VERIFY_EXPR_RES("center(#9,15)", "  Magna Carta  ");
	VERIFY_EXPR_RES("center(#9,-1)", "Magna Carta");
	VERIFY_EXPR_RES("center(#9,-3)", "agna Cart");

	VERIFY_EXPR_RES("pos('g', #9)", "3");
	VERIFY_EXPR_RES("pos('a', #9)", "2");
	VERIFY_EXPR_RES("pos('x', #9)", "0");
	VERIFY_EXPR_RES("pos('a ', #9)", "5");
	VERIFY_EXPR_RES("pos('a ', left(#9,13))", "5");
	VERIFY_EXPR_RES("pos('a')", "37");

	VERIFY_EXPR_RES("lastpos('g', #9)", "3");
	VERIFY_EXPR_RES("lastpos('a', #9)", "11");
	VERIFY_EXPR_RES("lastpos('x', #9)", "0");
	VERIFY_EXPR_RES("lastpos('a ', #9)", "5");
	VERIFY_EXPR_RES("lastpos('a ', left(#9,13))", "11");
	VERIFY_EXPR_RES("lastpos('e')", "34");

	VERIFY_EXPR_RES("includes(#9, 'a')", "1");
	VERIFY_EXPR_RES("includes(#9, 'x')", "0");
	VERIFY_EXPR_RES("includes(#9, 'gn')", "1");
	VERIFY_EXPR_RES("includes(#9, 'rt ')", "0");
	VERIFY_EXPR_RES("includes(#9, 'a', 'x')", "1");
	VERIFY_EXPR_RES("includes(#9, 'x', 'a')", "1");
	VERIFY_EXPR_RES("includes(#9, 'gn', 'a')", "1");
	VERIFY_EXPR_RES("includes(#9, 'x', 'rt ')", "0");
	VERIFY_EXPR_RES("includes(,'quick','fox')", "1");
	VERIFY_EXPR_RES("includes(,'fast','fox')", "1");
	VERIFY_EXPR_RES("includes(,'quick','goose')", "1");
	VERIFY_EXPR_RES("includes(,'fast','goose')", "0");

	VERIFY_EXPR_RES("includesall(#9, 'a')", "1");
	VERIFY_EXPR_RES("includesall(#9, 'x')", "0");
	VERIFY_EXPR_RES("includesall(#9, 'gn')", "1");
	VERIFY_EXPR_RES("includesall(#9, 'rt ')", "0");
	VERIFY_EXPR_RES("includesall(#9, 'a', 'x')", "0");
	VERIFY_EXPR_RES("includesall(#9, 'x', 'a')", "0");
	VERIFY_EXPR_RES("includesall(#9, 'gn', 'a')", "1");
	VERIFY_EXPR_RES("includesall(#9, 'x', 'rt ')", "0");
	VERIFY_EXPR_RES("includesall(,'quick','fox')", "1");
	VERIFY_EXPR_RES("includesall(,'fast','fox')", "0");
	VERIFY_EXPR_RES("includesall(,'quick','goose')", "0");
	VERIFY_EXPR_RES("includesall(,'fast','goose')", "0");

	VERIFY_EXPR_RES("#4:=5","5");  // Issue #48: an assignment returns the counter value
	return 0;
}

int runALUUnitTests9(unsigned int onlyTest)
{
	// Conversion-equivalent functions
	VERIFY_EXPR_RES("x2d('A',0)", "10");
	VERIFY_EXPR_RES("x2d('A',1)", "-6");
	VERIFY_EXPR_RES("x2d('A',2)", "-6");
	VERIFY_EXPR_RES("x2d('1234567890abcdef',0)", "1311768467294899695");
	VERIFY_EXPR_RES("x2d('1234567890abcdef',4)", "4660");
	VERIFY_EXPR_RES("x2d('fedcba',4)", "-292");
	VERIFY_EXPR_RES("x2d('1234567890abCdEf',0)", "1311768467294899695");
	VERIFY_EXPR_RES("x2d('1234567890abcdefg',0)", "1311768467294899695");
	VERIFY_EXPR_RES("x2d('1234567890abcdeff',0)", "Cannot convert <1234567890abcdeff> from format <Hex> to format <Decimal>: out of range");

	VERIFY_EXPR_RES("d2x('153')", "99");
	VERIFY_EXPR_RES("d2x('18446744073709551615')", "ffffffffffffffff");
	VERIFY_EXPR_RES("d2x('18446744073709551616')", "Cannot convert <18446744073709551616> from format <Decimal> to format <Hex>: out of range");
	VERIFY_EXPR_RES("d2x('18446744073709551616a')", "Cannot convert <18446744073709551616a> from format <Decimal> to format <Hex>: out of range");
	VERIFY_EXPR_RES("d2x('184467440737095516166')", "Cannot convert <184467440737095516166> from format <Decimal> to format <Hex>: out of range");
	VERIFY_EXPR_RES("d2x('-8')", "fffffffffffffff8");

	VERIFY_EXPR_RES("c2x('A')", "41");
	VERIFY_EXPR_RES("c2x('AbC.')", "4162432e");
	VERIFY_EXPR_RES("c2x('')", "");
	VERIFY_EXPR_RES("c2x('hello\tthere')", "68656c6c6f097468657265");

	VERIFY_EXPR_RES("x2ch('61')", "a");
	VERIFY_EXPR_RES("x2ch('4e43432d31373031')", "NCC-1701");
	VERIFY_EXPR_RES("x2ch('')", "");
	VERIFY_EXPR_RES("x2ch('616')", "Cannot convert <616> from format <Hex> to format <Char>");
	VERIFY_EXPR_RES("x2ch('616g')", "Cannot convert <616g> from format <Hex> to format <Char>");
	VERIFY_EXPR_RES("x2ch('6161g')", "Cannot convert <6161g> from format <Hex> to format <Char>");

	VERIFY_EXPR_RES("ucase('a')", "A");
	VERIFY_EXPR_RES("ucase('A')", "A");
	VERIFY_EXPR_RES("ucase('7-')", "7-");
	VERIFY_EXPR_RES("ucase('UpPeRcAsE')", "UPPERCASE");
	VERIFY_EXPR_RES("ucase('')", "");

	VERIFY_EXPR_RES("lcase('a')", "a");
	VERIFY_EXPR_RES("lcase('A')", "a");
	VERIFY_EXPR_RES("lcase('7-')", "7-");
	VERIFY_EXPR_RES("lcase('LoWeRcAsE')", "lowercase");
	VERIFY_EXPR_RES("lcase('')", "");

	VERIFY_EXPR_RES("bswap('a')", "a");
	VERIFY_EXPR_RES("bswap('A')", "A");
	VERIFY_EXPR_RES("bswap('7-')", "-7");
	VERIFY_EXPR_RES("c2x(bswap(x2ch('c0a80102')))", "0201a8c0")
	VERIFY_EXPR_RES("bswap('LoWeRcAsE')", "EsAcReWoL");
	VERIFY_EXPR_RES("bswap('')", "");
	return 0;
}

int runALUUnitTests10(unsigned int onlyTest)
{
	// rounding functions
	VERIFY_EXPR_RES("floor(3.4)", "3");
	VERIFY_EXPR_RES("floor(-3.4)", "-4");
	VERIFY_EXPR_RES("ceil(3.4)", "4");
	VERIFY_EXPR_RES("ceil(-3.4)", "-3");
	VERIFY_EXPR_RES("round(3.4)", "3");
	VERIFY_EXPR_RES("round(-3.4)", "-3");
	VERIFY_EXPR_RES("round(3.14159265, 0)", "3");
	VERIFY_EXPR_RES("round(3.14159265, 1)", "3.1");
	VERIFY_EXPR_RES("round(3.14159265, 2)", "3.14");
	VERIFY_EXPR_RES("round(3.14159265, 3)", "3.142");
	VERIFY_EXPR_RES("round(3.14159265, 4)", "3.1416");

	// trig
	VERIFY_EXPR_RES("round(sin(\"0.523598775\"),8)", "0.5");
	VERIFY_EXPR_RES("round(sin(3.14159265),8)", "0");
	VERIFY_EXPR_RES("round(cos(\"0.523598775\"),8)", "0.8660254");
	VERIFY_EXPR_RES("round(cos(3.14159265),8)", "-1");
	VERIFY_EXPR_RES("round(tan(\"0.523598775\"),8)", "0.57735027");
	VERIFY_EXPR_RES("round(tan(3.14159265),8)", "-0");
	VERIFY_EXPR_RES("round(arcsin(0.5),8)", "0.52359878");
	VERIFY_EXPR_RES("round(arcsin(-0.2),8)", "-0.20135792");
	VERIFY_EXPR_RES("round(arccos(0.5),8)", "1.04719755");
	VERIFY_EXPR_RES("round(arccos(-0.2),8)", "1.77215425");
	VERIFY_EXPR_RES("round(arctan(0.5),8)", "0.46364761");
	VERIFY_EXPR_RES("round(arctan(-0.2),8)", "-0.19739556");

	VERIFY_EXPR_RES("round(dsin(30),8)", "0.5");
	VERIFY_EXPR_RES("round(dsin(180),8)", "-0");
	VERIFY_EXPR_RES("round(dcos(30),8)", "0.8660254");
	VERIFY_EXPR_RES("round(dcos(180),8)", "-1");
	VERIFY_EXPR_RES("round(dtan(30),8)", "0.57735027");
	VERIFY_EXPR_RES("round(dtan(180),8)", "0");
	VERIFY_EXPR_RES("round(arcdsin(0.5),8)", "30");
	VERIFY_EXPR_RES("round(arcdsin(-0.2),8)", "-11.53695903");
	VERIFY_EXPR_RES("round(arcdcos(0.5),8)", "60");
	VERIFY_EXPR_RES("round(arcdcos(-0.2),8)", "101.53695903");
	VERIFY_EXPR_RES("round(arcdtan(0.5),8)", "26.56505118");
	VERIFY_EXPR_RES("round(arcdtan(-0.2),8)", "-11.30993247");

	VERIFY_EXPR_RES("exp(0)", "1");
	VERIFY_EXPR_RES("floor(exp(3))", "20");
	VERIFY_EXPR_RES("log(1)", "0");
	VERIFY_EXPR_RES("round(log(20,10)*10)","13");
	VERIFY_EXPR_RES("log(100,10)","2");

	VERIFY_EXPR_RES("fact(6)","720");
	VERIFY_EXPR_RES("fact(2)","2");
	VERIFY_EXPR_RES("fact(1)","1");
	VERIFY_EXPR_RES("fact(0)","1");
	VERIFY_EXPR_RES("fact(-5)","1");
	VERIFY_EXPR_RES("fact(3.14)","6");

	VERIFY_EXPR_RES("permutations(6,4)","360");  // 6*5*4*3
	VERIFY_EXPR_RES("permutations(6,2)","30");
	VERIFY_EXPR_RES("permutations(6,1)","6");
	VERIFY_EXPR_RES("permutations(6,0)","permutations: k must be greater than zero");
	VERIFY_EXPR_RES("permutations(6,8)","permutations: k must be no greater than n");

	VERIFY_EXPR_RES("combinations(6,4)","15");  // 6*5*4*3 / 4*3*2*1
	VERIFY_EXPR_RES("combinations(6,2)","15");
	VERIFY_EXPR_RES("combinations(6,1)","6");
	VERIFY_EXPR_RES("combinations(6,0)","combinations: k must be greater than zero");
	VERIFY_EXPR_RES("combinations(6,8)","combinations: k must be no greater than n");

	VERIFY_EXPR_RES("string(3.0000)", "3.0000");
	VERIFY_EXPR_RES("string(3.0000+2.0000)", "5");

	VERIFY_EXPR_RES("c2u('A')", "65");
	VERIFY_EXPR_RES("c2u('AA')", "16705");
	VERIFY_EXPR_RES("c2u('AAA')", "4276545");
	VERIFY_EXPR_RES("c2u('AAAA')", "1094795585");
	VERIFY_EXPR_RES("c2u('AAAAA')", "280267669825");
	VERIFY_EXPR_RES("c2u('AAAAAA')", "71748523475265");
	VERIFY_EXPR_RES("c2u('AAAAAAA')", "18367622009667905");
	VERIFY_EXPR_RES("c2u('AAAAAAAA')", "4702111234474983745");
	VERIFY_EXPR_RES("c2u('AAAAAAAAA')", "c2u/c2d: Invalid input length: 9");
	VERIFY_EXPR_RES("c2d('ג')", "-27945");
	VERIFY_EXPR_RES("c2d('גג')", "-1831365929");
	VERIFY_EXPR_RES("c2d('גגג')", "161454579225303");
	VERIFY_EXPR_RES("c2d('גגגג')", "-7865656769600056617");
	VERIFY_EXPR_RES("c2d('גגגגג')", "c2u/c2d: Invalid input length: 10");
	return 0;
}

int runALUUnitTests11(unsigned int onlyTest)
{
#if ALUFloatPrecision == 15
#define C2FERR ". Supported lengths: 4, 8"
#else
#define C2FERR ". Supported lengths: 4, 8, 16"
#endif

	VERIFY_EXPR_RES("c2f('A')", "c2f: Invalid floating point length: 1" C2FERR)
	VERIFY_EXPR_RES("c2f('AA')", "c2f: Invalid floating point length: 2" C2FERR)
	VERIFY_EXPR_RES("c2f('AAA')", "c2f: Invalid floating point length: 3" C2FERR)
#if ALUFloatPrecision == 15
	VERIFY_EXPR_RES("c2f('AAAA')", "12.0784311294556");
#elif ALUFloatPrecision == 16
	VERIFY_EXPR_RES("c2f('AAAA')", "12.07843112945557");
#elif ALUFloatPrecision == 18
	VERIFY_EXPR_RES("c2f('AAAA')", "12.0784311294555664");
#elif ALUFloatPrecision == 33
	VERIFY_EXPR_RES("c2f('AAAA')", "12.07843112945556640625");
#else
#warning "Dropping test case because of unsupported precision"
#endif
	VERIFY_EXPR_RES("c2f('AAAAA')", "c2f: Invalid floating point length: 5" C2FERR)
	VERIFY_EXPR_RES("c2f('AAAAAA')", "c2f: Invalid floating point length: 6" C2FERR)
	VERIFY_EXPR_RES("c2f('AAAAAAA')", "c2f: Invalid floating point length: 7" C2FERR)
#if ALUFloatPrecision == 15
	VERIFY_EXPR_RES("c2f('AAAAAAAA')", "2261634.50980392");
#elif ALUFloatPrecision == 16
	VERIFY_EXPR_RES("c2f('AAAAAAAA')", "2261634.509803921");
#elif ALUFloatPrecision == 18
	VERIFY_EXPR_RES("c2f('AAAAAAAA')", "2261634.50980392145");
#elif ALUFloatPrecision == 33
	VERIFY_EXPR_RES("c2f('AAAAAAAA')", "2261634.50980392144992947578430176");
#else
#warning "Dropping test case because of unsupported precision"
#endif
	VERIFY_EXPR_RES("c2f('AAAAAAAAA')", "c2f: Invalid floating point length: 9" C2FERR)
	VERIFY_EXPR_RES("c2f('AAAAAAAAAA')", "c2f: Invalid floating point length: 10" C2FERR)
	VERIFY_EXPR_RES("c2f('AAAAAAAAAAA')", "c2f: Invalid floating point length: 11" C2FERR)
	VERIFY_EXPR_RES("c2f('AAAAAAAAAAAA')", "c2f: Invalid floating point length: 12" C2FERR)
	VERIFY_EXPR_RES("c2f('AAAAAAAAAAAAA')", "c2f: Invalid floating point length: 13" C2FERR)
	VERIFY_EXPR_RES("c2f('AAAAAAAAAAAAAA')", "c2f: Invalid floating point length: 14" C2FERR)
	VERIFY_EXPR_RES("c2f('AAAAAAAAAAAAAAA')", "c2f: Invalid floating point length: 15" C2FERR)
#if ALUFloatPrecision == 15
	VERIFY_EXPR_RES("c2f('אאאאAAAAAAAA')", "c2f: Invalid floating point length: 16" C2FERR)
#elif ALUFloatPrecision == 16
	VERIFY_EXPR_RES("c2f('אאאאAAAAAAAA')", "9.668148415950124e+96");
#elif ALUFloatPrecision == 18
	VERIFY_EXPR_RES("c2f('אאאאAAAAAAAA')", "9.66814841595012435e+96");
#elif ALUFloatPrecision == 33
	VERIFY_EXPR_RES("c2f('אאאאAAAAAAAA')", "1.072181727834810710522875562594e+97");
#else
#warning "Dropping test case because of unsupported precision"
#endif

	VERIFY_EXPR_RES("substitute('Just the place for a snark','','',1)", "substitute: Search string must not be empty");
	VERIFY_EXPR_RES("substitute('Just the place for a snark',' ','',0)", "Just the place for a snark");
	VERIFY_EXPR_RES("substitute('Just the place for a snark',' ','',1)", "Justthe place for a snark");
	VERIFY_EXPR_RES("substitute('Just the place for a snark',' ','',2)", "Justtheplace for a snark");
	VERIFY_EXPR_RES("substitute('Just the place for a snark',' ','','u')", "Just the place for a snark");
	VERIFY_EXPR_RES("substitute('Just the place for a snark',' ','','U')", "Justtheplaceforasnark");
	VERIFY_EXPR_RES("substitute('Just the place for a snark',' ','_','U')", "Just_the_place_for_a_snark");

	VERIFY_EXPR_RES("sfield('Where hae\tya been',0,'')","sfield: Called with count equal to zero");
	VERIFY_EXPR_RES("sfield('Where hae\tya been',1,'')","Where hae");
	VERIFY_EXPR_RES("sfield('Where hae\tya been',2,'')","ya been");
	VERIFY_EXPR_RES("sfield('Where hae\tya been',3,'')","");
	VERIFY_EXPR_RES("sfield('\tWhere hae\tya been\t',-1,'')","");
	VERIFY_EXPR_RES("sfield('\tWhere hae\tya been\t',-2,'')","ya been");

	VERIFY_EXPR_RES("lvalue('pi=3.14')","pi");
	VERIFY_EXPR_RES("lvalue('pi=3.14=tau/2')","pi");
	VERIFY_EXPR_RES("lvalue('pi=3.14',':')","pi=3.14");
	VERIFY_EXPR_RES("lvalue('pi:3.14',':')","pi");
	VERIFY_EXPR_RES("lvalue('pi: 3.14',':')","pi");
	VERIFY_EXPR_RES("lvalue('pi :3.14',':')","pi");

	VERIFY_EXPR_RES("rvalue('pi=3.14')","3.14");
	VERIFY_EXPR_RES("rvalue('pi=3.14=tau/2')","3.14");
	VERIFY_EXPR_RES("rvalue('pi=3.14',':')","");
	VERIFY_EXPR_RES("rvalue('pi:3.14',':')","3.14");
	VERIFY_EXPR_RES("rvalue('pi: 3.14',':')","3.14");
	VERIFY_EXPR_RES("rvalue('pi :3.14',':')","3.14");


	VERIFY_EXPR_RES("sword('Where hae\tya been',0,'')","sword: Called with count equal to zero");
	VERIFY_EXPR_RES("sword('Where  hae\tya been',1,'')","Where");
	VERIFY_EXPR_RES("sword('   Where hae\tya been',2,'')","hae\tya");
	VERIFY_EXPR_RES("sword('Where hae\tya been',3,'')","been");
	VERIFY_EXPR_RES("sword('\tWhere hae\tya been\t',-1,'')","been\t");
	VERIFY_EXPR_RES("sword('\tWhere hae\tya been\t     ',-2,'')","hae\tya");

	VERIFY_EXPR_RES("abbrev('information','info')", "1");
	VERIFY_EXPR_RES("abbrev('information','infot')", "0");
	VERIFY_EXPR_RES("abbrev('information','infota',4)", "1");
	VERIFY_EXPR_RES("abbrev('information','infota',5)", "0");
	VERIFY_EXPR_RES("abbrev('information','info',6)", "1");

	VERIFY_EXPR_RES("bitand('information','info')", "info");
	VERIFY_EXPR_RES("bitand('info','infn')", "infn");
	VERIFY_EXPR_RES("bitand('info','infp')", "inf`");
	VERIFY_EXPR_RES("bitor('info','info')", "info");
	VERIFY_EXPR_RES("bitor('infq','infr')", "infs");
	VERIFY_EXPR_RES("bitxor('!','B')", "c");

	VERIFY_EXPR_RES("compare('Hello','Hello')", "0");
	VERIFY_EXPR_RES("compare('Hello','Hellp')", "5");
	VERIFY_EXPR_RES("compare('Hello','hello')", "1");
	VERIFY_EXPR_RES("compare('Hello','He')", "3");
	VERIFY_EXPR_RES("compare('Hello','Hellox')", "6");
	VERIFY_EXPR_RES("compare('Hello','Hello ')", "0");
	VERIFY_EXPR_RES("compare('Hello', 'He', 'x')", "3");
	VERIFY_EXPR_RES("compare('Hello', 'He', 'l')", "5");
	return 0;
}

int runALUUnitTests12(unsigned int onlyTest)
{
	VERIFY_EXPR_RES("copies('Hello',3)", "HelloHelloHello");
	VERIFY_EXPR_RES("copies('Hello',0)", "");
	VERIFY_EXPR_RES("copies('Hello',-1)", "copies: Second argument should be a non-negative integer. Got: -1");

	VERIFY_EXPR_RES("delstr('Hello',3,2)", "Heo");
	VERIFY_EXPR_RES("delstr('Hello',3,0)", "He");
	VERIFY_EXPR_RES("delstr('Hello',-3,2)", "llo");
	VERIFY_EXPR_RES("delstr('Hello',13,2)", "Hello");
	VERIFY_EXPR_RES("delstr('Hello',3,-2)", "He");
	VERIFY_EXPR_RES("delstr('Hello',3,12)", "He");

	VERIFY_EXPR_RES("delword('Roses are red',2,0)", "Roses");
	VERIFY_EXPR_RES("delword('Roses are red',2,1)", "Roses red");
	VERIFY_EXPR_RES("delword('Roses are red',-2,1)", "are red");
	VERIFY_EXPR_RES("delword('Roses are red',12,1)", "Roses are red");
	VERIFY_EXPR_RES("delword('Roses are red',2,11)", "Roses");

	VERIFY_EXPR_RES("find('now is the time', 'the')", "3");
	VERIFY_EXPR_RES("find('now is the time', 'xxx')", "0");
	VERIFY_EXPR_RES("find('now  is   the   time', 'the time')", "3");

	VERIFY_EXPR_RES("index('How much wood would a wood-chuck chuck', 'wood', 0)", "10");
	VERIFY_EXPR_RES("index('How much wood would a wood-chuck chuck', 'wood', 1)", "10");
	VERIFY_EXPR_RES("index('How much wood would a wood-chuck chuck', 'wood', 2)", "10");
	VERIFY_EXPR_RES("index('How much wood would a wood-chuck chuck', 'wood', -2)", "10");
	VERIFY_EXPR_RES("index('How much wood would a wood-chuck chuck', 'wood', 15)", "23");
	VERIFY_EXPR_RES("index('How much wood would a wood-chuck chuck', 'wood', 23)", "23");
	VERIFY_EXPR_RES("index('How much wood would a wood-chuck chuck', 'wood', 24)", "0");

	VERIFY_EXPR_RES("insert('xy','hello',0,0)", "xyhello");
	VERIFY_EXPR_RES("insert('xy','hello',1,0)", "hxyello");
	VERIFY_EXPR_RES("insert('xy','hello',0,1)", "xhello");
	VERIFY_EXPR_RES("insert('xy','hello',0,4)", "xy  hello");
	VERIFY_EXPR_RES("insert('xy','hello',4,4)", "hellxy  o");
	VERIFY_EXPR_RES("insert('xy','hello',5,0)", "helloxy");
	VERIFY_EXPR_RES("insert('xy','hello',6,0)", "helloxy");
	VERIFY_EXPR_RES("insert('xy','hello',-1,0)", "insert: Invalid negative position value: -1");
	VERIFY_EXPR_RES("insert('xy','hello',0,-1)", "insert: Invalid negative length value: -1");
	VERIFY_EXPR_RES("insert('xy','hello',0,1,'q')", "xhello");
	VERIFY_EXPR_RES("insert('xy','hello',0,4,'q')", "xyqqhello");
	VERIFY_EXPR_RES("insert('xy','hello',0,4,'')", "insert: Invalid pad argument: <>");
	VERIFY_EXPR_RES("insert('xy','hello',0,4,'qq')", "insert: Invalid pad argument: <qq>");

	VERIFY_EXPR_RES("justify('this is it',18)", "this     is     it");
	VERIFY_EXPR_RES("justify('this is it',19)", "this     is      it");
	VERIFY_EXPR_RES("justify('this is it',9)", "thisis it");
	VERIFY_EXPR_RES("justify('this is it',3)", "thi");
	VERIFY_EXPR_RES("justify('this is it',1)", "t");
	VERIFY_EXPR_RES("justify('this is it',0)", "");
	VERIFY_EXPR_RES("justify('this is it',-1)", "justify: len argument should be non-negative. Got -1");
	VERIFY_EXPR_RES("justify('this is it',18,'x')", "thisxxxxxisxxxxxit");
	VERIFY_EXPR_RES("justify('     this is it',18,'x')", "thisxxxxxisxxxxxit");
	VERIFY_EXPR_RES("justify('this is it',18,'')", "justify: Invalid pad argument: <>");
	VERIFY_EXPR_RES("justify('this is it',18,'qq')", "justify: Invalid pad argument: <qq>");

	VERIFY_EXPR_RES("overlay('not','this is really right',9,6,'.')", "this is not... right");
	VERIFY_EXPR_RES("overlay('eous','this is right',14,0,' ')", "this is righteous");
	VERIFY_EXPR_RES("overlay('eous','this is right',16,0,'X')", "this is rightXXeous");
	VERIFY_EXPR_RES("overlay('eous','this is right',16,2,'X')", "this is rightXXeo");
	VERIFY_EXPR_RES("overlay('eous','this is right',16,6,'X')", "this is rightXXeousXX");

	VERIFY_EXPR_RES("reverse('')", "");
	VERIFY_EXPR_RES("reverse('1')", "1");
	VERIFY_EXPR_RES("reverse('12')", "21");
	VERIFY_EXPR_RES("reverse('bird')", "drib");
	VERIFY_EXPR_RES("reverse('chuck')", "kcuhc");
	return 0;
}

int runALUUnitTests13(unsigned int onlyTest)
{
	VERIFY_EXPR_RES("sign(-88)", "-1");
	VERIFY_EXPR_RES("sign(88)", "1");
	VERIFY_EXPR_RES("sign(+0)", "0");
	VERIFY_EXPR_RES("sign(0)", "0");
	VERIFY_EXPR_RES("sign('hello')", "0");

	VERIFY_EXPR_RES("strip('    abc  ','B',' ')", "abc");
	VERIFY_EXPR_RES("strip('xxxabcxxx', 'B', 'x')", "abc");
	VERIFY_EXPR_RES("strip('xxxabcxxx', , 'x')", "abc");
	VERIFY_EXPR_RES("strip('xxxabcxxx', 't', 'x')", "xxxabc");
	VERIFY_EXPR_RES("strip(' \t abc \t ')", "abc");
	VERIFY_EXPR_RES("strip('everywhere', , 'aeiou')", "verywher");

	VERIFY_EXPR_RES("space('abc   abc','1',' ')", "abc abc");
	VERIFY_EXPR_RES("space('abc   abc','1','x')", "abcxabc");
	VERIFY_EXPR_RES("space('abc   abc','0',' ')", "abcabc");

	VERIFY_EXPR_RES("subword('There are  those who believe',0,2)", "There are");
	VERIFY_EXPR_RES("subword('There are  those who believe',1,2)", "There are");
	VERIFY_EXPR_RES("subword('There are  those who believe',2,2)", "are  those");
	VERIFY_EXPR_RES("subword('  There are  those who believe',0,2)", "  There are");
	VERIFY_EXPR_RES("subword('  There are  those who believe',1,2)", "There are");
	VERIFY_EXPR_RES("subword('  There are  those who believe',2,2)", "are  those");
	VERIFY_EXPR_RES("subword('There are  those who believe',2,4)", "are  those who believe");
	VERIFY_EXPR_RES("subword('There are  those who believe',2,5)", "are  those who believe");
	VERIFY_EXPR_RES("subword('There are  those who believe  ',2,4)", "are  those who believe");
	VERIFY_EXPR_RES("subword('There are  those who believe  ',2,5)", "are  those who believe  ");
	VERIFY_EXPR_RES("subword('There are  those who believe  ',2,0)", "are  those who believe  ");

	VERIFY_EXPR_RES("translate('abc')", "ABC");
	VERIFY_EXPR_RES("translate('abc','','','')", "ABC");
	VERIFY_EXPR_RES("translate('abc','xy','ab','$')", "xyc");
	VERIFY_EXPR_RES("translate('abc','xy','a','$')", "xbc");
	VERIFY_EXPR_RES("translate('abc','x','ab','$')", "x$c");
	VERIFY_EXPR_RES("translate('abc','xyz','ab','$')", "xyc");

	VERIFY_EXPR_RES("verify('ab12deadbeef8','abcdefgh')", "3");
	VERIFY_EXPR_RES("verify('dg','abcdefgh')", "0");
	VERIFY_EXPR_RES("verify('dg','abcdefgh','m')", "1");
	VERIFY_EXPR_RES("verify('ab12deadbeef8','abcdefgh',,5)", "13");

	VERIFY_EXPR_RES("wordindex('tis the time', 2)", "5");
	VERIFY_EXPR_RES("wordindex('tis the time', 4)", "0");
	VERIFY_EXPR_RES("wordindex('tis the time', 0)", "wordindex: wordno argument must be positive. Got: 0");

	VERIFY_EXPR_RES("wordlength('tis the time', 2)", "3");
	VERIFY_EXPR_RES("wordlength('tis the time', 4)", "0");
	VERIFY_EXPR_RES("wordlength('tis the time', 0)", "wordlength: wordno argument must be positive. Got: 0");

	VERIFY_EXPR_RES("wordpos('time of', 'tis the time of the season', 1)", "3");
	VERIFY_EXPR_RES("wordpos(' time of', 'tis the time of the season', 1)", "0");
	VERIFY_EXPR_RES("wordpos('time of ', 'tis the time of the season', 1)", "3");
	VERIFY_EXPR_RES("wordpos('never', 'tis the time of the season', 1)", "0");

	VERIFY_EXPR_RES("words('tis the time of the season for love')", "8");
	VERIFY_EXPR_RES("words('tis   the   time  of    the season   for  love')", "8");
	VERIFY_EXPR_RES("words('')", "0");

	VERIFY_EXPR_RES("xrange('a','d')", "abcd");
	VERIFY_EXPR_RES("length(xrange('a',''))", "159");
	VERIFY_EXPR_RES("substr(xrange('a',''),1,29)", "abcdefghijklmnopqrstuvwxyz{|}");

	// Issue #94
	VERIFY_EXPR_RES("pow(2+3,4)", "625");
	VERIFY_EXPR_RES("subword(substr('There are those who believe',3-2,4+4*4),(4-2)/(1+1),2)","There are");
	return 0;
}

int runALUUnitTests14(unsigned int onlyTest)
{
	tg.set('p', "10003.14159265359");
	tg.set('q', "1234567890");
	VERIFY_EXPR_RES("fmt(p)","10003.1");
	VERIFY_EXPR_RES("fmt(p,,12)","10003.1415927");
	VERIFY_EXPR_RES("fmt(p,'f',4)","10003.1416");
	VERIFY_EXPR_RES("fmt(p,'s')","1.000314e+04");
	VERIFY_EXPR_RES("fmt(p,'s',12)","1.000314159265e+04");
	VERIFY_EXPR_RES("fmt(p,,12,'$')","10003$1415927");
	VERIFY_EXPR_RES("fmt(p,,12,,'$')","10$003.1415927");
	VERIFY_EXPR_RES("pretty(p)","10,003.141593");
#ifdef SPANISH_LOCALE_SUPPORTED
	VERIFY_EXPR_RES("pretty(p,,,'de_DE')","10003,141593");
	VERIFY_EXPR_RES("pretty(q,,,'de_DE')","1234567890");
	VERIFY_EXPR_RES("pretty(p,,,'ru_RU')","10 003,141593");
	VERIFY_EXPR_RES("pretty(q,,,'ru_RU')","1 234 567 890");
#else
	VERIFY_EXPR_RES("pretty(p,,,'de_DE')","Invalid locale <de_DE> passed to pretty function");
	VERIFY_EXPR_RES("pretty(q,,,'de_DE')","Invalid locale <de_DE> passed to pretty function");
	VERIFY_EXPR_RES("pretty(p,,,'ru_RU')","Invalid locale <ru_RU> passed to pretty function");
	VERIFY_EXPR_RES("pretty(q,,,'ru_RU')","Invalid locale <ru_RU> passed to pretty function");
#endif
	VERIFY_EXPR_RES("pretty(p,5)","1.000314e+04");
	VERIFY_EXPR_RES("pretty(q)","1,234,567,890");
	VERIFY_EXPR_RES("pretty(q,,5)","1.234568e+09");

	/* regular expressions */
	disableRegexCache();
	tg.set('s', "There is a subsequence in the string");
	VERIFY_EXPR_RES("rmatch('subject','(sub)(.*)')", "1");
	VERIFY_EXPR_RES("rmatch('object','(sub)(.*)')", "0");
	VERIFY_EXPR_RES("rmatch('nonsubjective','(sub)(.*)')", "0");
	VERIFY_EXPR_RES("rmatch(,'.*brown.*')", "1");
	VERIFY_EXPR_RES("rmatch(,'.*black.*')", "0");
#ifdef ADVANCED_REGEX_FUNCTIONS
	VERIFY_EXPR_RES("rsearch('subject','(sub)(.*)')", "1");
	VERIFY_EXPR_RES("rsearch('object','(sub)(.*)')", "0");
	VERIFY_EXPR_RES("rsearch('nonsubjective','(sub)(.*)')", "1");
	VERIFY_EXPR_RES("rsearch(,'brown')", "1");
	VERIFY_EXPR_RES("rsearch(,'black')", "0");
	VERIFY_EXPR_RES("rreplace(s,'\\\\b(sub)([^ ]*)','sub-$2')","There is a sub-sequence in the string");
	VERIFY_EXPR_RES("rreplace(s,'\\\\b(sub)([^ ]*)','$2')","There is a sequence in the string");

	/* regular expression variations */
	tg.set('t', "It's just a jump to the left\nAnd then a step to the right\n");
	setRegexType("");
	VERIFY_EXPR_RES("rsearch(t,'JUMP')", "0");
	setRegexType("icase");
	VERIFY_EXPR_RES("rsearch(t,'JUMP')", "1");

	tg.set('z', "zzxayyzz");
#ifdef REGEX_GRAMMARS
	setRegexType("");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Oyyzz");
	setRegexType("basic");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
	setRegexType("extended");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Ozz");
	setRegexType("awk");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Ozz");
	setRegexType("grep");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
	setRegexType("egrep");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Ozz");
#else
#ifdef VISUAL_STUDIO
	setRegexType("");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Ozz");
	setRegexType("basic");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
	setRegexType("extended");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
	setRegexType("awk");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
	setRegexType("grep");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
	setRegexType("egrep");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
#else
	setRegexType("");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Oyyzz");
	setRegexType("basic");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
	setRegexType("extended");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Oyyzz");
	setRegexType("awk");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Oyyzz");
	setRegexType("grep");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "zzxayyzz");
	setRegexType("egrep");
	VERIFY_EXPR_RES("rreplace(z,'.*(a|xayy)','O')", "Oyyzz");
#endif
#endif
#endif

	setRegexType("");

#ifdef DEBUG
	// Keep these tests at the end. All real functions should go first
	VERIFY_EXPR_RES("testfunc(1,2,3,4)", "1,2,3,4");
	VERIFY_EXPR_RES("testfunc(1,,3,4)", "1,(nil),3,4");
	VERIFY_EXPR_RES("testfunc(,,3,4)", "(nil),(nil),3,4");
	VERIFY_EXPR_RES("testfunc(,2,3)", "(nil),2,3,(nil)");
	VERIFY_EXPR_RES("testfunc(1,2,)", "1,2,(nil),(nil)");
	VERIFY_EXPR_RES("testfunc(1,2)", "1,2,(nil),(nil)");
	VERIFY_EXPR_RES("testfunc()", "(nil),(nil),(nil),(nil)");
#endif
	return 0;
}

int runALUUnitTests15(unsigned int onlyTest)
{
	VERIFY_EXPR_RES("pget(unitTestVar)","NaN");
	VERIFY_EXPR_RES("#unitTestVar","NaN");
	VERIFY_EXPR_RES("pget(unitTestVar,8)","8");
	VERIFY_EXPR_RES("pdefined(unitTestVar)","0");
	VERIFY_EXPR_RES("pset(unitTestVar,9)","9");
	VERIFY_EXPR_RES("pget(unitTestVar)","9");
	VERIFY_EXPR_RES("#unitTestVar","9");
	VERIFY_EXPR_RES("pget(unitTestVar,8)","9");
	VERIFY_EXPR_RES("pdefined(unitTestVar)","1");
	VERIFY_EXPR_RES("pclear(unitTestVar)","9");
	VERIFY_EXPR_RES("pclear(unitTestVar)","NaN");
	VERIFY_EXPR_RES("pget(unitTestVar)","NaN");
	VERIFY_EXPR_RES("pget(unitTestVar,8)","8");
	VERIFY_EXPR_RES("pdefined(unitTestVar)","0");
	
	g_ps.setString(std::make_shared<std::string>("The quick brown fox     jumps over the lazy dog"));
	VERIFY_EXPR_RES("splus('ek',4)","");           // Should not find ek in the quick brown fox
	VERIFY_EXPR_RES("splus('ck',4)","r");
	VERIFY_EXPR_RES("splus('dog',-2)","y");
	VERIFY_EXPR_RES("splus('dog',2)","g");	
	VERIFY_EXPR_RES("splus('dog',3)","");	
	VERIFY_EXPR_RES("splus('dog',4)","");	
	VERIFY_EXPR_RES("splus('dog',400)","");	
	VERIFY_EXPR_RES("splus('dog',1,2)","og");	
	VERIFY_EXPR_RES("splus('dog',1,3)","og");	
	VERIFY_EXPR_RES("splus('dog',1,30)","og");	
	VERIFY_EXPR_RES("splus('The',1)","h");	
	VERIFY_EXPR_RES("splus('The',-1)","");
		

	VERIFY_EXPR_RES("wplus('fast',0)",""); 
	VERIFY_EXPR_RES("wplus('uick',0)",""); 
	VERIFY_EXPR_RES("wplus('quic',0)",""); 
	VERIFY_EXPR_RES("wplus('aquick',0)",""); 
	VERIFY_EXPR_RES("wplus('quickz',0)",""); 
	VERIFY_EXPR_RES("wplus('quick',0)","quick"); 
	VERIFY_EXPR_RES("wplus('quick',1)","brown"); 
	VERIFY_EXPR_RES("wplus('quick',-1)","The"); 
	VERIFY_EXPR_RES("wplus('quick',-2)",""); 
	VERIFY_EXPR_RES("wplus('quick',-12)",""); 
	VERIFY_EXPR_RES("wplus('quick',1,3)","brown fox     jumps"); 
	VERIFY_EXPR_RES("wplus('quick',1,7)","brown fox     jumps over the lazy dog"); 
	VERIFY_EXPR_RES("wplus('quick',1,8)","brown fox     jumps over the lazy dog"); 
	VERIFY_EXPR_RES("wplus('quick',1,70)","brown fox     jumps over the lazy dog"); 
	VERIFY_EXPR_RES("wplus('quick',7)","dog"); 
	VERIFY_EXPR_RES("wplus('quick',8)",""); 
	
	g_ps.setString(std::make_shared<std::string>("The\tquick brown\tfox jumps\tover the\tlazy dog"));
	VERIFY_EXPR_RES("fplus('fast',0)",""); 
	VERIFY_EXPR_RES("fplus('quick',0)",""); 
	VERIFY_EXPR_RES("fplus('brown',0)",""); 
	VERIFY_EXPR_RES("fplus('quick brown',0)","quick brown"); 
	VERIFY_EXPR_RES("fplus('quick brown',1)","fox jumps"); 
	VERIFY_EXPR_RES("fplus('quick brown',-1)","The"); 
	VERIFY_EXPR_RES("fplus('quick brown',-2)",""); 
	VERIFY_EXPR_RES("fplus('quick brown',-20)",""); 
	VERIFY_EXPR_RES("fplus('quick brown',3)","lazy dog"); 
	VERIFY_EXPR_RES("fplus('quick brown',4)",""); 
	VERIFY_EXPR_RES("fplus('quick brown',41)",""); 
	VERIFY_EXPR_RES("fplus('quick brown',1,1)","fox jumps"); 
	VERIFY_EXPR_RES("fplus('quick brown',1,2)","fox jumps\tover the"); 
	VERIFY_EXPR_RES("fplus('quick brown',1,3)","fox jumps\tover the\tlazy dog"); 
	VERIFY_EXPR_RES("fplus('quick brown',1,4)","fox jumps\tover the\tlazy dog"); 
	VERIFY_EXPR_RES("fplus('quick brown',1,34)","fox jumps\tover the\tlazy dog"); 

	return 0;
}


int runALUUnitTests16(unsigned int onlyTest)
{
	std::cout << "\nEvaluating Assignments\n======================\n\n";

	VERIFY_ASSN_RES("#4:=#3+1","4.14159265");
	VERIFY_ASSN_RES("#6:=1", "1");
	VERIFY_ASSN_RES("#6/=0", "NaN");

	counters.set(6,std::numeric_limits<ALUFloat>::quiet_NaN());
	VERIFY_ASSN_RES("#6+=5", "NaN");
	VERIFY_ASSN_RES("#6:=1", "1");   // Let can fix a NaN

	if (countFailures) {
		std::cout << "\n*** " << countFailures << " of " << testIndex << " tests failed.\n";
		std::cout << "Failed tests:\n";
		for (int i : failedTests) {
			std::cout << "\t" << i << "\n";
		}
		return 4;
	} else {
		std::cout << "\n*** All tests passed.\n";
		return 0;
	}
}

int main (int argc, char** argv)
{
	unsigned int onlyTest = 0;
	if (argc>1) {
		onlyTest = (unsigned int)(std::stoul(argv[1]));
	}
	g_bWarnAboutGrammars = false;

	specTimeSetTimeZone("UTC-2"); // All the time-format tests were set based on this time zone

	persistentVarLoad();

	int rc = runALUUnitTests(onlyTest);

	rc += runALUUnitTests2(onlyTest);
	rc += runALUUnitTests3(onlyTest);
	rc += runALUUnitTests4(onlyTest);
	rc += runALUUnitTests5(onlyTest);
	rc += runALUUnitTests6(onlyTest);
	rc += runALUUnitTests7(onlyTest);
	rc += runALUUnitTests8(onlyTest);
	rc += runALUUnitTests9(onlyTest);
	rc += runALUUnitTests10(onlyTest);
	rc += runALUUnitTests11(onlyTest);
	rc += runALUUnitTests12(onlyTest);
	rc += runALUUnitTests13(onlyTest);
	rc += runALUUnitTests14(onlyTest);
	rc += runALUUnitTests15(onlyTest);
	rc += runALUUnitTests16(onlyTest);

	persistentVarSaveIfNeeded();

	return rc;
}
