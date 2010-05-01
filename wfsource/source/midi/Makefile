#=============================================================================
# makefile for midi library (lowest-level PIGS interface: midi.h)
#=============================================================================
# 'make all'		: Builds the midi library.
# 'make test'		: Builds a test application which exercises the midi library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = midi
TEST_PROG = miditest

################################################################################

LIB_OBJS = \
        $(LIB_OBJ_DIR)midi.obj \
        $(LIB_OBJ_DIR)wave.obj \
        $(LIB_OBJ_DIR)cd.obj \

################################################################################

!include ../Makefile.lib
