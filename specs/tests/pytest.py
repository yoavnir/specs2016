import os,sys

def run_cmd(spec):
	cmd = "../exe/specs --set SPECSPATH=/tmp -o theout " + spec + "&> theerr"
	rc = os.system(cmd)
	if rc!=0:
		ret = "RC="+str(rc)
	elif os.path.exists("theout"):
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
sys.stdout.write("Test #1 -- ")
ret = run_cmd('print "sqrt(16)" 1')
if ret=="4":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")
	
# while still not having any file there, try calling the kuku function
sys.stdout.write("Test #2 -- ")
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
sys.stdout.write("Test #3 -- ")
ret = run_cmd('print "sqrt(81)" 1')
if ret=="9":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")

# But as soon as we try to load it...
sys.stdout.write("Test #4 -- ")
ret = run_cmd('print "kuku(16)" 1')
if ret=="Python Interface: Error loading local functions":
	sys.stdout.write("OK\n")
else:
	sys.stdout.write("Not OK: <"+ret+">\n")


	