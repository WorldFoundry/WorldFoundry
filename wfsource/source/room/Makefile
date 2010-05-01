#=============================================================================
# Makefile for room library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = room
!include ../../Makefile.env
TEST_PROG = roomtest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(MATH)

!if "$(WF_TARGET)" == "win"
CCCARGS += -Zm500
!endif

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)room.obj \
	$(LIB_OBJ_DIR)rooms.obj \
	$(LIB_OBJ_DIR)actrooms.obj \
	$(LIB_OBJ_DIR)actroit.obj \

################################################################################

!include ../Makefile.lib

################################################################################
