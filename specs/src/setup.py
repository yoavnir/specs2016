import os,sys,argparse

cppflags_other = "CPPFLAGS = -Werror $(CONDCOMP) -DGITTAG=$(TAG) --std=c++11 -I ."
cppflags_vs = "CPPFLAGS = $(CONDCOMP) /DGITTAG=$(TAG) /I."

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
	$(CXX) $(CONDLINK) {}$@ {} $^
		
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
	del $(EXE_DIR)\*.exe
	del $(EXE_DIR)\*.ilk
	del $(EXE_DIR)\*.pdb
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

if default_platform=="POSIX" or default_platform=="NT":
	default_compiler = "GCC"
else:
	sys.stderr.write("Unsupported platform: {}; Assuming POSIX\n".format(platform))
	default_platform = "POSIX"
	
parser = argparse.ArgumentParser(description="Parse compiler and variation flags")
parser.add_argument("-c", dest="compiler", action="store", default=default_compiler, 
					help="Compiler to be used. Options: {}".format(", ".join(valid_compilers)))
parser.add_argument("-p", dest="platform", action="store", default=default_platform, 
					help="Platform to target. Options: {}".format(", ".join(valid_platforms)))
parser.add_argument("-v", dest="variation", action="store", default="RELEASE", 
					help="Variation to be used. Options: {}".format(", ".join(valid_variations)))
parser.add_argument("--use_cached_depends", dest="ucd", action="store_true", default=None,
					help="Use Cached Depends rather than re-calculating. Necessary for VS")
args = parser.parse_args()

compiler = args.compiler.upper()
variation = args.variation.upper()
platform = args.platform.upper()
use_cached_depends = args.ucd

# default for use_cached_depends depends on the choice of compiler
if use_cached_depends is None:
	if compiler=="VS":
		use_cached_depends = True
	else:
		use_cached_depends = False

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
		condlink = "/O2"
		condcomp = "/O2 /EHsc"
	elif variation=="DEBUG":
		condlink = "/MAP /DEBUG"
		condcomp = "/Zi /EHsc /DDEBUG /DALU_DUMP"
	else:
		condlink = "/O2 /Zi /MAP /DEBUG"
		condcomp = "/O2 /Zi /EHsc"	
	
if platform=="NT":
	condcomp = condcomp + (" -DWIN64" if compiler!="VS" else " /DWIN64")
		
# Other compilers switches go here

if platform=="POSIX":
	mkdir_c = "mkdir -p"
	exe_dir = "../exe"
	clear_clean_part = clear_clean_posix
elif platform=="NT":
	mkdir_c = "mkdir"
	exe_dir = "..\\exe"
	clear_clean_part = clear_clean_nt
else:
	sys.stderr.write("Logic error: platform {} is not supported.\n".format(platform))
	exit(-4)




with open("Makefile", "w") as makefile:
	makefile.write("CXX={}\n".format(cxx))
	makefile.write("CONDCOMP={}\n".format(condcomp))
	makefile.write("CONDLINK={}\n".format(condlink))
	makefile.write("MKDIR_C={}\n".format(mkdir_c))
	makefile.write("EXE_DIR={}\n".format(exe_dir))
	makefile.write("TAG := $(shell git describe --abbrev=0 --tags)\n\n")
	
	if compiler=="VS":
		makefile.write("{}\n".format(cppflags_vs))
		body1fmt = body1.format("obj","obj")
		body2fmt = body2.format("obj","-o ","")
	else:
		makefile.write("{}\n".format(cppflags_other))
		body1fmt = body1.format("o","-o ")
		body2fmt = body2.format("o", "-o ", "-pthread")    # should be "/OUT:" but I haven't got it to work yet
	
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
