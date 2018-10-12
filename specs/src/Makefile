MKDIR_C = mkdir -p
EXE_DIR = ../exe
OBJ_DIR = ../obj
INC = -I cli
COMP = g++ -g $(INC)
LINK = g++ -g

CLI_OBJ = $(OBJ_DIR)/tokens.o $(OBJ_DIR)/splitter.o
SI_OBJ = $(OBJ_DIR)/specItems.o $(OBJ_DIR)/dataField.o
PR_OBJ = $(OBJ_DIR)/conversions.o $(OBJ_DIR)/Reader.o $(OBJ_DIR)/Writer.o

.DUMMY: directories

all: directories $(EXE_DIR)/tokenTest $(EXE_DIR)/itemTest

directories: $(EXE_DIR) $(OBJ_DIR)

$(EXE_DIR):
	$(MKDIR_C) $@

$(OBJ_DIR):
	$(MKDIR_C) $@

$(EXE_DIR)/tokenTest: $(OBJ_DIR)/test.o $(CLI_OBJ) $(PR_OBJ)
	$(COMP) -o $@ ../obj/test.o $(CLI_OBJ) $(PR_OBJ)

$(EXE_DIR)/itemTest: $(OBJ_DIR)/itemTest.o $(CLI_OBJ) $(SI_OBJ) $(PR_OBJ)
	$(COMP) -o $@ ../obj/itemTest.o $(CLI_OBJ) $(SI_OBJ) $(PR_OBJ)

$(OBJ_DIR)/tokens.o: cli/tokens.cc cli/tokens.h
	$(COMP) -o $@ -c cli/tokens.cc
	
$(OBJ_DIR)/splitter.o: cli/splitter.cc $(OBJ_DIR)/tokens.o
	$(COMP) -o $@ -c cli/splitter.cc

$(OBJ_DIR)/specItems.o: specitems/specItems.cc specitems/specItems.h specitems/item.h
	$(COMP) -o $@ -c specitems/specItems.cc

$(OBJ_DIR)/dataField.o: specitems/dataField.cc specitems/item.h processing/conversions.h
	$(COMP) -o $@ -c specitems/dataField.cc
	
$(OBJ_DIR)/conversions.o: processing/conversions.cc processing/conversions.h
	$(COMP) -o $@ -c processing/conversions.cc
	
$(OBJ_DIR)/test.o: test/test.cc cli/tokens.h
	$(COMP) -o $@ -c test/test.cc

$(OBJ_DIR)/itemTest.o: test/itemTest.cc cli/tokens.h specitems/specItems.h
	$(COMP) -o $@ -c test/itemTest.cc
	
	
$(OBJ_DIR)/Reader.o: processing/Reader.cc processing/Reader.h
	$(COMP) -o $@ -c processing/Reader.cc
$(OBJ_DIR)/Writer.o: processing/Writer.cc processing/Writer.h
	$(COMP) -o $@ -c processing/Writer.cc

clean:
	/bin/rm -rf $(EXE_DIR) $(OBJ_DIR)
