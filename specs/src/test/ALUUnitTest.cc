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

	if (countFailures) {
		std::cout << "\n*** " << countFailures << " of " << testIndex << " tests failed.\n";
		return 4;
	} else {
		std::cout << "\n*** All tests passes.\n";
		return 0;
	}
}
