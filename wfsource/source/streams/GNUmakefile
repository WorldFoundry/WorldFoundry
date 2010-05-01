################################################################################
# Makefile for streams library
################################################################################
# 'make all'            : Builds the streams library
# 'make test'           : Builds a test application which exercises the library
# 'make clean'          : Deletes all of the results of this Makefile
################################################################################

LIB_NAME = streams
TEST_PROG = strtest
include ../../GNUMakefile.env
TEST_LIBS += $(MEMORY) $(HAL) $(CPPLIB)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)binstrm$(OBJ) \

################################################################################

#include ../GNUMakefile.lib No longer used.
include ../../GNUMakefile.rul
