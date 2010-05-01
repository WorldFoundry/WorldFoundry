#=============================================================================
# Makefile for movement library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = movement
include ../../GNUMakefile.env

TEST_PROG = movementtest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(MATH) $(GFX) $(IFF) $(PHYSICS)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)movementobject$(OBJ) \
	$(LIB_OBJ_DIR)movement$(OBJ) \
	$(LIB_OBJ_DIR)movementmanager$(OBJ) \
	$(LIB_OBJ_DIR)movefoll$(OBJ) \
	$(LIB_OBJ_DIR)movepath$(OBJ) \


################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################
