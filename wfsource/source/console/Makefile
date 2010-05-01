################################################################################
# Makefile for console library
################################################################################
# 'make all'		: Builds the console library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = console
!include ../../Makefile.env
TEST_PROG = test
TEST_LIBS += $(MEMORY) $(PIGSYS) $(HAL) $(CPPLIB)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)console.obj \
	$(LIB_OBJ_DIR)menu.obj \
	$(LIB_OBJ_DIR)hdump.obj \
	$(LIB_OBJ_DIR)textbuff.obj \

################################################################################

!include ../Makefile.lib

################################################################################
