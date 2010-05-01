#=============================================================================
# Makefile for scripting library
#=============================================================================
# 'make all'		: Builds the scripting library.
# 'make test'		: Builds a test application which exercises the scripting library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = scripting
TEST_PROG = scriptingtest
!include ../../Makefile.env
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(MAILBOX)

!if "$(WF_TARGET)" == "win"
CCCARGS += -Zm500
!endif

.PATH.cc+=;.\$(WF_TARGET)\msvc

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)scriptinterpreter$(OBJ) \
	$(LIB_OBJ_DIR)tcl$(OBJ) \

################################################################################

!include ../Makefile.lib

################################################################################
