#=============================================================================
# Makefile for template library
#=============================================================================
# 'make all'            : Builds the template library.
# 'make test'           : Builds a test application which exercises the template library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

#.MS_NMAKE

LIB_NAME = template
include ../../GNUMakefile.env
TEST_LIBS += $(LOADFILE)

################################################################################

LIB_OBJS += \
        $(LIB_OBJ_DIR)$(LIB_NAME)$(OBJ) \
	$(LIB_OBJ_DIR)symbol$(OBJ) \

################################################################################


BINARY = ../../bin/template


LOCALTOP : TOP-ALL $(BINARY)


################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################

$(BINARY) :  $(LIB_OBJS) $(LIB_OBJ_DIR)$(TEST_PROG)$(EXE) $(TEST_LIBS)
	@echo ------------------------------------------------------------------------------
	@echo Creating \> \> $@ \< \<
	@echo ------------------------------------------------------------------------------
	gcc -o $@ $(LIB_OBJ_DIR)$(TEST_PROG)$(OBJ) $(TEST_OBJS) -Xlinker --start-group $(TEST_LIBS) -Xlinker --end-group $(LNARGS) $(SYS_LIBS)
