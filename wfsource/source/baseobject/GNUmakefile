#=============================================================================
# Makefile for baseobject library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = baseobject
include ../../GNUMakefile.env

TEST_PROG = baseobjecttest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(IFF)

#SYS_LIBS += $(GL_LIB) $(ODE_LIB) 

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)msgport$(OBJ) \
	$(LIB_OBJ_DIR)commonblock$(OBJ) \
	$(LIB_OBJ_DIR)baseobject$(OBJ) \


################################################################################

include ../../GNUMakefile.rul

################################################################################
