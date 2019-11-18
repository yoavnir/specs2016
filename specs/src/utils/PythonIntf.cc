#include "PythonIntf.h"

#ifndef SPECS_NO_PYTHON

#include <Python.h>

bool pythonInterfaceEnabled()
{
	return true;
}

#else  // SPECS_NO_PYTHON

bool pythonInterfaceEnabled()
{
	return false;
}

#endif

