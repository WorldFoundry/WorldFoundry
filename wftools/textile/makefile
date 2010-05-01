#==============================================================================
# Makefile for textile: Copyright (c) 1996-1999, World Foundry Group  
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

PROGRAM = textile

.MS_NMAKE

ALL : $(PROGRAM).exe rmuvdump.exe

OBJ_DIR = #
OBJS = main.obj texture.obj bitmap.obj bmp.obj tga.obj sgi.obj ccycle.obj srccntrl.obj allocmap.obj profile.obj locfile.obj version.obj

!include ../Makefile.tool

.cc:	.;readers;writers

textile.exe : $(OBJS) $(PIGS_LIBS)

rmuvdump.exe: rmuvdump.obj rmuv.obj version.obj $(PIGS_LIBS)
#        cl /TP /c version.cc
#        link /LIBPATH:"$(MSVCDir)\lib" /OUT:$(.TARGET) $(.SOURCES) version.obj

##############################################################################

test .PHONY : textile.exe
	textile.exe -Tpsx -transparent=0,255,0 -colourcycle=ccycle.ini -powerof2size -ini=test.ini %1 %2 %3 %4 %5 %6 %7 %8 %9
	start textile.log.htm

##############################################################################

debug .PHONY : textile.exe
	msdev textile.exe -Tpsx -transparent=0,255,0 -colourcycle=ccycle.ini -powerof2size -ini=test.ini 

##############################################################################

clean .PHONY : test-clean
	-del /q room*.* perm*.* palette.tga >& nul

##############################################################################
