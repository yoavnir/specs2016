#include <map>
#include <iostream>
#include <fstream>
#include "utils/ErrorReporting.h"
#include "Config.h"
#include "persistent.h"

static std::map<std::string,std::string> PersistentVariables;
static bool g_bPersistentVariablesAreDirty = false;

std::string& persistentVarGet(std::string& key)
{
	return PersistentVariables[key];
}

bool         persistentVarDefined(std::string& key)
{
	return !PersistentVariables[key].empty();
}

void         persistentVarSet(std::string& key, std::string& value)
{
	PersistentVariables[key] = value;
	g_bPersistentVariablesAreDirty = true;
}

void         persistentVarClear(std::string& key)
{
	if (persistentVarDefined(key)) {
		PersistentVariables.erase(key);
		g_bPersistentVariablesAreDirty = true;
	}
}


void persistentVarLoad()
{
	std::string theKey, theValue;
	std::string persistenceFileName = getPersistneceFileName();
	std::ifstream persistenceFile(persistenceFileName);
	if (persistenceFile.is_open()) {
		while (getline(persistenceFile, theKey)) {
			if (getline(persistenceFile, theValue)) {
				persistentVarSet(theKey, theValue);
			}
		}
	}

	// Since we have just read the variables, mark them as not dirty.
	g_bPersistentVariablesAreDirty = false;
}

void persistentVarSaveIfNeeded()
{
	if (g_bPersistentVariablesAreDirty) {
		std::string persistenceFileName = getPersistneceFileName();
		std::ofstream persistenceFile(persistenceFileName);
		MYASSERT_WITH_MSG(persistenceFile.is_open(), "Cannot open persistence file for write");

		for (auto it = PersistentVariables.begin() ; it != PersistentVariables.end() ; it++) {
			persistenceFile << it->first << "\n" << it->second << "\n";
		}

		g_bPersistentVariablesAreDirty = false;
	}
}

