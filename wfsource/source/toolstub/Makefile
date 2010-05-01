################################################################################
# Makefile for toolstub library
################################################################################
# 'make all'            : Builds the toolstub library
# 'make test'           : Builds a test application which exercises the library
# 'make clean'          : Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = toolstub
!include ../../Makefile.env
TEST_PROG = strtest
TEST_LIBS += $(MEMORY) $(HAL)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)toolstub.obj \

################################################################################

!include ../Makefile.lib
