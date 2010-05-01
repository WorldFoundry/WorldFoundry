#=============================================================================
# Makefile for iff library
#=============================================================================
# 'make all'		: Builds the iff library.
# 'make test'		: Builds a test application which exercises the iff library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = iff
!include ../../Makefile.env
TEST_PROG = ifftest

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)iffread.obj \
	$(LIB_OBJ_DIR)disktoc.obj \

!if "$(WF_TARGET)" != "psx"
!endif

################################################################################

!include ../Makefile.lib

################################################################################
