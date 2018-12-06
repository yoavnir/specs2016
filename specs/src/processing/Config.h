#ifndef SPECS2016__PROCESSING__CONFIG__H
#define SPECS2016__PROCESSING__CONFIG__H

#include <string>

extern bool g_bSupportUTF8;

extern bool g_bOutputTranslatedAscii;

void readConfigurationFile();

bool configSpecLiteralExists(std::string& key);

std::string& configSpecLiteralGet(std::string& key);

#endif
