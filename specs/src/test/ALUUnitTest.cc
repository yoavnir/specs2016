#include <iostream>
#include <iomanip>
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
	ALUCounter* ctr = u.compute(); \
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
	ALUCounter* ctr = u.compute(); \
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
	ALUCounter* ctr = u.compute(); \
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
	AluUnitCounter ctr(o,&counters);	\
	ALUCounter* _op = ctr.compute();	\
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
	AluUnitCounter ctr1(o1,&counters);	\
	AluUnitCounter ctr2(o2,&counters);	\
	ALUCounter* _op1 = ctr1.compute();	\
	ALUCounter* _op2 = ctr2.compute();	\
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

	AluUnitCounter uCtr(8,&counters);
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

#define X(nm,st) AluBinaryOperator u##nm(st);
	ALU_BOP_LIST
#undef X
	counters.set(4,"123");
	VERIFY_BINARY(uAdd,8,11,Float,"90.6");  // 98.6 + (-8)
	VERIFY_BINARY(uAdd,1,3,Float,"3.14159265");  // "hello" + 3.14159265
	VERIFY_BINARY(uAdd,9,3,Float,"68.14159265"); // 65 + 3.14159265
	VERIFY_BINARY(uAdd,9,4,Int,"188"); // 65 + 123
	VERIFY_BINARY(uSub,4,9,Int,"58");  // 123 - 65
	VERIFY_BINARY(uSub,9,4,Int,"-58"); // 65 - 123
	VERIFY_BINARY(uSub,9,3,Float,"61.85840735"); // 65 - 3.14159265
	VERIFY_BINARY(uSub,3,1,Float,"3.14159265"); // 3.14159265 - "hello"
	VERIFY_BINARY(uMult,8,11,Float,"-788.8");    // 98.6 * (-8)
	VERIFY_BINARY(uMult,9,4,Int,"7995");    // 65*123
	VERIFY_BINARY(uMult,3,9,Float,"204.20352225");	// Pi * 65
	VERIFY_BINARY(uDiv, 4,9,Float,"1.892307692307692");  // 123 / 65
	VERIFY_BINARY(uDiv,4,33,Int,"41");      // 123 / 3
	VERIFY_BINARY(uIntDiv,4,9,Int,"1");     // 123 % 65 where % is integer division
	VERIFY_BINARY(uIntDiv,7,3,Int,"32");     // 98.6 % 3.14
	VERIFY_BINARY(uRemDiv,4,9,Int,"58");     // 123 // 65 where // is modulo
	VERIFY_BINARY(uRemDiv,7,3,Int,"2");     // 98.6 // 3.14 ==> 98 // 3
	VERIFY_BINARY(uAppnd,1,3,Str,"hello3.14159265");


	if (countFailures) {
		std::cout << "\n*** " << countFailures << " of " << testIndex << " tests failed.\n";
		return 4;
	} else {
		std::cout << "\n*** All tests passed.\n";
		return 0;
	}
}
