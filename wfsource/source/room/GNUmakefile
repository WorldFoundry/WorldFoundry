#=============================================================================
# Makefile for room library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = room
include ../../GNUMakefile.env

TEST_PROG = roomtest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(IFF)

SYS_LIBS += $(GL_LIB) $(ODE_LIB) 


################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)room$(OBJ) \
	$(LIB_OBJ_DIR)rooms$(OBJ) \
	$(LIB_OBJ_DIR)actrooms$(OBJ) \
	$(LIB_OBJ_DIR)actroit$(OBJ) \

################################################################################

include ../../GNUMakefile.rul

################################################################################
