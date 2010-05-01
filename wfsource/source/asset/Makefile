#=============================================================================
# Makefile for physics library
#=============================================================================
# 'make all'		: Builds the physics library.
# 'make test'		: Builds a test application which exercises the physics library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = asset
!include ../../Makefile.env
TEST_PROG = assettest
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(MATH)

!if "$(WF_TARGET)" == "win"
CCCARGS += -Zm500
!endif

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)assslot.obj \
	$(LIB_OBJ_DIR)assets.obj \

################################################################################

!include ../Makefile.lib

################################################################################
