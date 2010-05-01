################################################################################
# Makefile for IFF Writer

LIB_NAME = iffwrite
TEST_PROG = test
include ../../GNUMakefile.env
TEST_LIBS += $(MATH) $(LOADFILE) $(STREAMS) $(TOOLSTUB) $(MEMORY) $(CPPLIB)
#TEST_LIBS += #$(PIGSYS) $(CPPLIB) $(MATH) $(STREAMS) $(MEMORY) $(TOOLSTUB) $(MENU)

################################################################################

LIB_OBJS += \
        $(OBJ_DIR)binary$(OBJ) \
        $(OBJ_DIR)fixed$(OBJ) \
        $(OBJ_DIR)id$(OBJ) \
        $(OBJ_DIR)text$(OBJ) \
        $(OBJ_DIR)timestamp$(OBJ) \
        $(OBJ_DIR)_iffwr$(OBJ) \
#        $(OBJ_DIR)$(LIB_NAME)$(OBJ) \

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################

#test .PHONY : s.$(OBJ)\test.exe
#        $(.SOURCE)

################################################################################
