################################################################################
# Makefile for audio library
################################################################################
# 'make all'            : Builds the audio library
# 'make test'           : Builds a test application which exercises the library
# 'make clean'          : Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = audio
!include ../../Makefile.env
TEST_PROG = test
#TEST_PROG_PARAMS = boing.wav
TEST_LIBS += $(HAL) $(MEMORY) $(STREAMS) $(PIGSYS) $(CPPLIB) $(GFX) $(LOADFILE)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)buffer.obj \
	$(LIB_OBJ_DIR)device.obj \
	$(LIB_OBJ_DIR)audio.obj \

################################################################################

!include ../Makefile.lib
