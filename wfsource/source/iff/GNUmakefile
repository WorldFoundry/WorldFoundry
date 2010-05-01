#=============================================================================
# GNUmakefile for iff library
#=============================================================================
# 'make all'		: Builds the iff library.
# 'make test'		: Builds a test application which exercises the iff library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = iff
include ../../GNUMakefile.env
TEST_PROG = ifftest

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)iffread$(OBJ) \
	$(LIB_OBJ_DIR)disktoc$(OBJ) \

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################
