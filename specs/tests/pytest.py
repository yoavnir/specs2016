import os,sys,subprocess

def run_cmd(spec, force=False):
	args = ["../exe/specs", "--set", "SPECSPATH=/tmp", "-o", "theout"]
	if force:
		args.extend(["--pythonFuncs", "on"])
	args.append(spec)
	
	with open("theerr", "w") as err_file:
		rc = subprocess.call(args, stdout=subprocess.DEVNULL, stderr=err_file)
	
	if os.path.exists("theerr"):
		with open("theerr", "r") as f:
			theerr_content = f.readlines()
		os.system("/bin/rm theerr")
	else:
		theerr_content = ""

	if os.path.exists("theout"):
		with open("theout", "r") as f:
			theout_content = f.read()
		os.system("/bin/rm theout")
	else:
		theout_content = ""

	if rc!=0 and rc!=252 and rc!=8:
		ret = "RC="+str(rc)
	elif (rc==0 or rc==252) and theout_content != "":
		ret = theout_content
	elif theerr_content != "":
		ret = theerr_content[-1]
	else:
		ret = "something happened"

	return ret.strip()
	
def set_localfuncs(lf):
	if os.path.exists("/tmp/localfuncs.py"):
		os.system("/bin/rm /tmp/localfuncs.py")
	if lf is None:
		return
	with open("/tmp/localfuncs.py", "w") as lf_file:
		lf_file.write(lf)

# main

set_localfuncs(None)

# warm up with a non-python function
sys.stdout.write("Test 01 (non-python function) -- ")
ret = run_cmd('print "sqrt(16)" 1')
if ret=="4":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)
	
# while still not having any file there, try calling the kuku function
sys.stdout.write("Test 02 (unknown function; no file) -- ")
ret = run_cmd('print "kuku(16)" 1')
if ret=="Unrecognized function kuku":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)
	
# So let's try loading an invalid file
lff = '''
dek plus1(a):
	return a+1
'''
set_localfuncs(lff)

# With a known function, we shouldn't notice
sys.stdout.write("Test 03 (bad file; non-python function) -- ")
ret = run_cmd('print "sqrt(81)" 1')
if ret=="9":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# But if we force it...
sys.stdout.write("Test 04 (bad file; non-python function; force) -- ")
ret = run_cmd('print "sqrt(81)" 1', True)
if ret=="Python Interface: Error loading local functions" or ret.startswith("SyntaxError: invalid syntax"):
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Or call a non-built-in function...
sys.stdout.write("Test 05 (bad file; unknown function) -- ")
ret = run_cmd('print "kuku(16)" 1')
if ret=="Python Interface: Error loading local functions" or ret.startswith("SyntaxError: invalid syntax"):
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Now for a valid file
lff = '''
from math import factorial as _factorial

def plus1(a):
	return a+1

def factorial(n):
	return _factorial(int(n))
	
calling_count = 0
def called_how_many_times():
	global calling_count
	calling_count = calling_count + 1
	return calling_count
'''
set_localfuncs(lff)

# Simply run the function
sys.stdout.write("Test 06 (simple function) -- ")
ret = run_cmd('print "plus1(3)" 1')
if ret=="4":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# FP parameter
sys.stdout.write("Test 07 (float parameter) -- ")
ret = run_cmd('print "plus1(3.2)" 1')
if abs(float(ret)-4.2) < 0.0001:
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# string parameter - should abend
sys.stdout.write("Test 08 (bad parameter; should abend) -- ")
ret = run_cmd('print "plus1(\'hello\')" 1')
if ret=="Runtime error. Error in external function":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# a function with memory
sys.stdout.write("Test 09 (function with memory; first run) -- ")
ret = run_cmd('print "called_how_many_times()" 1')
if ret=="1":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# a function with memory
sys.stdout.write("Test 10 (function with memory; second run) -- ")
ret = run_cmd('print "called_how_many_times()" . print "called_how_many_times()" 1')
if ret=="2":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# a function that does not exist
sys.stdout.write("Test 11 (non-existent function) -- ")
ret = run_cmd('print "plus2(3)" 1')
if ret=="Unrecognized function plus2":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)
	
# call a python imported function
sys.stdout.write("Test 12 (imported function) -- ")
ret = run_cmd('print "factorial(5)" 1')
if ret=="120":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test exactness feature - exact float
lff = '''
def exact_float():
	"""Return an exact floating-point value"""
	return (3.14159, True)

def inexact_float():
	"""Return an inexact floating-point value"""
	return (3.14159, False)

def inexact_int():
	"""Return an inexact integer (overriding default)"""
	return (42, False)

def bad_tuple_size():
	"""Return a tuple with wrong size"""
	return (1, 2, 3)

def bad_exactness_type():
	"""Return a tuple with non-bool exactness"""
	return (1.5, 1)

def lowindex(x):
	"""Returns the lower 16 bits of the number, and copies exactness"""
	return (int(x[0]) % 65536, x[1])

lowindex.arg_type = "exact"

def highindex(x):
	"""Returns the top 16 bits"""
	return int(x) // 65536

def bad_exact_use(x):
	"""Tries to use a tuple arg as a number - will TypeError"""
	return x + 1

bad_exact_use.arg_type = "exact"

def bad_plain_use(x):
	"""Tries to index into a plain value - will TypeError"""
	return x[0]
'''
set_localfuncs(lff)

# Test exact float with True
sys.stdout.write("Test 13 (exact float with True) -- ")
ret = run_cmd('print "exact(exact_float())" 1')
if ret=="1":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test exact float with False
sys.stdout.write("Test 14 (inexact float with False) -- ")
ret = run_cmd('print "exact(inexact_float())" 1')
if ret=="0":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test overriding default exact int with False
sys.stdout.write("Test 15 (inexact int override) -- ")
ret = run_cmd('print "exact(inexact_int())" 1')
if ret=="0":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test bad tuple size
sys.stdout.write("Test 16 (bad tuple size) -- ")
ret = run_cmd('print "bad_tuple_size()" 1')
if "Invalid tuple returned from function bad_tuple_size" in ret:
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test bad exactness type
sys.stdout.write("Test 17 (bad exactness type) -- ")
ret = run_cmd('print "bad_exactness_type()" 1')
if "Invalid exactness value returned from function bad_exactness_type" in ret:
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test lowindex with inexact argument
sys.stdout.write("Test 18 (lowindex with inexact arg) -- ")
ret = run_cmd('print "exact(lowindex(4/3))" 1')
if ret=="0":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test lowindex with exact argument
sys.stdout.write("Test 19 (lowindex with exact arg) -- ")
ret = run_cmd('print "exact(lowindex(4))" 1')
if ret=="1":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test highindex without arg_type (normal behavior)
sys.stdout.write("Test 20 (highindex without arg_type) -- ")
ret = run_cmd('print "exact(highindex(4/3))" 1')
if ret=="1":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test combined spec from problem statement
sys.stdout.write("Test 21 (combined spec) -- ")
ret = run_cmd('print "exact(lowindex(4/3))" 1 print "exact(lowindex(4))" NEXTWORD print "exact(highindex(4/3))" NEXTWORD')
if ret=="0 1 1":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test bad_exact_use - function tries to use tuple arg as number
sys.stdout.write("Test 22 (bad_exact_use - TypeError) -- ")
ret = run_cmd('print "bad_exact_use(5)" 1')
if "Runtime error. Error in external function" in ret:
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test bad_plain_use - function tries to index plain value
sys.stdout.write("Test 23 (bad_plain_use - TypeError) -- ")
ret = run_cmd('print "bad_plain_use(5)" 1')
if "Runtime error. Error in external function" in ret:
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

# Test bad_arg_type - invalid arg_type value
sys.stdout.write("Test 24 (bad_arg_type - invalid value) -- ")
lff_bad = '''
def bad_arg_type():
	"""Function with invalid arg_type value"""
	return 42

bad_arg_type.arg_type = "bogus"
'''
set_localfuncs(lff_bad)
ret = run_cmd('print "bad_arg_type()" 1', True)
if "Invalid arg_type value for function bad_arg_type" in ret:
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	exit(4)

sys.stdout.write("\n*** All 24 tests passed.\n")
