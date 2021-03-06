#ifndef SPECS2016__PYTHON__INTF__H
#define SPECS2016__PYTHON__INTF__H

#include <memory>
#include "utils/aluValue.h"

bool pythonInterfaceEnabled();

class ExternalFunctionRec {
public:
	virtual size_t    GetArgCount() = 0;
	virtual void      ResetArgs() = 0;
	virtual void      setArgValue(size_t idx, PValue pValue) = 0;
	virtual PValue    Call() = 0;
};

typedef std::shared_ptr<ExternalFunctionRec> PExternalFunctionRec;

class ExternalFunctionCollection {
public:
	virtual bool                 IsInitialized() = 0;
	virtual void                 Initialize(const char* _path) = 0;
	virtual void                 Debug() = 0;
	virtual bool                 DebugOne(std::string& funcName) = 0;
	virtual size_t               CountFunctions() = 0;
	virtual PExternalFunctionRec GetFunctionByName(std::string fname) = 0;
	virtual void                 SetErrorHandling(std::string& smethod) = 0;
};

extern ExternalFunctionCollection* p_gExternalFunctions;

#endif
