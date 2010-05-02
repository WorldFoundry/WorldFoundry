################################################################################
# GNUmakefile for expression parser

LIB_NAME = eval
include ../../GNUMakefile.env
CCCARGS += -DYY_NEVER_INTERACTIVE
#TEST_PROG = eval
TEST_LIBS += $(MATH) $(STREAMS) $(CPPLIB) $(MEMORY) $(HAL)

################################################################################

LIB_OBJS += \
        $(LIB_OBJ_DIR)expr_tab$(OBJ) \
        $(LIB_OBJ_DIR)lexyy$(OBJ) \

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

#$(PROGRAM).exe : $(PROGRAM)$(OBJ) $(OBJS) $(PIGS_LIBS) $(PROGRAM).lib $(PIGS_LIBS)

#$(PROGRAM).dll $(PROGRAM).lib : expr_tab$(OBJ) lexyy$(OBJ) $(PIGS_LIBS)
#        link /dll /nologo /debug /incremental:yes /libpath:"$(MSVCDIR)\lib" /implib:eval.lib /out:eval.dll $(.SOURCES)

expr.y : expr.h

expr.l : expr_tab.h

expr_tab.cc : expr.y 
	bison --defines --verbose --output=expr_tab.c expr.y
	mv expr_tab.c expr_tab.cc
#         --defines --verbose expr.y


expr_tab.h : expr.y 
	bison --defines --verbose --output=expr_tab.c expr.y
	mv expr_tab.c expr_tab.cc
#        ..\bin\bison --defines --verbose expr.y


lexyy.cc : expr.l
	flex -B -8 -Caf -p -t expr.l >lexyy.cc
	@perl -i~ -p -n fixlex.pl lexyy.cc
	@-rm lexyy.c~ >/dev/null
