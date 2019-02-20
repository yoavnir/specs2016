#ifndef SPECS2016__UTILS_PLATFORM__H
#define SPECS2016__UTILS_PLATFORM__H

/*
 * This file contains every platform-dependent and compiler-dependent
 * define.
 */


// determine whether the compiler here supports put_time
#ifdef __clang__
	#if __GNUC__ > 3
		#define PUT_TIME__SUPPORTED
	#endif
#else
	#if __GNUC__ > 4
		#define PUT_TIME__SUPPORTED
	#endif
#endif

#ifdef WIN64
typedef unsigned long long int u_int64_t;
#endif

#endif
