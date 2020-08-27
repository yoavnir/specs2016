#ifndef SPECS2016__PROCESSING__PERSISTENT__H
#define SPECS2016__PROCESSING__PERSISTENT__H

#include <string>

std::string& persistentVarGet(std::string& key);
bool         persistentVarDefined(std::string& key);
void         persistentVarSet(std::string& key, std::string& value);
void         persistentVarClear(std::string& key);

void persistentVarLoad();
void persistentVarSaveIfNeeded();

#endif
