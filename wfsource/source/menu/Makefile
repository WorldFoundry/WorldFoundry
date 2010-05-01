#=============================================================================
# Makefile for particle library, testing
#=============================================================================
# 'make all'		: Builds the iff library.
# 'make test'		: Builds a test application which exercises the iff library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = menu
!include ../../Makefile.env
#TEST_PROG = test
TEST_LIBS += $(PIGSYS)

################################################################################

LIB_OBJS += \
#        $(LIB_OBJ_DIR)simpmenu.obj \
       $(LIB_OBJ_DIR)menu.obj \

################################################################################

!include ../Makefile.lib

################################################################################
