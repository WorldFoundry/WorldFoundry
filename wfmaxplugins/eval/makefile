################################################################################
# Makefile for expression parser

.MS_NMAKE


PROGRAM = eval

all : $(PROGRAM).lib

#!include ../Makefile.def
#TEST_PROG = eval
TEST_LIBS += $(MATH) $(STREAMS) $(CPPLIB) $(MEMORY) $(HAL)

#INCLUDE += -I c:\tools\shared 
#INCLUDE += -I .                 # for missing unistd.h

.PATH.cc = .;..\lib

################################################################################

LIB_OBJS += \
        $(LIB_OBJ_DIR)expr_tab.obj \
        $(LIB_OBJ_DIR)lexyy.obj \

################################################################################

#!include ../Makefile.inc

#$(PROGRAM).exe : $(PROGRAM).obj $(OBJS) $(PIGS_LIBS) $(PROGRAM).lib $(PIGS_LIBS)

$(PROGRAM).lib : $(LIB_OBJS) 
        lib  /out:eval.lib $(.SOURCES)


.cc.obj :
	echo $(CPP) $(.SOURCE) -I. -I..\lib /I$(3DSMAX2_DIR)\maxsdk\include /TP /nologo /c $(CCCARGS) /Fo$(.TARGET)
	$(CPP) $(.SOURCE) @<<
-I. -I..\lib /I"$(3DSMAX2_DIR)\maxsdk\include" /TP /nologo /c $(CCCARGS) /Fo$(.TARGET)
<<


expr.y : expr.h

expr.l : expr_tab.cc.h

expr_tab.cc expr_tab.cc.h : expr.y c:\tools\share\bison.simple
        bison --defines --verbose --output=expr_tab.cc expr.y
#        ..\bin\bison --defines --verbose expr.y

lexyy.cc : expr.l
        flex -8 -Caf -p -t expr.l >lexyy.cc
#        ..\bin\flex -8 -Caf -p -t expr.l >lexyy.cc
        @perl -i~ -p -n fixlex.pl lexyy.cc
	@-del lexyy.c~ >nul:




