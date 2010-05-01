#=============================================================================
# Makefile for physics library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = physics
include ../../GNUMakefile.env

VPATH = $(PHYSICS_ENGINE)

TEST_PROG = physicstest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(MATH) $(GFX) $(IFF)

ifeq ($(WF_TARGET),win)
CCCARGS += -Zm500
endif

SYS_LIBS += $(GL_LIB) $(ODE_LIB) 


################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)colbox$(OBJ) \
	$(LIB_OBJ_DIR)colspace$(OBJ) \
	$(LIB_OBJ_DIR)physical$(OBJ) \
	$(LIB_OBJ_DIR)collision$(OBJ) \
	$(LIB_OBJ_DIR)physicalobject$(OBJ) \
	$(LIB_OBJ_DIR)activate$(OBJ) \

ifeq ($(PHYSICS_ENGINE),ode)
LIB_OBJS += \
	$(LIB_OBJ_DIR)ode$(OBJ) 
endif

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################
