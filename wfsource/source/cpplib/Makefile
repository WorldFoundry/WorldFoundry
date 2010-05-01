#=============================================================================
# Makefile for cpplib library
#=============================================================================
# 'make all'		: Builds the cpplib library.
# 'make test'		: Builds a test application which exercises the cpplib library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

CPPLIB-ALL :       ..\libstrm.inc TOP-ALL

LIB_NAME = cpplib
TEST_PROG = cpptest

!include ../../Makefile.env

#TEST_LIBS += $(MENU)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)range$(OBJ) \
	$(LIB_OBJ_DIR)hdump$(OBJ) \
#	$(LIB_OBJ_DIR)hierarch$(OBJ) \

!if "$(BUILD_DBSTREAM)" != "0"
LIB_OBJS += \
	$(LIB_OBJ_DIR)stdstrm$(OBJ) \
	$(LIB_OBJ_DIR)libstrm$(OBJ)

!if "$(WF_TARGET)" != "psx"
LIB_OBJS += $(LIB_OBJ_DIR)strmnull$(OBJ)
!endif		# psx
!endif		# dbstream

################################################################################

!include ../Makefile.lib

################################################################################

libstrm.cc : ..\libstrm.inc

..\libstrm.inc :
	%cd ..
	make libstrm
	%cd cpplib

################################################################################
