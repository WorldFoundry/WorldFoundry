#=============================================================================
# makefile for savegame library
#=============================================================================
# 'make all'		: Builds the savegame library.
# 'make test'		: Builds a test application which exercises the savegame library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = savegame
!include ../../Makefile.env

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)sgin.obj \
	$(LIB_OBJ_DIR)sgout.obj \
	$(LIB_OBJ_DIR)savei.obj \
	$(LIB_OBJ_DIR)saveo.obj \

################################################################################

!include ../Makefile.lib

################################################################################
