#===============================================================================
# Makefile for iff2lvl: Copyright (c) 1999, World Foundry Group  
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

.MS_NMAKE

ALL : $(PROGRAM).exe

OBJ_DIR = #$(PROGRAM).obj
OBJS = \
	3d.obj \
	assert.obj \
	Asset.obj \
	channel.obj \
	Colbox.obj \
	Hdump.obj \
	Level.obj \
	Level2.obj \
	Level3.obj \
	Level4.obj \
	Level5.obj \
	Levelcon.obj \
	Loadfile.obj \
	Oad.obj \
	Object.obj \
	Path.obj \
	Point6.obj \
	Room.obj \
	Symbol.obj \


#	Template.obj \



PIGS_LIBS += $(RECOLIB) $(EVAL) $(REGEXP) $(STREAMS) $(IFF) $(REGISTRY) $(MEMORY)
!include ../Makefile.tool

#INCLUDE += -I$(WF_TOOL_DIR)\iff2lvl\stl
#INCLUDE += -Id:\wf\wftools\iff2lvl\stl
INCLUDE += -I.\ -I.\stl



#$(PROGRAM).exe: $(OBJS) $(PIGS_LIBS)
#	echo Linking tool
#        link /debug /LIBPATH:"$(MSVCDir)\lib" /OUT:$(.TARGET) $(.SOURCES) 

##############################################################################

clean : tool-clean

##############################################################################

