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

PROGRAM = iff2lvl

ALL: REALALL


#OBJ_DIR = #$(PROGRAM)$(OBJ)
OBJS = \
	$(OBJ_DIR)3d$(OBJ) \
	$(OBJ_DIR)assert$(OBJ) \
	$(OBJ_DIR)asset$(OBJ) \
	$(OBJ_DIR)channel$(OBJ) \
	$(OBJ_DIR)colbox$(OBJ) \
	$(OBJ_DIR)hdump$(OBJ) \
	$(OBJ_DIR)level$(OBJ) \
	$(OBJ_DIR)level2$(OBJ) \
	$(OBJ_DIR)level3$(OBJ) \
	$(OBJ_DIR)level4$(OBJ) \
	$(OBJ_DIR)level5$(OBJ) \
	$(OBJ_DIR)levelcon$(OBJ) \
	$(OBJ_DIR)loadfile$(OBJ) \
	$(OBJ_DIR)oad$(OBJ) \
	$(OBJ_DIR)object$(OBJ) \
	$(OBJ_DIR)path$(OBJ) \
	$(OBJ_DIR)point6$(OBJ) \
	$(OBJ_DIR)room$(OBJ) \
	$(OBJ_DIR)symbol$(OBJ) 

include ../GNUMakefile.tool

REALALL : $(OBJ_DIR) $(PROGRAM)$(EXE)

PIGS_LIBS += $(PIGSYS) $(STREAMS) $(MEMORY) $(CPPLIB) $(MATH) $(TOOLSTUB) $(IFFWRITE)

#INCLUDE += -I$(WF_TOOL_DIR)\iff2lvl\stl
#INCLUDE += -Id:\wf\wftools\iff2lvl\stl
INCLUDE += -I.\ -I.\stl

$(PROGRAM)$(EXE): $(OBJS) $(PIGS_LIBS)

##############################################################################

clean : tool-clean
	rm -f lang_tab.cc lang_tab.cc.h lang_tab.cc.output lexyy.cc 

##############################################################################
