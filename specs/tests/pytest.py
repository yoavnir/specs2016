import os,sys

def run_cmd(spec, force=False):
	if force:
		cmd = "../exe/specs --pythonFuncs on --set SPECSPATH=/tmp -o theout " + spec + "&> theerr"
	else:
		cmd = "../exe/specs --set SPECSPATH=/tmp -o theout " + spec + "&> theerr"
	rc = os.system(cmd)
	if rc!=0 and rc!=2048:
		ret = "RC="+str(rc)
	elif rc==0 and os.path.exists("theout"):
		with open("theout","r") as out:
			ret = out.read()
		os.system("/bin/rm theout")
	elif os.path.exists("theerr"):
		with open("theerr","r") as err:
			ret = err.readlines()[-1]
		os.system("/bin/rm theerr")
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
	
# while still not having any file there, try calling the kuku function
sys.stdout.write("Test 02 (unknown function; no file) -- ")
ret = run_cmd('print "kuku(16)" 1')
if ret=="Unrecognized function kuku":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	
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

# But if we force it...
sys.stdout.write("Test 04 (bad file; non-python function; force) -- ")
ret = run_cmd('print "sqrt(81)" 1', True)
if ret=="Python Interface: Error loading local functions":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")

# Or call a non-built-in function...
sys.stdout.write("Test 05 (bad file; unknown function) -- ")
ret = run_cmd('print "kuku(16)" 1')
if ret=="Python Interface: Error loading local functions":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")

# Now for a valid file
lff = '''
from math import factorial

def plus1(a):
	return a+1
	
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

# FP parameter
sys.stdout.write("Test 07 (float parameter) -- ")
ret = run_cmd('print "plus1(3.2)" 1')
if ret=="4.2":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")

# string parameter - should abend
sys.stdout.write("Test 08 (bad parameter; should abend) -- ")
ret = run_cmd('print "plus1(\'hello\')" 1')
if ret=="Runtime error. Error in external function":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")

# a function with memory
sys.stdout.write("Test 09 (function with memory; first run) -- ")
ret = run_cmd('print "called_how_many_times()" 1')
if ret=="1":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")

# a function with memory
sys.stdout.write("Test 10 (function with memory; second run) -- ")
ret = run_cmd('print "called_how_many_times()" . print "called_how_many_times()" 1')
if ret=="2":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")

# a function that does not exist
sys.stdout.write("Test 11 (non-existent function) -- ")
ret = run_cmd('print "plus2(3)" 1')
if ret=="Unrecognized function plus2":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	
# call a python imported function
sys.stdout.write("Test 12 (imported function) -- ")
ret = run_cmd('print "factorial(5)" 1')
if ret=="120":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
