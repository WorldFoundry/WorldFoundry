################################################################################
# Makefile for IFF Writer

.MS_NMAKE

LIB_NAME = iffwrite
!include ../../Makefile.env
TEST_PROG = test
TEST_LIBS += $(MATH) $(LOADFILE) $(STREAMS) $(TOOLSTUB) $(MEMORY) $(CPPLIB)
#TEST_LIBS += #$(PIGSYS) $(CPPLIB) $(MATH) $(STREAMS) $(MEMORY) $(TOOLSTUB) $(MENU)

################################################################################

LIB_OBJS += \
        $(OBJ_DIR)binary.obj \
        $(OBJ_DIR)fixed.obj \
        $(OBJ_DIR)id.obj \
        $(OBJ_DIR)text.obj \
        $(OBJ_DIR)timestamp.obj \
        $(OBJ_DIR)_iffwr.obj \
#        $(OBJ_DIR)$(LIB_NAME).obj \

################################################################################

!include ../Makefile.lib

################################################################################

#test .PHONY : s.obj\test.exe
#        $(.SOURCE)

################################################################################
