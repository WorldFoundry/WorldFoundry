################################################################################
# Makefile for console library
################################################################################
# 'make all'		: Builds the console library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

LIB_NAME = console
include ../../GNUMakefile.env
TEST_PROG = test
TEST_LIBS += $(MEMORY) $(PIGSYS) $(HAL) $(CPPLIB)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)console$(OBJ) \
	$(LIB_OBJ_DIR)menu$(OBJ) \
	$(LIB_OBJ_DIR)hdump$(OBJ) \
	$(LIB_OBJ_DIR)textbuff$(OBJ) \

################################################################################

#include ../GNUMakefile.lib No longer used, merged with .rul
include ../../GNUMakefile.rul

################################################################################
