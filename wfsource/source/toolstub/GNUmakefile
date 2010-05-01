################################################################################
# Makefile for toolstub library
################################################################################
# 'make all'            : Builds the toolstub library
# 'make test'           : Builds a test application which exercises the library
# 'make clean'          : Deletes all of the results of this Makefile
################################################################################

LIB_NAME = toolstub
include ../../GNUMakefile.env
TEST_PROG = strtest
TEST_LIBS += $(MEMORY) $(HAL)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)toolstub$(OBJ) \

################################################################################

#include ../GNUMakefile.lib No longer used
include ../../GNUMakefile.rul
