#==============================================================================
# Makefile for aicomp (AI Compiler) : Copyright (c) 1996-1999, World Foundry Group  
# Part of the World Foundry 3D video game engine/production environment
# for more information about World Foundry, see www.worldfoundry.org
#==============================================================================
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# or see www.fsf.org
#==============================================================================

LANGUAGE ?= scheme
LANGUAGE ?= c

PROGRAM = aicomp_$(LANGUAGE)

.MS_NMAKE

OBJ_DIR = obj\			#comment so that \ is not last char on 

ALL : $(OBJ_DIR) $(LANGUAGE) $(PROGRAM).exe airun.exe aidump.exe

OBJS = \
        $(OBJ_DIR)$(LANGUAGE)\ai_tab.obj \
        $(OBJ_DIR)lex$(LANGUAGE).obj \
        $(OBJ_DIR)ailexer.obj \
        $(OBJ_DIR)grammar.obj \
        $(OBJ_DIR)backpatch.obj \
        $(OBJ_DIR)function.obj \
        $(OBJ_DIR)strtable.obj \
        $(OBJ_DIR)fileline.obj \
        $(OBJ_DIR)beep.obj \
        $(OBJ_DIR)aicomp.obj \
        $(OBJ_DIR)symbol.obj \
        $(OBJ_DIR)flex.obj \
        $(OBJ_DIR)filesize.obj \
        $(OBJ_DIR)version.obj \

!include ../Makefile.tool
CCCARGS += -Zm500
CCCARGS += -DYYDEBUG=1 -DYYERROR_VERBOSE=1

PIGS_LIBS += $(IFFWRITE)
INCLUDE += -I .                 # for unistd.h placeholder

##############################################################################

airun.exe :     $(OBJ_DIR)airun.obj $(OBJ_DIR)filesize.obj $(OBJ_DIR)version.obj $(PIGS_LIBS)

aidump.exe :    $(OBJ_DIR)aidump.obj $(OBJ_DIR)filesize.obj $(OBJ_DIR)version.obj $(PIGS_LIBS)

##############################################################################

ai.l : ai_tab.cc.h


$(OBJ_DIR):
	mkdir $(OBJ_DIR)
	mkdir $(OBJ_DIR)$(LANGUAGE)


$(LANGUAGE):
	mkdir $(LANGUAGE)

$(LANGUAGE)\ai_tab.cc $(LANGUAGE)\ai_tab.cc.h : $(LANGUAGE).y c:\tools\share\bison.simple
        ..\bin\bison --defines --verbose --output=$(LANGUAGE)\ai_tab.cc $(LANGUAGE).y
	copy $(LANGUAGE)\ai_tab.cc.h $(OBJ_DIR)\tokens_tab.h

lex$(LANGUAGE).cc FlexLe~1.h : $(LANGUAGE).l
        ..\bin\flex -8 -Caf -p -t -+ $(LANGUAGE).l >lexyy.cc
        @ren lexyy.cc lex$(LANGUAGE).cc
        @..\bin\perl -i~ -p -n ..\fixlex.pl lex$(LANGUAGE).cc
        @-del lex$(LANGUAGE).cc~

ailexer.cc : flexle~1.h

symbol.cc : flexle~1.h

ailexer.hp : flexle~1.h

##############################################################################

clean : tool-clean
        del $(LANGUAGE)\ai_tab.c $(LANGUAGE)\ai_tab.h $(LANGUAGE)_tab.h
	del *.*~
	del /Y scheme c 
        del lex$(LANGUAGE).cc *.out
	del *.dll *.lib *.obj *.exe
	del *.opt *.lcf *.pdb *.ilk *.ncb *.plg
	del *.aib *.log.htm

realclean .PHONY :
	make clean LANGUAGE=scheme
	make clean LANGUAGE=c

test-scheme .PHONY : aicomp_$(LANGUAGE).exe
	aicomp_scheme.exe test/test.scm
	start test/test.log.htm

debug-scheme .PHONY : aicomp_$(LANGUAGE).exe
	msdev aicomp_scheme.exe test/test.scm

test-c .PHONY : aicomp_$(LANGUAGE).exe
	aicomp_c.exe test/test.c
	start test/test.log.htm

test .PHONY : test-$(LANGUAGE)

testall .PHONY : test-scheme test-c

printlocal .PHONY :
        @echo LANGUAGE = $(LANGUAGE) [*scheme*,c]

###############################################################################
