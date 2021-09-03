import os,sys,argparse,subprocess,sysconfig

python_cflags=""
python_ldflags=""
python_version=0

def run_the_cmd(cmd):
	with open("xx.txt","w") as o:
		rc = subprocess.call(cmd,shell=True,stdout=o.fileno(), stderr=o.fileno())
	return rc
	
def cleanup_after_compile():
	global compiler_cleanup_cmd,platform
	with open("yy.txt","w") as o:
		subprocess.call(compiler_cleanup_cmd,shell=True,stdout=o.fileno(), stderr=o.fileno())
	if platform=="NT":
		os.system("del yy.txt")
	else:
		os.system("/bin/rm yy.txt")

def cleanup_after_python():
	global platform
	if platform=="NT":
		os.system("del test_script.py")
	else:
		os.system("/bin/rm test_script.py")

def python_search(arg):
	global python_cflags,python_ldflags,python_version,variation
	
	# Get python version
	script = '''
import platform
with open("xx.txt","w") as v:
	v.write(platform.python_version())
'''
	with open("test_script.py","w") as scriptf:
		scriptf.write(script)
	cmd = "{} test_script.py".format(arg)
	rc = os.system(cmd)
	cleanup_after_python()
	if rc!=0:
		sys.stdout.write("No -- could not get python version from {}.\n".format(arg))
		return False
	with open("xx.txt", "r") as vers:
		version_string = vers.read().strip()
		if version_string[0]=="2":
			python_version=2
		elif version_string[0]=="3":
			python_version=3
		else:
			sys.stdout.write("No -- {} is an unrecognized Python version.\n".format(version_string))
			return False
	
	# Get the result of python-config --cflags
	cmd = "{}-config --cflags".format(arg)
	rc = run_the_cmd(cmd)
	if rc!=0:
		sys.stdout.write("No -- could not get cflags from {}-config.\n".format(arg))
		return False
	with open("xx.txt", "r") as flags:
		filtered_flags = ['-g', '-O0', '-O1', '-O2', '-O3', '-Wstrict-prototypes']
		filtered_flags_debug = ['-g', '-O0', '-O1', '-O2', '-O3', '-Wstrict-prototypes', '-Wp,-D_FORTIFY_SOURCE=2']
		cflags=flags.read().strip().split()
		filter = filtered_flags_debug if variation=="DEBUG" else filtered_flags
		filtered_cflags = [f for f in cflags if f not in filter]
		python_cflags = " ".join(filtered_cflags) + " -Wno-deprecated-register -fPIC"
	
	# Get the result of python-config --cflags
	with open("xx.txt","w") as o:
		cmd = "{}-config --ldflags --embed".format(arg)  # first try with --embed needed for python 3.8
		rc = run_the_cmd(cmd)
	if rc!=0:
		cmd = "{}-config --ldflags".format(arg)
		rc = run_the_cmd(cmd)
	if rc!=0:
		sys.stdout.write("No -- could not get ldflags from {}-config.\n".format(arg))
		return False
	with open("xx.txt", "r") as flags:
		python_ldflags=flags.read().strip()
	
	sys.stdout.write("Yes.\n")
	return True

cppflags_gcc = "-Werror $(CONDCOMP) -DGITTAG=$(TAG) --std=c++11 -I ."
cppflags_clang = "-Werror $(CONDCOMP) -DGITTAG=$(TAG) -std=c++11 -I ."
cppflags_vs = "$(CONDCOMP) /DGITTAG=$(TAG) /nologo /I."

body1 = \
"""
CCSRC = $(wildcard cli/*.cc) \
		$(wildcard specitems/*.cc) \
		$(wildcard processing/*.cc) \
		$(wildcard utils/*.cc)
		
TESTSRC = $(wildcard test/*.cc)
TESTS = $(notdir $(basename $(TESTSRC)))
TEST_EXES = $(addprefix $(EXE_DIR)/,$(TESTS))
		
LIBOBJS = $(CCSRC:.cc=.{})
TESTOBJS = $(TESTSRC:.cc=.{})

#default goal
some: directories $(EXE_DIR)/specs

all: directories $(TEST_EXES)

%.obj : %.cc
	$(CXX) $(CPPFLAGS) /Fo$@ /c $<

"""

make_depends = \
"""
DEPS = $(LIBOBJS:.o=.d) $(TESTOBJS:.o=.d)

%.d: %.cc
	@$(CXX) $(CPPFLAGS) $< -MM -MT $(@:.d=.o) >$@
	
-include $(DEPS)
"""

cached_depends = "-include Makefile.cached_depends"

cached_depends_vs = "-include Makefile.cached_depends_vs"

body2 = \
"""	
run_tests: $(TEST_EXES)
	$(EXE_DIR)/TokenTest
	$(EXE_DIR)/ProcessingTest
	$(EXE_DIR)/ALUUnitTest

directories: $(EXE_DIR)

$(EXE_DIR):
	$(MKDIR_C) $@
	
$(EXE_DIR)/%: test/%.{} $(LIBOBJS)
	$(LINKER) {}$@{} {} $^ $(CONDLINK)
		
install_unix: $(EXE_DIR)/specs specs.1.gz
	cp $(EXE_DIR)/specs /usr/local/bin/
	/bin/rm */*.d
	$(MKDIR_C) /usr/local/share/man/man1
	cp specs.1.gz /usr/local/share/man/man1/
	/bin/rm specs.1.gz
	
install_win: $(EXE_DIR)/specs.exe
	echo "Please copy the file specs.exe in the EXE dir to a location on the PATH"
"""

clear_clean_posix = \
"""
clean:
	/bin/rm -rf $(EXE_DIR) */*.d */*.o specs.1.gz
	
clear:
	/bin/rm */*.d */*.o
"""

clear_clean_nt = \
"""
clean:
	del /S *.d *.o *.obj
	del /q $(EXE_DIR)\*
	rmdir $(EXE_DIR)
	
clear:
	del /S *.d *.o *.obj
"""

manpart = \
"""
specs.1.gz: ../../manpage
	cp ../../manpage specs.1
	gzip specs.1
"""

valid_compilers = ["GCC", "CLANG", "VS"]
valid_variations = ["RELEASE", "DEBUG", "PROF"]
valid_platforms = ["POSIX","NT"]

default_platform = os.name.upper()

if default_platform=="POSIX":
	default_compiler = "GCC"
elif default_platform=="NT":
	default_compiler = "VS"
else:
	sys.stderr.write("Unsupported platform: {}; Assuming POSIX\n".format(platform))
	default_platform = "POSIX"
	default_compiler = "GCC"
	
parser = argparse.ArgumentParser(description="Parse compiler and variation flags")
parser.add_argument("-c", dest="compiler", action="store", default=default_compiler, 
					help="Compiler to be used. Options: {}".format(", ".join(valid_compilers)))
parser.add_argument("-p", dest="platform", action="store", default=default_platform, 
					help="Platform to target. Options: {}".format(", ".join(valid_platforms)))
parser.add_argument("-v", dest="variation", action="store", default="RELEASE", 
					help="Variation to be used. Options: {}".format(", ".join(valid_variations)))
parser.add_argument("--use_cached_depends", dest="ucd", action="store_true", default=None,
					help="Use Cached Depends rather than re-calculating. Necessary for VS")
parser.add_argument("--fast_random", dest="nocrypt", action="store_true", default=None,
					help="Avoid cryptographic random number generators")
parser.add_argument("--os_version", dest="osversion", action="store", default="",
					help="OS version to link against. Available only in Mac OS")
parser.add_argument("--python", dest="pyprefix", action="store", default="",
                    help="Python prefix to use. 'python' is the default, optional if unspecified; 'no' means no.  Examples: 'python', 'python2', 'python3.7', 'no'")
args = parser.parse_args()

compiler = args.compiler.upper()
variation = args.variation.upper()
platform = args.platform.upper()
osversion = args.osversion if sys.platform=="darwin" else ""
use_cached_depends = args.ucd
avoid_cryptographic_random = args.nocrypt
python_prefix = args.pyprefix

# default for use_cached_depends depends on the choice of compiler
if use_cached_depends is None:
	if compiler=="VS":
		use_cached_depends = True
	else:
		use_cached_depends = False
		
if compiler=="VS":
	def_prefix = " /D"
	inc_prefix = "/I"
	lp_prefix = "/LIBPATH"
else:
	def_prefix = " -D"
	inc_prefix = "-I"
	lp_prefix = "-L"

if compiler not in valid_compilers:
	sys.stderr.write("compiler {} is not a valid compiler for specs.\nValid compilers are: {}\n".format(compiler, ", ".join(valid_compilers)))
	exit(-4)

if variation not in valid_variations:
	sys.stderr.write("Variation {} is not a valid variation for specs.\nValid variations are: {}\n".format(variation, ", ".join(valid_variations)))
	exit(-4)

if platform not in valid_platforms:
	sys.stderr.write("Platform {} is not a valid platform for specs.\nValid platforms are: {}\n".format(platform, ", ".join(valid_platforms)))
	exit(-4)
	
sys.stderr.write("Platform={}; Compiler={}; Variation={}; Cached Dependencies={}\n".format(platform,compiler,variation,use_cached_depends))

if compiler=="GCC":
	cxx = "g++"
	if variation=="RELEASE":
		condlink = "-O3"
		condcomp = "-O3"
	elif variation=="DEBUG":
		condlink = "-g"
		condcomp = "-g -DDEBUG -DALU_DUMP"
	else:
		condlink = "-O3 -g"
		condcomp = "-O3 -g"	
elif compiler=="CLANG":
	cxx = "clang++"
	if variation=="RELEASE":
		condlink = "-O3"
		condcomp = "-O3"
	elif variation=="DEBUG":
		condlink = "-g"
		condcomp = "-g -DDEBUG -DALU_DUMP"
	else:
		condlink = "-O3 -g"
		condcomp = "-O3 -g"	
elif compiler=="VS":
	cxx = "cl.exe"
	if variation=="RELEASE":
		condlink = ""
		condcomp = "/O2 /EHsc"
	elif variation=="DEBUG":
		condlink = "/MAP /DEBUG"
		condcomp = "/Zi /EHsc /DDEBUG /DALU_DUMP"
	else:
		condlink = "/Zi /MAP /DEBUG"
		condcomp = "/O2 /Zi /EHsc"	
	
if platform=="NT":
	condcomp = condcomp + "{}WIN64".format(def_prefix)
		
# Other compilers switches go here

if platform=="POSIX":
	mkdir_c = "mkdir -p"
	exe_dir = "../exe"
	clear_clean_part = clear_clean_posix
	compiler_cleanup_cmd = "/bin/rm xx.cc xx.o xx.exe xx.txt"
elif platform=="NT":
	mkdir_c = "mkdir"
	exe_dir = "..\\exe"
	clear_clean_part = clear_clean_nt
	compiler_cleanup_cmd = "del xx.cc xx.o xx.exe xx.txt"
else:
	sys.stderr.write("Logic error: platform {} is not supported.\n".format(platform))
	exit(-4)
	
if compiler=="VS":
	cppflags = cppflags_vs
	cppflags_test = "/EHsc /nologo"
elif compiler=="CLANG":
	cppflags = cppflags_clang
	cppflags_test = "-std=c++11"
else:
	cppflags = cppflags_gcc
	cppflags_test = "--std=c++11"
	
	
# Test if the compiler exists
test_compiler_exists_cmd = "{} {} -o xx.o -c xx.cc".format(cxx,cppflags_test)
with open("xx.cc", "w") as testfile:
	testfile.write('void iefbr14() {}\n')
sys.stdout.write("Testing compiler exists...")
rc = run_the_cmd(test_compiler_exists_cmd)
cleanup_after_compile()
if 0==rc:
	sys.stdout.write("Yes.\n")
else:
	sys.stdout.write("No.  Aborting...\n")
	exit(-4)
	
# Test if the compiler supports C++11
test_cpp11_cmd = "{} {} -o xx.o -c xx.cc".format(cxx,cppflags_test)
with open("xx.cc", "w") as testfile:
	testfile.write('int ret_auto_0() {auto i=0; return i;}\n')
sys.stdout.write("Testing C++11 support.....")
rc = run_the_cmd(test_cpp11_cmd)
cleanup_after_compile()
if 0==rc:
	sys.stdout.write("Yes.\n")
else:
	sys.stdout.write("No.  Aborting...\n")
	exit(-4)

# Test if the compiler supports put_time
test_put_time_cmd = "{} {} -o xx.o -c xx.cc".format(cxx,cppflags_test)
with open("xx.cc", "w") as testfile:
	testfile.write('#include <iomanip>\nvoid x() { std::put_time(NULL,""); }\n')
sys.stdout.write("Testing std::put_time()...")
if 0==run_the_cmd(test_put_time_cmd):
	sys.stdout.write("Supported.\n")
	CFG_put_time = True
else:
	sys.stdout.write("not supported.\n")
	CFG_put_time = False
cleanup_after_compile()

# Test if regex_search and regex_replace are supported
test_advanced_regex_cmd = "{} {} -o xx.exe xx.cc".format(cxx,cppflags_test)
with open("xx.cc", "w") as testfile:
	testfile.write('#include <regex>\n')
	testfile.write('int main(int argc, char** argv) {\n')
	testfile.write('    static const char st[]="this subject has a submarine as a subsequence";\n')
	testfile.write('    std::regex e ("\\\\b(sub)([^ ]*)");   // matches words beginning by "sub"\n')
	testfile.write('    bool b = std::regex_search(st,e);\n')
	testfile.write('    return b ? 0 : -4;\n')
	testfile.write('}\n')
sys.stdout.write("Testing std::regex_search()...")
if 0==run_the_cmd(test_advanced_regex_cmd):
	sys.stdout.write("Compiled...")
	test_advanced_regex_cmd = "./xx.exe" if platform!="NT" else "xx.exe"
	if 0==run_the_cmd(test_advanced_regex_cmd):
		sys.stdout.write("Supported\n")
		CFG_advanced_regex = True
	else:
		sys.stdout.write("Not supported\n")
		CFG_advanced_regex = False
else:
	sys.stdout.write("Internal error.\n")
	exit(-4)
cleanup_after_compile()
	
# Test if the environment supports a Spanish locale
test_spanish_locale_cmd = "{} {} -o xx.exe xx.cc".format(cxx,cppflags_test)
with open("xx.cc", "w") as testfile:
	testfile.write('#include <locale>\nint main(int argc, char** argv){std::locale l("es_ES"); return 0;}')
sys.stdout.write("Testing Spanish locale (for unit tests)...")
if 0==run_the_cmd(test_spanish_locale_cmd):
	sys.stdout.write("Compiled...")
	test_spanish_locale_cmd = "./xx.exe" if platform!="NT" else "xx.exe"
	if 0==run_the_cmd(test_spanish_locale_cmd):
		sys.stdout.write("Supported\n")
		CFG_spanish_locale = True
	else:
		sys.stdout.write("Not supported\n")
		CFG_spanish_locale = False
else:
	sys.stdout.write("Internal error.\n")
	exit(-4)
cleanup_after_compile()
	
# Test if the environment contains a random number generator
found_random_source = False
rand_source = "rand"

test_rand_cmd = "{} {} -o xx.o -c xx.cc".format(cxx,cppflags_test)
with open("xx.cc", "w") as testfile:
	testfile.write("#include <CommonCrypto/CommonRandom.h>\nvoid x() {}\n")
sys.stdout.write("Testing if the CommonCrypto library is available...")
if 0==run_the_cmd(test_rand_cmd):
	sys.stdout.write("Yes.\n")
	if not avoid_cryptographic_random:
		rand_source="CommonCrypto"
		found_random_source = True
else:
	sys.stdout.write("No.\n")
cleanup_after_compile()

test_rand_cmd = "{} {} -o xx.o -c xx.cc".format(cxx,cppflags_test)
with open("xx.cc", "w") as testfile:
	testfile.write("#include <windows.h>\n#include <wincrypt.h>\nvoid x() {}\n")
sys.stdout.write("Testing if the wincrypt library is available...")
if 0==run_the_cmd(test_rand_cmd):
	sys.stdout.write("Yes.\n")
	if not found_random_source and not avoid_cryptographic_random:
		rand_source="wincrypt"
		found_random_source = True
else:
	sys.stdout.write("No.\n")
cleanup_after_compile()
	
test_rand_cmd = "{} {} -o xx.o -c xx.cc".format(cxx,cppflags_test)
with open("xx.cc", "w") as testfile:
	testfile.write("#include <stdlib.h>\nint x() { return drand48_r(NULL, NULL); }\n")
sys.stdout.write("Testing if the rand48 extension to stdlib is available...")
if 0==run_the_cmd(test_rand_cmd):
	sys.stdout.write("Yes.\n")
	if not found_random_source:
		rand_source="rand48"
		found_random_source = True
else:
	sys.stdout.write("No.\n")
cleanup_after_compile()
	
#
# Python support
sys.stdout.write("Testing if python support is available...")	
if platform=="NT":
	if python_prefix=="no":
		sys.stdout.write("Python support configured off.\n")
		CFG_python = False
	else:
		cv = sysconfig.get_config_vars()
		python_prefix = cv['prefix']
		python_ver = cv['py_version_nodot']
		python_version = int(python_ver[0])
		python_cflags = '{} "{}\\include"'.format(inc_prefix,python_prefix)
		python_ldflags = '"{}\\libs\\python{}.lib"'.format(python_prefix,python_ver)
		sys.stdout.write("configured for Python version {}.\n".format(cv['py_version']))
		CFG_python = True
else:
	if python_prefix=="": # default: python is optional and prefix is 'python'
		CFG_python = python_search("python")
		os.system("/bin/rm xx.txt")

	elif python_prefix=="no":
		sys.stdout.write("Python support configured off.\n")
		CFG_python = False
	else:
		if python_prefix=="yes":
			python_prefix = "python"

		rc = python_search(python_prefix)
		run_the_cmd("/bin/rm xx.txt")
		if rc:
			CFG_python = True
		else:
			sys.stdout.write("\nFailed to make Makefile. Python support is not properly configured.\n")
			exit(-4)

# RegEx different grammars
CFG_regex_grammars = (sys.platform=="darwin")
	
if CFG_put_time:
	condcomp = condcomp + "{}PUT_TIME__SUPPORTED".format(def_prefix)
	
if CFG_spanish_locale:
	condcomp = condcomp + "{}SPANISH_LOCALE_SUPPORTED".format(def_prefix)

if CFG_advanced_regex:
	condcomp = condcomp + "{}ADVANCED_REGEX_FUNCTIONS".format(def_prefix)
	
if rand_source is not None:
	condcomp = condcomp + "{}ALURAND_{}".format(def_prefix,rand_source)
	
if CFG_python:
	condcomp = condcomp + " " + python_cflags + "{}PYTHON_VER_{}".format(def_prefix,python_version)
	condlink = condlink + " " + python_ldflags
else:
	condcomp = condcomp + "{}SPECS_NO_PYTHON".format(def_prefix)
	
if CFG_regex_grammars:
	condcomp = condcomp + "{}REGEX_GRAMMARS".format(def_prefix)

if osversion != "":
	condlink = condlink + " -mmacosx-version-min={}".format(osversion)

with open("Makefile", "w") as makefile:
	makefile.write("CXX={}\n".format(cxx))
	makefile.write("LINKER={}\n".format("link.exe" if (compiler=="VS") else cxx))
	makefile.write("CONDCOMP={}\n".format(condcomp))
	makefile.write("CONDLINK={}\n".format(condlink))
	makefile.write("MKDIR_C={}\n".format(mkdir_c))
	makefile.write("EXE_DIR={}\n".format(exe_dir))
	makefile.write("TAG := $(shell git describe --abbrev=0 --tags)\n\n")
	makefile.write("CPPFLAGS = {}\n".format(cppflags))
	
	if compiler=="VS":
		body1fmt = body1.format("obj","obj")
		body2fmt = body2.format("obj","/OUT:",".exe","advapi32.lib")
	elif compiler=="CLANG":
		body1fmt = body1.format("o","o")
		body2fmt = body2.format("o", "-o ", "", "-pthread")
	else:
		body1fmt = body1.format("o","o")
		body2fmt = body2.format("o", "-o ", "", "-pthread")
	
	makefile.write("{}\n".format(body1fmt))
	if use_cached_depends:
		if compiler=="VS":
			makefile.write("\n{}\n".format(cached_depends_vs))
		else:
			makefile.write("\n{}\n".format(cached_depends))
	else:
		makefile.write("\n{}\n".format(make_depends))
	makefile.write("{}\n".format(body2fmt))
	makefile.write("{}\n".format(clear_clean_part))
	
	if platform!="NT":
		makefile.write("{}\n\ninstall: install_unix\n".format(manpart))
	else:
		makefile.write("install: install_win\n")
		
sys.stderr.write("Makefile created.\n")
