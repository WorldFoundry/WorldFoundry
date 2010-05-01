################################################################################
# GNUmakefile for regexp library
################################################################################
# 'make all'            : Builds the regexp library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

LIB_NAME = regexp
include ../../GNUMakefile.env

################################################################################

LIB_OBJS += \
        $(LIB_OBJ_DIR)regerror$(OBJ) \
        $(LIB_OBJ_DIR)regexp$(OBJ) \
        $(LIB_OBJ_DIR)regsub$(OBJ) \

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################
