################################################################################
# Makefile for streams library
################################################################################
# 'make all'            : Builds the streams library
# 'make test'           : Builds a test application which exercises the library
# 'make clean'          : Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = streams
TEST_PROG = strtest
!include ../../Makefile.env
TEST_LIBS += $(MEMORY) $(HAL)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)binstrm$(OBJ) \

################################################################################

!include ../Makefile.lib
