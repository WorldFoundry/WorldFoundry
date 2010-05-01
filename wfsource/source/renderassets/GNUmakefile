#=============================================================================
# Makefile for memory library
#=============================================================================
# 'make all'		: Builds the memory library.
# 'make test'		: Builds a test application which exercises the memory library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = renderassets
include ../../GNUMakefile.env

TEST_PROG = rendertest
# This next line was at the bottom, but was moved here to be consistant with all
# the other make files.
TEST_LIBS +=   $(HAL)  $(CPPLIB) $(PIGSYS) $(GFX)  $(ASSET)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)rendacto$(OBJ) \
	$(LIB_OBJ_DIR)rendcrow$(OBJ) \


################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################
