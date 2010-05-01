#=============================================================================
# Makefile for particle library
#=============================================================================
# 'make all'		: Builds the iff library.
# 'make test'		: Builds a test application which exercises the iff library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = particle
!include ../../Makefile.env
TEST_PROG = test
TEST_LIBS += $(TIMER)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)particle.obj \
	$(LIB_OBJ_DIR)emitter.obj \

################################################################################

!include ../Makefile.lib

################################################################################
