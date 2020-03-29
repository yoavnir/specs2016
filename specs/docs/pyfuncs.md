# Python Functions

Python functions are a new feature in version 0.6. They allow the user to add to **specs** [built-in functions](alu_adv.md) by writing some in the python language. While the built-in functions provide rich functionality, particular users may want to add some more. Here's how.

All external functions should exist in a single file called `localfuncs.py`. This file should reside in the `SPECSPATH`, the same path of the specification files. This file is a regular Python file that you should be able to **import** from the python environment.  All functions that do not begin with an underscore (`_`) are available to **specs**. Conversely, name any helper function with an underscore as the first character.

## Example
Suppose our input is a list of integer numbers like this:
```
1
8400
1398234
45382
```
We'd like to print them out with commas separating the thousands like so:
`specs print "commas(word(1))" 1.12 RIGHT`
and get this:
```
           1
       8,400
   1,398,234
      45,382
```
Of course, **specs** does not have a *commas* function.  But now we can add it.  Just edit the `localfuncs.py` file and add the following function:
```
def commas(x):
	x = int(x)
	ret = ""
	while x>=1000:
		rm = str(x % 1000)
		x = x // 1000
		while len(rm) < 3:
			rm = "0" + rm
		ret = "," + rm + ret
	if x>0:
		ret = str(x) + ret
	return ret
```
Simple, right. And gets the job done.

And you can `import` other python modules.  It's all available to you.  Just make sure that your function returns an integer, a floating-point number, a string, or `nil`.

## Options

When compiling **specs**, you can use the `--python` switch to the `setup.py` script to choose the python version (such as `python2`, `python3.8`, etc, depending on the prefix installed on your system), or choose the string `no` to compile **specs** without Python support.

In use, you have two relevant command-line switches:
* **--pythonFuncs** on/off/**auto** - determines whether or not to load the python functions. **auto**, which is the default means that Python functions will be loaded only if a function call was found that is not a known built-in function.
* **--pythonErr** zero/nan/nullstr/**throw** - determines what **specs** will do if the Python function encounters an error and ends abnormally. The default behavior is that **specs** will throw an exception and terminate, reflecting as much as it can of the Python error. The other options are to pretend that the Python function returned an integer zero, a *NaN*, or an empty string respectively.

## Docstrings

Python supports documenting functions through **docstrings** as described [PEP 257](https://www.python.org/dev/peps/pep-0257/) 

**specs** Python functions can be documented just like any other function. For example, the previous `commas` function can be documented like this:
```
def commas(x):
	''' Convert the integer x into a string with thousands groups separated by commas'''
	x = int(x)
	ret = ""
	while x>=1000:
		rm = str(x % 1000)
		x = x // 1000
		while len(rm) < 3:
			rm = "0" + rm
		ret = "," + rm + ret
	if x>0:
		ret = str(x) + ret
	return ret
```
With the `--help pyfuncs` **specs** will print out the docstring:
```
$ specs --help pyfuncs

Python Interface Functions: 
===========================
- commas (x) :  Convert the integer x into a string with thousands groups separated by commas
```

## Advanced functions
