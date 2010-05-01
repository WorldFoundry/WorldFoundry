#=============================================================================
# Makefile for baseobject library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = baseobject
!include ../../Makefile.env
TEST_PROG = baseobjecttest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) 

!if "$(WF_TARGET)" == "win"
CCCARGS += -Zm500
!endif

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)msgport.obj \
	$(LIB_OBJ_DIR)commonblock.obj \
	$(LIB_OBJ_DIR)baseobject.obj \

################################################################################

!include ../Makefile.lib

################################################################################
