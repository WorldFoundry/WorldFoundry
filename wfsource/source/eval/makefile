################################################################################
# Makefile for expression parser

.MS_NMAKE

LIB_NAME = eval
!include ../../Makefile.env
#TEST_PROG = eval
TEST_LIBS += $(MATH) $(STREAMS) $(CPPLIB) $(MEMORY) $(HAL)

INCLUDE += -I c:\tools\shared
INCLUDE += -I .                 # for missing unistd.h

################################################################################

LIB_OBJS += \
        $(LIB_OBJ_DIR)expr_tab.obj \
        $(LIB_OBJ_DIR)lexyy.obj \

################################################################################

!include ../Makefile.lib

#$(PROGRAM).exe : $(PROGRAM).obj $(OBJS) $(PIGS_LIBS) $(PROGRAM).lib $(PIGS_LIBS)

#$(PROGRAM).dll $(PROGRAM).lib : expr_tab.obj lexyy.obj $(PIGS_LIBS)
#        link /dll /nologo /debug /incremental:yes /libpath:"$(MSVCDIR)\lib" /implib:eval.lib /out:eval.dll $(.SOURCES)

expr.y : expr.h

expr.l : expr_tab.h

expr_tab.cc expr_tab.h : expr.y c:\tools\share\bison.simple
        ..\..\bin\bison --defines --verbose --output=expr_tab.c expr.y
	-del expr_tab.cc 
	ren expr_tab.c expr_tab.cc
#        ..\bin\bison --defines --verbose expr.y

lexyy.cc : expr.l
        ..\..\bin\flex -8 -Caf -p -t expr.l >lexyy.cc
#        ..\bin\flex -8 -Caf -p -t expr.l >lexyy.cc
        @..\..\bin\perl -i~ -p -n fixlex.pl lexyy.cc
	@-del lexyy.c~ >nul:
