#=============================================================================
# Makefile for physics library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = physics
!include ../../Makefile.env
TEST_PROG = physicstest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(MATH)

!if "$(WF_TARGET)" == "win"
CCCARGS += -Zm500
!endif

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)colbox.obj \
	$(LIB_OBJ_DIR)colspace.obj \
	$(LIB_OBJ_DIR)physical.obj \
	$(LIB_OBJ_DIR)physicalobject.obj \
	$(LIB_OBJ_DIR)collision.obj \
	$(LIB_OBJ_DIR)physicalobject.obj \
	$(LIB_OBJ_DIR)activate.obj \

!if "$(WF_TARGET)" != "psx"
LIB_OBJS +=
!endif

################################################################################

!include ../Makefile.lib

################################################################################
