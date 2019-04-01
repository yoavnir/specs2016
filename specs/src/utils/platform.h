#ifndef SPECS2016__UTILS_PLATFORM__H
#define SPECS2016__UTILS_PLATFORM__H

/*
 * This file contains every platform-dependent and compiler-dependent
 * define.
 */


#ifdef WIN64
typedef unsigned long long int u_int64_t;
int setenv(const char *name, const char *value, int overwrite);
#endif

#ifdef _MSC_VER 
	//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
	#define strncasecmp _strnicmp
	#define strcasecmp _stricmp
	#define ALUFloatPrecision  15
	#define VISUAL_STUDIO
#else
	#define ALUFloatPrecision 16
#endif

#endif
