#==============================================================================
# Makefile for psxprof: Copyright (c) 1996-1999, World Foundry Group  
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
PROGRAM = psxprof
#==============================================================================

.MS_NMAKE

ALL : $(PROGRAM).exe

OBJ_DIR = #
OBJS = $(PROGRAM).obj main.obj symbol.obj sample.obj version.obj

!include ../Makefile.tool

$(PROGRAM).exe: $(OBJS) $(PIGS_LIBS)

##############################################################################

clean : tool-clean
	-del /q room*.* perm*.* palette.tga >& nul

################################################################################

test .PHONY : $(PROGRAM).exe test.map test.smp
	$(PROGRAM).exe -relative test.map test.smp >test-rel.htm
	start test-rel.htm
#	$(PROGRAM).exe -absolute test.map test.smp >test-abs.htm
#	start test-abs.htm

################################################################################

debug .PHONY : $(PROGRAM).exe test.map test.smp
	msdev $(PROGRAM).exe -relative test.map test.smp >test-rel.htm

