#=============================================================================
# Makefile for cpplib library
#=============================================================================
# 'make all'		: Builds the cpplib library.
# 'make test'		: Builds a test application which exercises the cpplib library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

CPPLIB-ALL :       ../libstrm.inc TOP-ALL

LIB_NAME = cpplib
TEST_PROG = cpptest

include ../../GNUMakefile.env

#TEST_LIBS += $(MENU)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)hdump$(OBJ) \
	$(LIB_OBJ_DIR)range$(OBJ) \
#	$(LIB_OBJ_DIR)hierarch$(OBJ) \

ifneq ($(BUILD_DBSTREAM),0)
LIB_OBJS += \
	$(LIB_OBJ_DIR)strmnull$(OBJ) \
	$(LIB_OBJ_DIR)stdstrm$(OBJ) \
	$(LIB_OBJ_DIR)libstrm$(OBJ)
endif		# dbstream

################################################################################

#include ../GNUMakefile.lib Not used any more.
include ../../GNUMakefile.rul

################################################################################

libstrm.cc : ../libstrm.inc

# Changed to use the new "libstrm.inc" target in the above level make file.
# (Instead of the old "libstrm" target)
..\libstrm.inc :
	$(MAKE) -C .. libstrm.inc

################################################################################
