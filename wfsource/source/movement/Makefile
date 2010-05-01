#=============================================================================
# Makefile for physics library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = movement
!include ../../Makefile.env
TEST_PROG = movementtest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(MATH) $(PHYSICS)

!if "$(WF_TARGET)" == "win"
CCCARGS += -Zm500
!endif

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)movementobject.obj \
	$(LIB_OBJ_DIR)movement.obj \
	$(LIB_OBJ_DIR)movementmanager.obj \
	$(LIB_OBJ_DIR)movefoll.obj \
	$(LIB_OBJ_DIR)movepath.obj \

			      
################################################################################

!include ../Makefile.lib

################################################################################
