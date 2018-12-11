#ifndef SPECS2016__PROCESSING__CONFIG__H
#define SPECS2016__PROCESSING__CONFIG__H

#include <string>

#define CONFIG_PARAMS  \
	X(bSupportUTF8,                 bool,         false,  EXP-UTF8,           true)       \
	X(bOutputTranslatedAscii,       bool,         false,  toASCII,            true)       \
	X(bForceFileRead,               bool,         false,  force-read-input,   true)       \
	X(specFile,                     std::string,  "",     specFile,           NEXTARG)    \

#define X(nm,typ,defval,cliswitch,oval) extern typ g_##nm;
CONFIG_PARAMS
#undef X

void readConfigurationFile();

bool configSpecLiteralExists(std::string& key);

std::string& configSpecLiteralGet(std::string& key);

#endif
