#==============================================================================
# Makefile for IFF compiler (iffcomp.exe) : Copyright (c) 1996-1999, World Foundry Group  
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

PROGRAM = iffcomp

.MS_NMAKE

ALL : $(PROGRAM).exe

OBJ_DIR = #$(PROGRAM).obj
OBJS = \
        $(OBJ_DIR)$(PROGRAM).obj \
        $(OBJ_DIR)lexyy.obj \
        $(OBJ_DIR)langlex.obj \
        $(OBJ_DIR)grammar.obj \
        $(OBJ_DIR)fileline.obj \
        $(OBJ_DIR)lang_tab.obj \
        $(OBJ_DIR)main.obj \
        $(OBJ_DIR)version.obj \


CCCARGS += -DYYDEBUG=1 -DYYERROR_VERBOSE=1
PIGS_LIBS += $(PIGSYS) $(STREAMS) $(MEMORY) $(CPPLIB) $(MATH) $(TOOLSTUB) $(IFFWRITE) $(RECOLIB)

!include ../Makefile.tool

INCLUDE += -I win                 # for unistd.h placeholder

#$(PROGRAM).exe: $(OBJS) $(PIGS_LIBS)

##############################################################################

#clean : tool-clean

##############################################################################

lang.l : lang_tab.h

lang_tab.cc lang_tab.h : lang.y c:\tools\share\bison.simple
        $(WF_BIN_DIR)\bison --defines --verbose --output=lang_tab.c lang.y
	del /e lang_tab.cc
	ren lang_tab.c lang_tab.cc
#        ..\bin\bison --defines --verbose lang.y

lexyy.cc : lang.l
        $(WF_BIN_DIR)\flex -8 -Caf -p -t -+ lang.l >lexyy.cc
#        $(WF_BIN_DIR)\flex -8 -Caf -p -t lang.l >lexyy.cc
        @$(WF_BIN_DIR)\perl -i~ -p -n ..\fixlex.pl lexyy.cc
	@-del lexyy.c~ >nul:

################################################################################

test-txt .PHONY : iffcomp.exe
	iffcomp.exe -ascii -o=testtxt.iff test.iff.txt
	type testtxt.iff

test-bin .PHONY : iffcomp.exe
	iffcomp.exe -binary -o=testbin.iff test.iff.txt
	hdump .\testbin.iff

#test .PHONY : test-txt test-bin

################################################################################

clean .PHONY : tool-clean
        -del lang_tab.cc lang_tab.cc.h
        -del lexyy.c lexyy.c~
        -del *.dll *.lib obj\*.obj *.exe

################################################################################

debug-txt .PHONY : iffcomp.exe
	"$(MSDEVDIR)\bin\msdev" iffcomp.exe -ascii -o=testtxt.iff test.iff.txt

debug-bin .PHONY : iffcomp.exe
	"$(MSDEVDIR)\bin\msdev" iffcomp.exe -binary -o=testbin.iff test.iff.txt

#debug .PHONY :
#        @echo Specify "debug-txt" or "debug-bin"

################################################################################
