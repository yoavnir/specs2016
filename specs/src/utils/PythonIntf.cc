
#ifndef SPECS_NO_PYTHON

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "PythonIntf.h"
#include "ErrorReporting.h"
#include "aluFunctions.h"
#include "processing/Config.h"
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>

// Some defines for compatibility
#ifdef PYTHON_VER_3
static const char emptyString[]="";
#define PyInt_Check(x)         false
#define PyInt_AsLong(x)           0L
#define PyString_Check(x)      false
#define PyString_AS_STRING(x) ((char*)(emptyString))
#endif

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
		case counterType__Str:
			return m_name + "='" + m_defStr + "'";
		case counterType__Int:
			return m_name + "=" + std::to_string(m_defInt);
		case counterType__Float:
			return m_name + "=" + std::to_string(m_defFloat);
		default:
			return m_name;
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
	PythonFuncRec(std::string& _name, PyObject* _pFunc) : m_name(_name), m_pFuncPtr(_pFunc), m_pTuple(NULL) {}

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

	PyObject* GetFuncPtr() {
		return m_pFuncPtr;
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
		std::ostringstream strm;
		strm << m_name << " @ " << std::hex << m_pFuncPtr << " (";
		bool first = true;
		for (auto& arg : m_args) {
			if (first) {
				first = false;
			} else {
				strm << ", ";
			}
			strm << arg.getStr();
		}
		strm << ")";
		return strm.str();
	}
private:
	std::string                m_name;
	PyObject*                  m_pFuncPtr;
	std::vector<PythonFuncArg> m_args;
	PyObject*                  m_pTuple;
};


class PythonFunctionCollection : public ExternalFunctionCollection {
public:
	PythonFunctionCollection() : m_Initialized(false) {}

	~PythonFunctionCollection() {
		if (Py_IsInitialized()) {
			Py_Finalize();
			if (g_bVerbose) {
				std::cerr << "Python Interface: Unloaded" << std::endl;
			}
		}
	}

	virtual void  Initialize(const char* _path) {

		// update the python path
		if (_path && _path[0]) {
#ifdef PYTHON_VER_3
			std::wstring wpath(strlen(_path)+1, L'#');
			mbstowcs(&wpath[0],_path,strlen(_path));
			wpath.erase(wpath.length()-1);
			std::wstring newPath = std::wstring(Py_GetPath()) + std::wstring(L":") + wpath;
			Py_SetPath(newPath.data());
			Py_Initialize();
#else
			Py_Initialize();
			std::string newPath = std::string(Py_GetPath()) + ":" + std::string(_path);
			PySys_SetPath((char*)newPath.c_str());
#endif
		} else {
			Py_Initialize();
		}

		// Get the argument parsing function getfullargspec/getargspec
		PyObject* pInspectMod = PyImport_ImportModule("inspect");
		MYASSERT_NOT_NULL(pInspectMod);

		// load the local functions at localfuncs.py
		m_LocalMod = PyImport_ImportModule("localfuncs");
		if (!m_LocalMod) {
			// TODO: When we support only 3.6+, PyExc_ModuleNotFoundError is a more specialized error
			if (PyErr_GivenExceptionMatches(PyErr_Occurred(), PyExc_ImportError)) {
				if (g_bVerbose) {
					std::cerr << "Python Interface: Local functions not found" << std::endl;
				}
				m_Initialized = true;
				return;
			}
		}

		PyObject* pArgSpecFunc = PyObject_GetAttrString(pInspectMod,"getfullargspec");
		if (!pArgSpecFunc) {
			if (g_bVerbose) {
				std::cerr << "Python Interface: ";
				PyErr_Print();
			} else {
				PyErr_Clear();
			}

			pArgSpecFunc = PyObject_GetAttrString(pInspectMod,"getargspec");
			MYASSERT_NOT_NULL(pArgSpecFunc);
		}

		// Get a dictionary of all the module's functions
		PyObject* pModuleDictionary = PyModule_GetDict(m_LocalMod);
		MYASSERT_NOT_NULL(pModuleDictionary);

		PyObject* pModuleKeyList = PyDict_Keys(pModuleDictionary);
		MYASSERT_NOT_NULL(pModuleKeyList);
		auto keyListSize = PyList_Size(pModuleKeyList);

		for (Py_ssize_t i = 0 ; i < keyListSize ; i++) {
			PyObject* pKey = PyList_GetItem(pModuleKeyList, i);
			PyObject* pRepr = PyObject_Repr(pKey);
#ifdef PYTHON_VER_2
			const char *pKeyName = PyString_AS_STRING(pRepr);
#else
			PyObject* pStr = PyUnicode_AsEncodedString(pRepr, "utf-8", "~E~");
			const char *pKeyName = PyBytes_AS_STRING(pStr);
#endif

			if (0==strncmp("'pyspecfunc_", pKeyName, 11)) {
				std::string funcName = std::string((char*)pKeyName + 12);
				funcName.resize(funcName.size()-1);
				PyObject *pFunc = PyDict_GetItem(pModuleDictionary, pKey);
				MYASSERT_NOT_NULL(pFunc);
				MYASSERT(PyCallable_Check(pFunc));

				// Set up arguments for the getargspec function
				PyObject *pTuple = PyTuple_New(1);
				PyTuple_SetItem(pTuple, 0, pFunc);

				PyObject* pArgSpec = PyObject_CallObject(pArgSpecFunc, pTuple);
				MYASSERT_NOT_NULL_WITH_DESC(pArgSpec,funcName);

				PyObject* pArgList = PyObject_GetAttrString(pArgSpec, "args");
				MYASSERT_NOT_NULL(pArgList);

				PyObject* pDefaultList = PyObject_GetAttrString(pArgSpec, "defaults");
				Py_ssize_t firstWithDefault = (Py_None==pDefaultList) ? MAX_FUNC_OPERANDS :
						PyObject_Length(pArgList) - PyObject_Length(pDefaultList);

				PythonFuncRec* pFuncRec = new PythonFuncRec(funcName, pFunc);

				for (Py_ssize_t i = 0 ; i < PyObject_Length(pArgList) ; i++) {
					char* pArgName;
					PyObject* pArg = PyList_GetItem(pArgList,i);
					PyObject* pRepr = PyObject_Repr(pArg);
#ifdef PYTHON_VER_2
					pArgName = PyString_AS_STRING(pRepr);
#else
					PyObject* pUnicode = PyUnicode_AsEncodedString(pRepr, "utf-8", "~E~");
					pArgName = PyBytes_AS_STRING(pUnicode);
#endif

					if (i>=firstWithDefault) {
						PyObject* pDef = PyTuple_GetItem(pDefaultList, i-firstWithDefault);
						if (PyLong_Check(pDef)) {
							pFuncRec->addArg(pArgName, PyLong_AsLong(pDef));
						} else if (PyInt_Check(pDef)) {
							pFuncRec->addArg(pArgName, PyInt_AsLong(pDef));
						} else if (PyFloat_Check(pDef)) {
							pFuncRec->addArg(pArgName, PyFloat_AsDouble(pDef));
						} else if (PyUnicode_Check(pDef)) {
							PyObject* pDefBytes = PyUnicode_AsASCIIString(pDef);
							pFuncRec->addArg(pArgName, PyBytes_AS_STRING(pDefBytes));
						} else if (PyString_Check(pDef)) {
							pFuncRec->addArg(pArgName, PyString_AS_STRING(pDef));
						} else {
							std::string err = std::string("Invalid default argument ") +
									std::string(pArgName) +
									" for function " + funcName;
							MYTHROW(err);
						}
					} else {
						pFuncRec->addArg(pArgName);
					}
				}
				m_Functions[funcName] = pFuncRec;
				Py_DECREF(pTuple);
			}
			Py_DECREF(pRepr);
#ifdef PYTHON_VER_3
			Py_DECREF(pStr);
#endif
		}
		Py_DECREF(pModuleKeyList);

		if (g_bVerbose) {
			std::cerr << "Python Interface: Loaded" << std::endl;
		}

		m_Initialized = true;

		// release the functions from inspect module
		Py_DECREF(pArgSpecFunc);
		Py_DECREF(pInspectMod);
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

	virtual void Debug() {
		MYASSERT(m_Initialized);
		if (m_Functions.empty()) {
			std::cerr << "Python Interface: No Python functions loaded." << std::endl;
			return;
		}

		std::cerr << "Python Interface: " << m_Functions.size() << " functions loaded:\n";
		for (auto it = m_Functions.begin() ; it != m_Functions.end() ; it++) {
			std::cerr << "\t" << it->second->getStr() << "\n";
		}
		std::cerr << std::endl;
	}

private:
	std::map<std::string,PythonFuncRec*> m_Functions;
	bool                                 m_Initialized;
	PyObject*                            m_LocalMod;
};

bool pythonInterfaceEnabled()
{
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
