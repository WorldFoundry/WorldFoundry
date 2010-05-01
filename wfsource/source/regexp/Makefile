################################################################################
# makefile for regexp library
################################################################################
# 'make all'            : Builds the regexp library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = regexp
!include ../../Makefile.env

################################################################################

LIB_OBJS += \
        $(LIB_OBJ_DIR)regerror.obj \
        $(LIB_OBJ_DIR)regexp.obj \
        $(LIB_OBJ_DIR)regsub.obj \

################################################################################

!include ../Makefile.lib

################################################################################
