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

#if ALURAND == CommonCrypto
  #include <CommonCrypto/CommonRandom.h>
  #define AluRandContext   char
  #define AluRandSeedType  char
  #define AluRandSeed(s)
  #define ALU_RAND_FUNC_WITH_LEN
  #define AluRandFunc(d,l) CCRandomGenerateBytes((void*)(d), (size_t)(l))
#elif ALURAND == rand48
  #include <stdlib.h>
  #define AluRandContext    drand48_data
  #define AluRandSeedType   long int
  #define AluRandSeed(s)    srand48_r(s,&AluRandCtxBuffer_G)
  #define AluRandFunc(r)    lrand48_r(&AluRandCtxBuffer_G, &r)
#elif ALURAND == wincrypt
  #include <wincrypt.h>
  #define AluRandContext    HCRYPTPROV
  #define AluRandSeedType   char
  #define AluRandSeed(s)    CryptAcquireContext(&AluRandCtxBuffer_G, NULL, \
		  (LPCWSTR)L"Microsoft Base Cryptographic Provider v1.0", \
		  PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)
  #define AluRandFunc(r)    CryptGenRandom(AluRandCtxBuffer_G, ALUInt_SZ, &r)
#elif ALURAND == rand
  #include <stdlib.h>
  #define AluRandContext    unsigned int
  #define AluRandSeedType   unsigned int
  #define AluRandSeed(s)    srand(s)
  #define AluRandFunc(r)    rand_r(&AluRandCtxBuffer_G, &r)
#else
  #error "No random number generator found."
#endif

#endif
