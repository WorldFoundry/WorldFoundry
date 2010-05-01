#=============================================================================
# Makefile for scripting library
#=============================================================================
# 'make all'		: Builds the scripting library.
# 'make test'		: Builds a test application which exercises the scripting library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = mailbox
OVERRIDE-ALL: OVERRIDE-ALL2

!include ../../Makefile.env
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) 

!if "$(WF_TARGET)" == "win"
CCCARGS += -Zm500
!endif

.PATH.cc+=;.\$(WF_TARGET)\msvc

#==============================================================================

OVERRIDE-ALL2: $(OAD_DIR)/../mailbox.def TOP-ALL

#==============================================================================

$(OAD_DIR)/../mailbox.def : mailbox.des mailbox.inc
	..\..\bin\prep mailbox.des $(OAD_DIR)/../mailbox.def

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)mailbox.obj \

################################################################################

!include ../Makefile.lib

################################################################################
