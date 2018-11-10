MKDIR_C = mkdir -p
EXE_DIR = ../exe
OBJ_DIR = ../obj
INC = -I .
COMP = g++ -g -DDEBUG --std=c++11 $(INC)
LINK = g++ -g

CLI_OBJ = $(OBJ_DIR)/tokens.o $(OBJ_DIR)/splitter.o
SI_OBJ = $(OBJ_DIR)/specItems.o $(OBJ_DIR)/dataField.o $(OBJ_DIR)/InputPart.o
PR_OBJ = $(OBJ_DIR)/conversions.o $(OBJ_DIR)/Reader.o $(OBJ_DIR)/Writer.o $(OBJ_DIR)/ProcessingState.o $(OBJ_DIR)/StringBuilder.o $(OBJ_DIR)/Config.o
UT_OBJ = $(OBJ_DIR)/SpecString.o $(OBJ_DIR)/ErrorReporting.o

.DUMMY: directories

all: directories tests run_tests $(EXE_DIR)/specs

run_tests: tests
	$(EXE_DIR)/tokenTest
	$(EXE_DIR)/ProcessingTest

tests: $(EXE_DIR)/tokenTest $(EXE_DIR)/itemTest $(EXE_DIR)/readWriteTest $(EXE_DIR)/ProcessingTest

directories: $(EXE_DIR) $(OBJ_DIR)

$(EXE_DIR):
	$(MKDIR_C) $@

$(OBJ_DIR):
	$(MKDIR_C) $@

$(EXE_DIR)/tokenTest: $(OBJ_DIR)/TokenTest.o $(CLI_OBJ) $(PR_OBJ) $(UT_OBJ)
	$(COMP) -o $@ $(OBJ_DIR)/TokenTest.o $(CLI_OBJ) $(PR_OBJ) $(UT_OBJ)

$(EXE_DIR)/itemTest: $(OBJ_DIR)/itemTest.o $(CLI_OBJ) $(SI_OBJ) $(PR_OBJ) $(UT_OBJ)
	$(COMP) -o $@ $(OBJ_DIR)/itemTest.o $(CLI_OBJ) $(SI_OBJ) $(PR_OBJ) $(UT_OBJ)

$(EXE_DIR)/readWriteTest: $(OBJ_DIR)/readWriteTest.o $(PR_OBJ) $(UT_OBJ)
	$(COMP) -o $@ $(OBJ_DIR)/readWriteTest.o $(PR_OBJ) $(UT_OBJ)
	
$(EXE_DIR)/ProcessingTest: $(OBJ_DIR)/ProcessingTest.o $(PR_OBJ) $(UT_OBJ)
	$(COMP) -o $@ $(OBJ_DIR)/ProcessingTest.o $(PR_OBJ) $(UT_OBJ) $(CLI_OBJ) $(SI_OBJ)
	
$(EXE_DIR)/specs: $(OBJ_DIR)/Main.o $(CLI_OBJ) $(PR_OBJ) $(SI_OBJ) $(UT_OBJ)
	$(COMP) -o $@ $(OBJ_DIR)/Main.o $(CLI_OBJ) $(PR_OBJ) $(SI_OBJ) $(UT_OBJ) 
	
$(OBJ_DIR)/tokens.o: cli/tokens.cc cli/tokens.h
	$(COMP) -o $@ -c cli/tokens.cc
	
$(OBJ_DIR)/splitter.o: cli/splitter.cc $(OBJ_DIR)/tokens.o
	$(COMP) -o $@ -c cli/splitter.cc

$(OBJ_DIR)/specItems.o: specitems/specItems.cc specitems/specItems.h specitems/item.h
	$(COMP) -o $@ -c specitems/specItems.cc

$(OBJ_DIR)/dataField.o: specitems/dataField.cc specitems/item.h processing/conversions.h
	$(COMP) -o $@ -c specitems/dataField.cc
	
$(OBJ_DIR)/InputPart.o: specitems/InputPart.cc specitems/item.h 
	$(COMP) -o $@ -c specitems/InputPart.cc
	
$(OBJ_DIR)/conversions.o: processing/conversions.cc processing/conversions.h
	$(COMP) -o $@ -c processing/conversions.cc
	
$(OBJ_DIR)/TokenTest.o: test/TokenTest.cc cli/tokens.h
	$(COMP) -o $@ -c test/TokenTest.cc

$(OBJ_DIR)/itemTest.o: test/itemTest.cc cli/tokens.h specitems/specItems.h
	$(COMP) -o $@ -c test/itemTest.cc
	
$(OBJ_DIR)/readWriteTest.o: test/readWriteTest.cc processing/Reader.h processing/Writer.h
	$(COMP) -o $@ -c test/readWriteTest.cc

$(OBJ_DIR)/ProcessingTest.o: test/ProcessingTest.cc processing/ProcessingState.h
	$(COMP) -o $@ -c test/ProcessingTest.cc
	
$(OBJ_DIR)/ProcessingState.o: processing/ProcessingState.cc processing/ProcessingState.h 
	$(COMP) -o $@ -c processing/ProcessingState.cc
	
$(OBJ_DIR)/StringBuilder.o: processing/StringBuilder.cc processing/StringBuilder.h 
	$(COMP) -o $@ -c processing/StringBuilder.cc
	
$(OBJ_DIR)/Reader.o: processing/Reader.cc processing/Reader.h utils/StringQueue.h
	$(COMP) -o $@ -c processing/Reader.cc
	
$(OBJ_DIR)/Writer.o: processing/Writer.cc processing/Writer.h utils/StringQueue.h
	$(COMP) -o $@ -c processing/Writer.cc
	
$(OBJ_DIR)/Config.o: processing/Config.cc
	$(COMP) -o $@ -c processing/Config.cc
	
$(OBJ_DIR)/SpecString.o: utils/SpecString.cc utils/SpecString.h
	$(COMP) -o $@ -c utils/SpecString.cc

$(OBJ_DIR)/ErrorReporting.o: utils/ErrorReporting.cc utils/ErrorReporting.h
	$(COMP) -o $@ -c utils/ErrorReporting.cc
	
$(OBJ_DIR)/Main.o: processing/Main.cc 
	$(COMP) -o $@ -c processing/Main.cc

clean:
	/bin/rm -rf $(EXE_DIR) $(OBJ_DIR)
