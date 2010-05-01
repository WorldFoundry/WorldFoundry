################################################################################
# Makefile for audio library
################################################################################
# 'make all'            : Builds the audio library
# 'make test'           : Builds a test application which exercises the library
# 'make clean'          : Deletes all of the results of this Makefile
################################################################################

LIB_NAME = audio

include ../../GNUMakefile.env

TEST_PROG = test
#TEST_PROG_PARAMS = boing.wav
TEST_LIBS += $(HAL) $(MEMORY) $(STREAMS) $(PIGSYS) $(CPPLIB) $(GFX) $(LOADFILE)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)buffer$(OBJ) \
	$(LIB_OBJ_DIR)device$(OBJ) \
	$(LIB_OBJ_DIR)audio$(OBJ) \

################################################################################


include ../../GNUMakefile.rul
# .rul was called from env, but not now.  Plus all the .lib stuff went into .rul
#include ../GNUMakefile.lib

