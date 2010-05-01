#=============================================================================
# Makefile for physics library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = asset
include ../../GNUMakefile.env

TEST_PROG = assettest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(IFF)

SYS_LIBS += $(GL_LIB) $(ODE_LIB) 


################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)assslot$(OBJ) \
	$(LIB_OBJ_DIR)assets$(OBJ) \

################################################################################

include ../../GNUMakefile.rul

################################################################################
