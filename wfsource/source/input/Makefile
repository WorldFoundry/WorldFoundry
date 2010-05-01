#=============================================================================
# Makefile for input library (lowest-level PIGS interface: input.h)
#=============================================================================
# 'make all'		: Builds the input library.
# 'make test'		: Builds a test application which exercises the input library.
# 'make clean'		: Deletes all of the results of this Makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = input
TEST_PROG = misctest
#TEST_LIBS += $(HAL)
#TEST_LIBS += $(PIGSYS) $(CPPLIB) $(IFF) $(MATH) $(STREAMS) $(PIGS_OBJ_DIR)\$(LIB_NAME)\$(LIB_NAME).lib


!include ../../Makefile.env

################################################################################

LIB_OBJS = \
	$(LIB_OBJ_DIR)inputdig.obj \

################################################################################

!include ../Makefile.lib

