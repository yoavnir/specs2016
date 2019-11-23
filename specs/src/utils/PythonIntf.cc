
#ifndef SPECS_NO_PYTHON

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "PythonIntf.h"
#include "ErrorReporting.h"
#include <vector>


class PythonFuncArg {
public:
	PythonFuncArg(char* name) : m_name(name), m_default(counterType__None) {}
	PythonFuncArg(char* name, char* def) :
		m_name(name),m_default(counterType__Str),m_defStr(def)  {}
	PythonFuncArg(char* name, long def) :
		m_name(name),m_default(counterType__Int),m_defInt(def)  {}
	PythonFuncArg(char* name, double def) :
		m_name(name),m_default(counterType__Float),m_defFloat(def) {}
	std::string     getStr() {
		switch (m_default) {
		case counterType__None:
			return m_name;
		case counterType__Str:
			return m_name + "='" + m_defStr + "'";
		case counterType__Int:
			return m_name + "=" + std::to_string(m_defInt);
		case counterType__Float:
			return m_name + "=" + std::to_string(m_defFloat);
		}
	}
private:
	std::string     m_name;
	ALUCounterType  m_default;
	std::string     m_defStr;
	long            m_defInt;
	double          m_defFloat;
};

class PythonFuncRec : public ExternalFunctionRec {
public:
	PythonFuncRec(PyObject* _pFunc) : m_pFuncPtr(_pFunc), m_pTuple(NULL) {}

	void addArg(char* name) {
		m_args.push_back(PythonFuncArg(name));
	}

	void addArg(char* name, char* def) {
		m_args.push_back(PythonFuncArg(name,def));
	}

	void addArg(char* name, long def) {
		m_args.push_back(PythonFuncArg(name,def));
	}

	void addArg(char* name, double def) {
		m_args.push_back(PythonFuncArg(name,def));
	}

	size_t GetArgCount() {
		return m_args.size();
	}

	void ResetArgs() {
		m_pTuple = NULL;   // Yeah, that requires something more elaborate
	}

	void setArgValue(size_t idx, ALUValue *pValue) {
		PyObject* pValObj;
		size_t argCount = GetArgCount();

		MYASSERT(idx < argCount);

		if (!m_pTuple) m_pTuple = PyTuple_New(GetArgCount());

		switch (pValue->getType()) {
		case counterType__Int:
			pValObj = PyLong_FromLongLong(pValue->getInt());
			break;
		case counterType__Float:
			pValObj = PyFloat_FromDouble(pValue->getFloat());
			break;
		case counterType__Str:
			pValObj = PyUnicode_FromString(pValue->getStr().c_str());
			break;
		default:
			pValObj = Py_None;
		}
		PyTuple_SetItem(m_pTuple, idx, pValObj);
	}

	ALUValue* Call() {
		return new ALUValue();  // maybe we want to do it a little better, no?
	}

	std::string getStr() {
		std::string res = "func@" + std::to_string(long(m_pFuncPtr)) + "(";
		for (auto& arg : m_args) {
			res += arg.getStr();
		}
		res += ")";
		return res;
	}
private:
	PyObject*                  m_pFuncPtr;
	std::vector<PythonFuncArg> m_args;
	PyObject*                  m_pTuple;
};


class PythonFunctionCollection : public ExternalFunctionCollection {
public:
	PythonFunctionCollection() : m_Initialized(false) {}
	virtual void  Initialize() {
		Py_Initialize();
		PyObject* pInspectMod = PyImport_ImportModule("inspect");
		MYASSERT_NOT_NULL(pInspectMod);

		PyObject* pSignatureFunc = PyObject_GetAttrString(pInspectMod,"signature");
		MYASSERT_NOT_NULL(pSignatureFunc);

		m_Initialized = true;
	}
	virtual bool IsInitialized() {
		return m_Initialized;
	}
	virtual size_t CountFunctions() {
		return m_Functions.size();
	}
	virtual ExternalFunctionRec* GetFunctionByName(std::string fname) {
		return m_Functions[fname];
	}

private:
	std::map<std::string,PythonFuncRec*> m_Functions;
	bool                                 m_Initialized;
};

bool pythonInterfaceEnabled()
{
	PythonFuncRec rec(NULL);
	return true;
}

static PythonFunctionCollection gFunctionCollection;

ExternalFunctionCollection* p_gExternalFunctions = &gFunctionCollection;


#else  // SPECS_NO_PYTHON
#include "PythonIntf.h"

bool pythonInterfaceEnabled()
{
	return false;
}

#endif

