MKDIR_C = mkdir -p
EXE_DIR = ../exe
OBJ_DIR = ../obj
INC = -I cli
COMP = g++ -g $(INC)
LINK = g++ -g

.DUMMY: directories

all: directories $(EXE_DIR)/tokenTest

directories: $(EXE_DIR) $(OBJ_DIR)

$(EXE_DIR):
	$(MKDIR_C) $@

$(OBJ_DIR):
	$(MKDIR_C) $@

$(EXE_DIR)/tokenTest: $(OBJ_DIR)/test.o $(OBJ_DIR)/tokens.o $(OBJ_DIR)/splitter.o
	$(COMP) -o $@ ../obj/test.o ../obj/tokens.o $(OBJ_DIR)/splitter.o

$(OBJ_DIR)/tokens.o: cli/tokens.cc cli/tokens.h
	$(COMP) -o $@ -c cli/tokens.cc
	
$(OBJ_DIR)/splitter.o: cli/splitter.cc $(OBJ_DIR)/tokens.o
	$(COMP) -o $@ -c cli/splitter.cc
	
$(OBJ_DIR)/test.o: test/test.cc ../obj/tokens.o cli/tokens.h
	$(COMP) -o $@ -c test/test.cc
	
clean:
	/bin/rm -rf $(EXE_DIR) $(OBJ_DIR)
