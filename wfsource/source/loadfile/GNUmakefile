#=============================================================================
# GNUmakefile for loadfile library
#=============================================================================
# 'make all'		: Builds the iff library.
# 'make test'		: Builds a test application which exercises the iff library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = loadfile
include ../../GNUMakefile.env
#TEST_PROG = test
TEST_LIBS += $(MATH) $(STREAMS) $(CPPLIB) $(MEMORY) $(GFX) $(HAL) $(IFF) $(TIMER)

################################################################################

LIB_OBJS += \
        $(LIB_OBJ_DIR)$(LIB_NAME)$(OBJ) \

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################
