#ifndef SPECS2016__PROCESSING__CONFIG__H
#define SPECS2016__PROCESSING__CONFIG__H

#include <string>

// For the ssw parameter, use zero (0) for the short switch if none is needed
#define CONFIG_PARAMS  \
	X(bDummy,                       bool,         false,  0,DummyOption,        true)       \
	X(bSupportUTF8,                 bool,         false,  0,EXP-UTF8,           true)       \
	X(bOutputTranslatedAscii,       bool,         false,  0,toASCII,            true)       \
	X(bForceFileRead,               bool,         false,  0,force-read-input,   true)       \
	X(specFile,                     std::string,  "",     f,specFile,           NEXTARG)    \
	X(bVerbose,                     bool,         false,  v,verbose,            true)       \
	X(bPrintStats,                  bool,         false,  0,stats,              true)       \
	X(inputFile,                    std::string,  "",     i,inFile,             NEXTARG)    \
	X(outputFile,                   std::string,  "",     o,outFile,            NEXTARG)    \
	X(bLocalWhiteSpace,             bool,         true,   w,spaceWS,            false)      \
	X(bDebugAluCompile,             bool,         false,  0,debug-alu-comp,     true)       \
	X(bDebugAluRun,                 bool,         false,  0,debug-alu-run,      true)       \
	X(configurationFile,            std::string,  "",     c,config,             NEXTARG)    \
	X(timeZone,                     std::string,  "",     0,timezone,           NEXTARG)    \
	X(recfm,                        std::string,  "",     0,recfm,              NEXTARG)    \
	X(lrecl,                        int,          0,      0,lrecl,   std::stoi(NEXTARG))    \
	X(linedel,                      std::string,  "",     0,linedel,            NEXTARG)    \
	X(configuredString,             std::string,  "",     s,set,                NEXTARG)    \
	X(regexSyntaxType,              std::string,  "",     0,regexType,          NEXTARG)    \
	X(pythonFuncs,                  std::string,  "auto", 0,pythonFuncs,        NEXTARG)    \
	X(pythonErr,                    std::string,  "",     0,pythonErr,          NEXTARG)    \
	X(help,                         std::string,  "",     0,help,               NEXTARG)    \
	X(inputStream2,                 std::string,  "",     0,is2,                NEXTARG)    \
	X(inputStream3,                 std::string,  "",     0,is3,                NEXTARG)    \
	X(inputStream4,                 std::string,  "",     0,is4,                NEXTARG)    \
	X(inputStream5,                 std::string,  "",     0,is5,                NEXTARG)    \
	X(inputStream6,                 std::string,  "",     0,is6,                NEXTARG)    \
	X(inputStream7,                 std::string,  "",     0,is7,                NEXTARG)    \
	X(inputStream8,                 std::string,  "",     0,is8,                NEXTARG)    \
	X(outputStream2,                std::string,  "",     0,os2,                NEXTARG)    \
	X(outputStream3,                std::string,  "",     0,os3,                NEXTARG)    \
	X(outputStream4,                std::string,  "",     0,os4,                NEXTARG)    \
	X(outputStream5,                std::string,  "",     0,os5,                NEXTARG)    \
	X(outputStream6,                std::string,  "",     0,os6,                NEXTARG)    \
	X(outputStream7,                std::string,  "",     0,os7,                NEXTARG)    \
	X(outputStream8,                std::string,  "",     0,os8,                NEXTARG)    \

#define X(nm,typ,defval,ssw,cliswitch,oval) extern typ g_##nm;
CONFIG_PARAMS
#undef X

#define EXTERNAL_FUNC_AUTO  "auto"
#define EXTERNAL_FUNC_ON    "on"
#define EXTERNAL_FUNC_OFF   "off"

#define EXTERNAL_FUNC_ERR_THROW   "throw"
#define EXTERNAL_FUNC_ERR_NAN     "nan"
#define EXTERNAL_FUNC_ERR_ZERO    "zero"
#define EXTERNAL_FUNC_ERR_NULLSTR "nullstr"

void readConfigurationFile();

bool configSpecLiteralExists(std::string& key);

std::string& configSpecLiteralGet(std::string& key);

std::string& configSpecLiteralGetWithDefault(std::string& key, std::string& _default);

void configSpecLiteralSet(std::string& key, std::string& value);

bool anyNonPrimaryInputStreamDefined();

bool inputStreamIsDefined(int i);

const char* getFullSpecPath();

std::string getPersistneceFileName();

#endif
