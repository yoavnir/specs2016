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

Functions written in Python can be as complex as you like. What's more, such functions need not be stateless. Consider the following example. The `countocc` function will tell us how many times the `needle` appears in the `haystack`.  Here's how to do it:
```
occ_dict = dict()
def countocc(haystack,needle):
	'''
	This function counts how many time each needle was found in the haystack in invocations
	of this function. The function returns the current count of matches
	'''
	global occ_dict  # make the global struct available to the function
	
	if not needle in occ_dict.keys():
		occ_dict[needle] = 0
	if haystack.find(needle) >= 0:
		occ_dict[needle] = occ_dict[needle] + 1
	
	return occ_dict[needle]
```
So how would we use this?
```
specs set "#0:=countocc(@@,'hello')"  EOF print "#0" 1
```
This counts the lines in the input that included the word 'hello'.

## Exactness

By default, **specs** applies a heuristic to determine whether a Python function's return value is exact or inexact:
- Integer returns are marked as **exact**
- String returns are marked as **exact**
- Floating-point returns are marked as **inexact**

This heuristic may not always be correct. For example, a function that computes π should return an inexact value, but a function that computes a well-defined mathematical constant might return an exact value.

To override the default heuristic, a Python function can return a 2-tuple instead of a plain value:
```python
def exact_pi():
    '''Return an exact value of pi'''
    return (3.141592653589793, True)

def inexact_sqrt():
    '''Return an inexact square root'''
    return (2.23606797749979, False)
```

The tuple must have exactly 2 elements:
1. The first element is the return value (integer, float, string, or None)
2. The second element is a Python boolean: `True` for exact, `False` for inexact

If the tuple is malformed (wrong number of elements, or second element is not a boolean), **specs** will report an error.

For example:
```python
def good_exact():
    return (42, True)  # OK: exact integer

def good_inexact():
    return (3.14, False)  # OK: inexact float

def bad_tuple_size():
    return (1, 2, 3)  # ERROR: tuple has 3 elements, not 2

def bad_exactness_type():
    return (1.5, 1)  # ERROR: second element is int, not bool
```

The `exact()` built-in function can be used to check whether a value is exact:
```python
specs print "exact(exact_pi())" 1
```
would print `1` (true), while:
```python
specs print "exact(inexact_sqrt())" 1
```
would print `0` (false).

## Argument Exactness

By default, Python functions receive their arguments as plain values, and exactness information is discarded. However, you can configure a function to receive exactness information with its arguments by setting the `arg_type` attribute to `"exact"`.

### Setting `arg_type = "exact"`

To enable argument exactness, add this line after your function definition:

```python
def my_function(x, y):
    # function body
    pass

my_function.arg_type = "exact"
```

When `arg_type = "exact"` is set, **all arguments** are passed as 2-tuples instead of plain values:
- The first element is the argument value (integer, float, string, or None)
- The second element is a Python boolean: `True` if the value is exact, `False` if inexact

### Example: Propagating Exactness

Here's a practical example that propagates exactness information:

```python
def lowindex(x):
    '''Returns the lower 16 bits of the number, and copies exactness'''
    return (int(x[0]) % 65536, x[1])

lowindex.arg_type = "exact"

def highindex(x):
    '''Returns the top 16 bits'''
    return int(x) // 65536
```

In this example:
- `lowindex` receives its argument as a 2-tuple `(value, exactness)` and returns a 2-tuple preserving the exactness
- `highindex` receives a plain value (no `arg_type` set) and returns a plain integer

You can verify the behavior with:

```
specs print "exact(lowindex(4/3))" 1 print "exact(lowindex(4))" NEXTWORD print "exact(highindex(4/3))" NEXTWORD
```

This prints `0 1 1`:
- `lowindex(4/3)` receives an inexact value (4/3), returns it with exactness=0
- `lowindex(4)` receives an exact value (4), returns it with exactness=1
- `highindex(4/3)` receives a plain value, returns an integer (which is exact by default), so exactness=1

### Combining Argument and Return Exactness

A function can both receive argument exactness and return exactness information. For example:

```python
def add_exact(a, b):
    '''Add two numbers, exact only if both inputs are exact'''
    return (a[0] + b[0], a[1] and b[1])

add_exact.arg_type = "exact"
```

This function:
1. Receives both arguments as 2-tuples (because `arg_type = "exact"`)
2. Returns a 2-tuple with the sum and a boolean indicating exactness (both inputs must be exact)

### Error Handling

If `arg_type` is set to a value other than `"exact"`, **specs** will report an error during initialization:

```python
def bad_function(x):
    pass

bad_function.arg_type = "bogus"  # ERROR: Invalid arg_type value
```

Also, if a function has `arg_type = "exact"` but tries to use an argument as a plain value (or vice versa), Python will raise a `TypeError`:

```python
def bad_use(x):
    return x + 1  # ERROR: can't add tuple + int

bad_use.arg_type = "exact"
```

When such errors occur, **specs** will report them as external function errors.

