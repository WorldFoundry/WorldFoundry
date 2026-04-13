#==============================================================================
# GNUmakefile for IFF compiler (iffcomp.exe) : Copyright (c) 1999, 2026, World Foundry Group
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

# Force float-backed scalar on 2026 Linux builds. The default `fixed` path
# relies on x86-32 inline asm in math/scalar.cc which doesn't assemble on
# x86-64. `export` so recursive builds of libmath.a pick it up too.
SCALAR_TYPE := float
export SCALAR_TYPE

# iffcomp is a CLI tool; don't drag X11 into the link line.
WF_TOOL_NEEDS_X11 := no

.PHONY : ALL
ALL : REALALL

OBJS = \
        $(OBJ_DIR)$(PROGRAM)$(OBJ) \
        $(OBJ_DIR)grammar$(OBJ) \
        $(OBJ_DIR)fileline$(OBJ) \
        $(OBJ_DIR)langlex$(OBJ) \
        $(OBJ_DIR)lang.tab$(OBJ) \
        $(OBJ_DIR)lang.yy$(OBJ) \
        $(OBJ_DIR)main$(OBJ) \
        $(OBJ_DIR)version$(OBJ) \

include ../GNUMakefile.tool

.PHONY : REALLALL 
REALALL : $(OBJ_DIR) $(PROGRAM)$(EXE)

CCCARGS += -std=c++17 -DYYDEBUG=1
PIGS_LIBS += $(PIGSYS) $(STREAMS) $(MEMORY) $(CPPLIB) $(MATH) $(TOOLSTUB) $(IFFWRITE) $(RECOLIB)

$(PROGRAM)$(EXE) : $(OBJS) $(PIGS_LIBS)

##############################################################################

$(OBJ_DIR)lang.tab$(OBJ) : lang.tab.cc
$(OBJ_DIR)lang.yy$(OBJ)  : lang.yy.cc lang.tab.hh

lang.l : lang.tab.hh

lang.tab.cc lang.tab.hh location.hh : lang.y
	bison -Wall --defines=lang.tab.hh --output=lang.tab.cc lang.y

lang.yy.cc : lang.l
	flex -o lang.yy.cc lang.l

################################################################################

.PHONY : test-txt
test-txt : iffcomp$(EXE)
	./$(PROGRAM)$(EXE) -ascii -o=testout.iff.txt test.iff.txt
	cat testout.iff.txt

.PHONY : test-bin
test-bin : iffcomp$(EXE)
	./$(PROGRAM)$(EXE) -binary -o=testout.iff test.iff.txt
	hexdump -C testout.iff

#test .PHONY : test-txt test-bin

##############################################################################

.PHONY : clean
clean : tool-clean
	rm -f lang.tab.cc lang.tab.hh location.hh lang.output
	rm -f lang.yy.cc
	rm -f testtxt.iff testbin.iff test.wf testout.iff testout.iff.txt
	rm -f *.err
	rm -f $(PROGRAM)$(EXE)

.PHONY : realclean
realclean : clean
	rm -f *~

################################################################################
