################################################################################
# Makefile for audio library
################################################################################
# 'make all'		: Builds the audio library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = gfxfmt
!include ../../Makefile.env
TEST_PROG = test
#TEST_OBJS += $(LIB_OBJ_DIR)util.obj
#TEST_LIBS += $(PIGS_LIBS_DIR)/getopt/getopt.lib

LIB_OBJS += \
	$(LIB_OBJ_DIR)bitmap.obj \
	$(LIB_OBJ_DIR)ccycle.obj \
	$(LIB_OBJ_DIR)tga.obj \
	$(LIB_OBJ_DIR)bmp.obj \
	$(LIB_OBJ_DIR)sgi.obj \

################################################################################

!include ../Makefile.lib

################################################################################
