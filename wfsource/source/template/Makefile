#=============================================================================
# Makefile for template library
#=============================================================================
# 'make all'            : Builds the template library.
# 'make test'           : Builds a test application which exercises the template library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = template
!include ../../Makefile.env
TEST_LIBS += $(LOADFILE)

################################################################################

LIB_OBJS += \
        $(LIB_OBJ_DIR)$(LIB_NAME).obj \
	$(LIB_OBJ_DIR)symbol.obj \

################################################################################

!include ../Makefile.lib

################################################################################
