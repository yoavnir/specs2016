MKDIR_C = mkdir -p
EXE_DIR = ../exe
INC = -I .

# FOR DEBUG
# CONDCOMP = -g -DDEBUG
# CONDLINK = -g

# FOR RELEASE
CONDLINK =  -O3
CONDCOMP =  -O3

TAG := $(shell git describe --abbrev=0 --tags)

CXX = g++
CPPFLAGS = -Werror $(CONDCOMP) -DGITTAG=$(TAG) --std=c++11 $(INC)

CCSRC = $(wildcard cli/*.cc) \
		$(wildcard specitems/*.cc) \
		$(wildcard processing/*.cc) \
		$(wildcard utils/*.cc)
		
TESTSRC = $(wildcard test/*.cc)
TESTS = $(notdir $(basename $(TESTSRC)))
TEST_EXES = $(addprefix ../exe/,$(TESTS))
		
LIBOBJS = $(CCSRC:.cc=.o)
TESTOBJS = $(TESTSRC:.cc=.o)

#default goal
some: directories $(EXE_DIR)/specs

all: directories $(TEST_EXES)

DEPS = $(LIBOBJS:.o=.d) $(TESTOBJS:.o=.d)

%.d: %.cc
	@$(CXX) $(CPPFLAGS) $< -MM -MT $(@:.d=.o) >$@
	
-include $(DEPS)
	
run_tests: $(TEST_EXES)
	$(EXE_DIR)/TokenTest
	$(EXE_DIR)/ProcessingTest
	$(EXE_DIR)/ALUUnitTest

directories: $(EXE_DIR)

$(EXE_DIR):
	$(MKDIR_C) $@
	
../exe/%: test/%.o $(LIBOBJS)
	$(CXX) $(CONDLINK) -o $@ $^		

clean:
	/bin/rm -rf $(EXE_DIR) */*.d */*.o
	
clear:
	/bin/rm */*.d */*.o
	
install: $(EXE_DIR)/specs
	cp $(EXE_DIR)/specs /usr/local/bin/
