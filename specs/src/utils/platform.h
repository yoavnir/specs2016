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
#define SUPPRESS_UNUSED_WARNING(a)
#else
#define SUPPRESS_UNUSED_WARNING(a) (void)a
#endif

#ifdef _MSC_VER 
	//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
	#define strncasecmp _strnicmp
	#define strcasecmp _stricmp
	#define ALUFloatPrecision  15
	#define VISUAL_STUDIO
	#define NOMINMAX 1
#else
#ifdef __LDBL_DIG__
	#define ALUFloatPrecision __LDBL_DIG__
#else
	#define ALUFloatPrecision 16
#endif
#endif

#ifdef ALURAND_CommonCrypto
  #include <CommonCrypto/CommonRandom.h>
  #define AluRandContext   char
  #define AluRandSeedType  char
  #define AluRandSeed(s)
  #define ALU_RAND_FUNC_WITH_LEN
  #define AluRandFunc(d,l) CCRandomGenerateBytes((void*)(d), (size_t)(l))
  #define RandomProvider "Common Crypto"
#endif

#ifdef ALURAND_rand48
  #include <stdlib.h>
  #define AluRandContext    drand48_data
  #define AluRandSeedType   long int
  #define AluRandSeed(s)    srand48_r(s,&AluRandCtxBuffer_G)
  #define AluRandFunc(r)    lrand48_r(&AluRandCtxBuffer_G, (long int*)(&r))
  #define RandomProvider "rand48"
#endif

#ifdef ALURAND_wincrypt
  #include <windows.h>
  #include <wincrypt.h>
  #define AluRandContext    HCRYPTPROV
  #define AluRandSeedType   char
  #define AluRandSeed(s)    if (0==CryptAcquireContext(&AluRandCtxBuffer_G, NULL, \
		  nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {                            \
		      std::string err = "CryptAquireContext() failed. GetLastError() returns " + std::to_string(GetLastError()); \
		      MYTHROW(err);   \
		  }
  #define ALU_RAND_FUNC_WITH_LEN
  #define AluRandFunc(d,l)    if (0==CryptGenRandom(AluRandCtxBuffer_G, (DWORD)(l), (BYTE*)(d))) {  \
		      std::string err = "CryptGenRandom() failed. GetLastError() returns " + std::to_string(GetLastError()); \
		      MYTHROW(err);   \
		  }
  #define RandomProvider "WinCrypt"
#endif

#ifdef ALURAND_rand
  #include <stdlib.h>
  #define AluRandContext    unsigned int
  #define AluRandSeedType   unsigned int
  #define AluRandSeed(s)    srand(s)
  #define AluRandFunc(r)    r = rand()
  #define RandomProvider "stdlib"
#endif

#ifndef AluRandContext
  #error "No random number generator defined"
#endif

#ifdef DEBUG
#define QUEUE_HIGH_WM 10
#define QUEUE_LOW_WM  8
#else
#define QUEUE_HIGH_WM 5000
#define QUEUE_LOW_WM  4500
#endif

#ifdef WIN64
#define PATHSEP "\\"
#define PATH_LIST_SEPARATOR ";"
#define PATH_LIST_WSEPARATOR L";"
#else
#define PATHSEP "/"
#define PATH_LIST_SEPARATOR ":"
#define PATH_LIST_WSEPARATOR L":"
#endif

#ifdef WIN64
#define DEFAULT_SPECS_PARENT_DIR "APPDATA"
#define FALLBACK_SPECS_PARENT_DIR  "C:"
#else
#define DEFAULT_SPECS_PARENT_DIR "HOME"
#define FALLBACK_SPECS_PARENT_DIR  "/etc"
#endif

char** getDirectoryFileNames(const char* spath);

#endif
