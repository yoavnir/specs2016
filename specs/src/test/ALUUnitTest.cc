#include <iostream>
#include <iomanip>
#include "utils/ErrorReporting.h"
#include "utils/alu.h"

ALUCounters counters;

std::string counterTypeNames[]= {"None", "Str", "Int", "Float"};

#define VERIFY_TYPE(i,t) \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
		": type[" << i << "]=="#t": "; \
	if (counters.type(i)==counterType__##t) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counterTypeNames[counters.type(i)] << ")\n"; \
		countFailures++; \
	}

#define VERIFY_INT(i,val) \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
		": #" << i << "==" << val << ": "; \
	if (counters.getInt(i)==val) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counters.getInt(i) << ")\n"; \
		countFailures++; \
	}

#define VERIFY_WHOLE(i,b) \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
		": #" << i << (b ? " is whole: ":" isn't whole: "); \
	if (counters.isWholeNumber(i)==b) {  \
		std::cout << "OK (" << counters.getFloat(i) << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << counters.getFloat(i) << ")\n";  \
		countFailures++; \
	}

#define VERIFY_HEX(i,val) \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
		": #" << i << "==" << val << ": "; \
	if (counters.getHex(i)==val) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counters.getHex(i) << ")\n"; \
		countFailures++; \
	}

#define VERIFY_FLOAT(i,val) \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
		": #" << i << "==" << std::setprecision(ALUFloatPrecision) << val << ": "; \
	if (counters.getFloat(i)==val) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counters.getFloat(i) << ")\n"; \
		countFailures++; \
	}

#define VERIFY_STR(i,val) \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
		": #" << i << "==" << val << ": "; \
	if (counters.getStr(i)==val) {  \
		std::cout << "OK\n"; \
	} else { \
		std::cout << "*** NOT OK *** (Got " << counters.getStr(i) << ")\n"; \
		countFailures++; \
	}

#define VERIFY_NUMERIC(i,b)	\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": #" << i << (b ? " is numeric: ":" isn't numeric: "); \
	if (counters.isNumeric(i)==b) {  \
		std::cout << "OK (";    \
	} else {     \
		std::cout << "*** NOT OK *** (";  \
		countFailures++; \
	}  \
	std::cout << counters.getStr(i) << ")\n";

#define VERIFY_DIVINED_TYPE(i,t) \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": #" << i << " is "<< ALUCounterType2Str[counterType__##t] <<": "; \
	if (counters.divinedType(i)==counterType__##t) { \
		std::cout << "OK (" << counters.getStr(i) << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << counters.getStr(i) << " - " <<   \
				ALUCounterType2Str[counters.divinedType(i)] << ")\n";  \
		countFailures++; \
	}

#define UNIT_DIVINED_TYPE(u,t) { \
	ALUCounter* ctr = u.compute(); \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": "<< #u << " is "<< ALUCounterType2Str[counterType__##t] <<": "; \
	if (ctr->getDivinedType()==counterType__##t) { \
		std::cout << "OK (" << ctr->getStr() << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << ctr->getStr() << " - " <<   \
				ALUCounterType2Str[ctr->getDivinedType()] << ")\n";  \
		countFailures++; \
	} \
	delete ctr; \
}

#define VERIFY_UNIT_ST(u,s) { \
	ALUCounter* ctr = u.compute(&counters); \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": "<< #u << " is \""<< s <<"\": "; \
	if (ctr->getStr()==s) { \
		std::cout << "OK.\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << ctr->getStr() << ")\n";  \
		countFailures++; \
	} \
	delete ctr; \
}

#define VERIFY_UNIT_INT(u,i) { \
	ALUCounter* ctr = u.compute(&counters); \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": "<< #u << " is "<< i <<": "; \
	if (ctr->getInt()==i) { \
		std::cout << "OK (" << ctr->getStr() << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << ctr->getStr() << " - " << ctr->getInt() << ")\n";  \
		countFailures++; \
	} \
	delete ctr; \
}

#define VERIFY_UNIT_F(u,f) { \
	ALUCounter* ctr = u.compute(&counters); \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": "<< #u << " is "<< f <<": "; \
	if (ctr->getFloat()==f) { \
		std::cout << "OK (" << ctr->getStr() << ")\n";    \
	} else {     \
		std::cout << "*** NOT OK *** (" << ctr->getStr() << " - " << ctr->getFloat() << ")\n";  \
		countFailures++; \
	} \
	delete ctr; \
}

#define VERIFY_UNARY(u,o,t,s) { \
	AluUnitCounter ctr(o);	\
	ALUCounter* _op = ctr.compute(&counters);	\
	ALUCounter* _res = u.compute(_op);	\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": "<< #u << "(" << #t << ") is \""<< s <<"\": "; \
	if (counterType__##t!=_res->getType()) { \
		std::cout << "*** NOT OK *** (type=" << ALUCounterType2Str[_res->getType()] << ")\n"; \
		countFailures++; \
	} else if ((_res->getType()!=counterType__None) && (_res->getStr() != s)) {	\
		std::cout << "*** NOT OK *** (" << _res->getStr() << ")\n"; \
		countFailures++; \
	} else { \
		std::cout << "OK.\n"; \
	} \
	delete _res; \
}

#define VERIFY_BINARY(u,o1,o2,t,s) { \
	AluUnitCounter ctr1(o1);	\
	AluUnitCounter ctr2(o2);	\
	ALUCounter* _op1 = ctr1.compute(&counters);	\
	ALUCounter* _op2 = ctr2.compute(&counters);	\
	ALUCounter* _res = u.compute(_op1,_op2);	\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": "<< #u << "(" << #t << ") is \""<< s <<"\": "; \
	if (counterType__##t!=_res->getType()) { \
		std::cout << "*** NOT OK *** (type=" << ALUCounterType2Str[_res->getType()] << ")\n"; \
		countFailures++; \
	} else if ((_res->getType()!=counterType__None) && (_res->getStr() != s)) {	\
		std::cout << "*** NOT OK *** (" << _res->getStr() << ")\n"; \
		countFailures++; \
	} else { \
		std::cout << "OK.\n"; \
	} \
	delete _res; \
}

#define VERIFY_ASSN(u,p,o,t,s) {		\
	AluUnitCounter 	ctr(o);	\
	ALUCounter*		op = ctr.compute(&counters);	\
	u.perform(p,&counters,op);			\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": "<< #u << "(" << #t << ") is \""<< s <<"\": "; \
	if (counterType__##t!=counters.type(p)) { \
		std::cout << "*** NOT OK *** (type=" << ALUCounterType2Str[counters.type(p)] << ")\n"; \
		countFailures++; \
	} else if ((counters.type(p)!=counterType__None) && (counters.getStr(p) != s)) {	\
		std::cout << "*** NOT OK *** (" << counters.getStr(p) << ")\n"; \
		countFailures++; \
	} else { \
		std::cout << "OK.\n"; \
	} \
}

#define VERIFY_EXPR(s,e) {						\
	std::string _expr(s);						\
	bool _res = parseAluExpression(_expr,vec);	\
	std::string _dump = dumpAluVec(vec, true);	\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": <"<< s << "> ==> \"" << e << "\": "; 	\
	if (_dump==e) std::cout << "OK\n";			\
	else {										\
		std::cout << "*** NOT OK *** - " << _dump << "\n";	\
		countFailures++;						\
	}											\
}

#define VERIFY_ASSNMENT(s,ex)  {							\
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
		_res =  (e.what(true)==std::string(ex));				\
		actual = e.what(true);									\
		dumpAluVec(vec,true);								\
	}														\
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
	": <"<< s << "> ==> \"" << ex << "\": "; 				\
	if (_res) std::cout << "OK\n";							\
	else {													\
		std::cout << "*** NOT OK *** - " << actual << "\n";	\
		countFailures++;									\
	}														\
}

#define VERIFY_RPN(s,ex) {									\
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
		std::cout << "Test #" << std::setfill('0') << std::setw(3) << ++testIndex << \
		": <"<< s << "> ==> \"" << ex << "\": ";			\
		if (_res) std::cout << "OK\n";						\
		else {												\
			std::cout << "*** NOT OK *** - " << _dump << "\n";	\
			countFailures++;								\
		}													\
	}


int main (int argc, char** argv)
{
	unsigned int testIndex = 0;
	unsigned int countFailures = 0;

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
	VERIFY_UNARY(uNot,0,None,"0");
	VERIFY_UNARY(uNot,12,Int,"1");
	VERIFY_UNARY(uNot,13,Int,"1");

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
	VERIFY_BINARY(uDiv,4,9,Float,"1.892307692307692");  // 123 / 65
	VERIFY_BINARY(uDiv,4,33,Int,"41");      // 123 / 3
	VERIFY_BINARY(uIntDiv,4,9,Int,"1");     // 123 % 65 where % is integer division
	VERIFY_BINARY(uIntDiv,7,3,Int,"32");     // 98.6 % 3.14
	VERIFY_BINARY(uRemDiv,4,9,Int,"58");     // 123 // 65 where // is modulo
	VERIFY_BINARY(uRemDiv,7,3,Int,"2");     // 98.6 // 3.14 ==> 98 // 3
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


#define X(nm,st) AluAssnOperator uAss##nm(st);
	ALU_ASSOP_LIST
#undef X

	VERIFY_ASSN(uAssLet,33,3,Float,"3.14159265");
	VERIFY_ASSN(uAssAdd,33,3,Float,"6.2831853");
	VERIFY_ASSN(uAssSub,33,3,Float,"3.14159265");
	VERIFY_ASSN(uAssAppnd,33,3,Str,"3.141592653.14159265");

	// TODO: Many more needed

	AluVec vec;
	VERIFY_EXPR("23+45", "Number(23);BOP(+);Number(45)");
	VERIFY_EXPR(" 23 + -8", "Number(23);BOP(+);Number(-8)");

	VERIFY_EXPR("34+b-#2","Number(34);BOP(+);FI(b);BOP(-);Counter(2)");
	VERIFY_EXPR("34+b- -c","Number(34);BOP(+);FI(b);BOP(-);UOP(-);FI(c)");

	VERIFY_EXPR("2*(2+2)","Number(2);BOP(*);(;Number(2);BOP(+);Number(2);)");

	VERIFY_EXPR("5+sqrt(1)-pow(3,4)","Number(5);BOP(+);FUNC(sqrt);(;Number(1););BOP(-);FUNC(pow);(;Number(3);COMMA;Number(4);)");
	VERIFY_EXPR("a>b & b>c","FI(a);BOP(>);FI(b);BOP(&);FI(b);BOP(>);FI(c)");


	// TODO: Yeah, a whole bunch of more expressions

	VERIFY_ASSNMENT("#6 := 2+2","Number(2);BOP(+);Number(2)");
	VERIFY_ASSNMENT("#6 = 2+2","ALU assignment statements must have an assignment operator as the second element. Got BOP(=) instead.");
	VERIFY_ASSNMENT("b := 2+2","ALU assignment statements must begin with a counter. Got FI(b) instead.");
	VERIFY_ASSNMENT("#6 b += 2+2","ALU assignment statements must have an assignment operator as the second element. Got FI(b) instead.");
	VERIFY_ASSNMENT("2+2 := 4","ALU assignment statements must begin with a counter. Got Number(2) instead.");

	// TODO: More

	VERIFY_RPN("2+3", "Number(2);Number(3);BOP(+)");
	VERIFY_RPN("-b","FI(b);UOP(-)");
	VERIFY_RPN("-2+3","Number(-2);Number(3);BOP(+)");
	VERIFY_RPN("-(2+3)","Number(2);Number(3);BOP(+);UOP(-)");
	VERIFY_RPN("a+b-c","FI(a);FI(b);BOP(+);FI(c);BOP(-)");
	VERIFY_RPN("a-b+c","FI(a);FI(b);BOP(-);FI(c);BOP(+)")
	VERIFY_RPN("a*b-c","FI(a);FI(b);BOP(*);FI(c);BOP(-)");
	VERIFY_RPN("a+b*c","FI(a);FI(b);FI(c);BOP(*);BOP(+)");
	VERIFY_RPN("(a+b)*c","FI(a);FI(b);BOP(+);FI(c);BOP(*)");
	VERIFY_RPN("2<3","Number(2);Number(3);BOP(<)");
	VERIFY_RPN("a>b","FI(a);FI(b);BOP(>)");
	VERIFY_RPN("a>b & b>c","FI(a);FI(b);BOP(>);FI(b);FI(c);BOP(>);BOP(&)");

	if (countFailures) {
		std::cout << "\n*** " << countFailures << " of " << testIndex << " tests failed.\n";
		return 4;
	} else {
		std::cout << "\n*** All tests passed.\n";
		return 0;
	}
}
