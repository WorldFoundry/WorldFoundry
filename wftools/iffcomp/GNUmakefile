#==============================================================================
# GNUmakefile for IFF compiler (iffcomp.exe) : Copyright (c) 1999, World Foundry Group  
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


.PHONY : ALL
ALL: REALALL

OBJS = \
        $(OBJ_DIR)$(PROGRAM)$(OBJ) \
        $(OBJ_DIR)grammar$(OBJ) \
        $(OBJ_DIR)fileline$(OBJ) \
        $(OBJ_DIR)langlex$(OBJ) \
        $(OBJ_DIR)lang_tab$(OBJ) \
        $(OBJ_DIR)lexyy$(OBJ) \
        $(OBJ_DIR)main$(OBJ) \
        $(OBJ_DIR)version$(OBJ) \

include ../GNUMakefile.tool

.PHONY : REALLALL 
REALALL : $(OBJ_DIR) $(PROGRAM)$(EXE)

CCCARGS += -DYYDEBUG=1 -DYYERROR_VERBOSE=1
PIGS_LIBS += $(PIGSYS) $(STREAMS) $(MEMORY) $(CPPLIB) $(MATH) $(TOOLSTUB) $(IFFWRITE) $(RECOLIB)

$(PROGRAM)$(EXE) : $(OBJS) $(PIGS_LIBS)

##############################################################################

$(OBJ_DIR)lang_tab$(OBJ) : lang_tab.cc 

lang.l : lang_tab.h 

lang_tab.cc lang_tab.h : lang.y 
	bison --defines --verbose --output=lang_tab.c lang.y
	mv lang_tab.c lang_tab.cc

lexyy.cc : lang.l
	flex -B -8 -Caf -p -t -+ lang.l >lexyy.cc
	perl -i~ -p -n ../GNUfixlex.pl lexyy.cc

################################################################################

.PHONY: test-txt
test-txt : iffcomp$(EXE)
	./$(PROGRAM)$(EXE) -ascii -o=testout.iff.txt test.iff.txt
	cat testout.iff.txt

.PHONY : test-bin
test-bin  : iffcomp$(EXE)
	./$(PROGRAM)$(EXE) -binary -o=testout.iff test.iff.txt
#	less --force testout.iff

#test .PHONY : test-txt test-bin

##############################################################################

.PHONY: clean
clean : tool-clean
	rm -f lang_tab.cc lang_tab.cc.h lang_tab.cc.output lexyy.cc* lang.output
	rm -f testtxt.iff testbin.iff test.wf
	rm -f $(PROGRAM)$(EXE)

.PHONY: realclean
realclean : clean
	rm -f *~

################################################################################
