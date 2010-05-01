#=============================================================================
# GNUmakefile for particle library
#=============================================================================
# 'make all'		: Builds the iff library.
# 'make test'		: Builds a test application which exercises the iff library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = particle
TEST_PROG = test
include ../../GNUMakefile.env
TEST_LIBS += $(TIMER)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)particle$(OBJ) \
	$(LIB_OBJ_DIR)emitter$(OBJ) \

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################
