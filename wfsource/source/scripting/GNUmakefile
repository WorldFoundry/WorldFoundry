#=============================================================================
# Makefile for scripting library
#=============================================================================
# 'make all'		: Builds the scripting library.
# 'make test'		: Builds a test application which exercises the scripting library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME =scripting
TEST_PROG = scriptingtest
include ../../GNUMakefile.env
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY) $(MAILBOX)

INCLUDE += -I/usr/include/tcl

ifeq ($(WF_TARGET),win)
	CCCARGS += -Zm500
endif

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)scriptinterpreter$(OBJ) \
	

	
#ifeq ($(TCL)),1)
	LIB_OBJS += $(LIB_OBJ_DIR)tcl$(OBJ) 
#endif


################################################################################

include ../../GNUMakefile.rul

################################################################################
