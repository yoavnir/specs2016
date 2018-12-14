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

#define X(nm,typ,defval,ssw,cliswitch,oval) extern typ g_##nm;
CONFIG_PARAMS
#undef X

void readConfigurationFile();

bool configSpecLiteralExists(std::string& key);

std::string& configSpecLiteralGet(std::string& key);

#endif
