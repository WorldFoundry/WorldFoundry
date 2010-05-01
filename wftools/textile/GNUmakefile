#==============================================================================
# GNUmakefile for textile : Copyright (c) 1999, World Foundry Group  
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

#.MS_NMAKE

ALL: REALALL


#OBJ_DIR = #$(PROGRAM)$(OBJ)
OBJS = \
	$(OBJ_DIR)main$(OBJ) \
	$(OBJ_DIR)texture$(OBJ) \
	$(OBJ_DIR)bitmap$(OBJ) \
	$(OBJ_DIR)bmp$(OBJ) \
	$(OBJ_DIR)tga$(OBJ) \
	$(OBJ_DIR)sgi$(OBJ) \
	$(OBJ_DIR)ccycle$(OBJ) \
	$(OBJ_DIR)srccntrl$(OBJ) \
	$(OBJ_DIR)allocmap$(OBJ) \
	$(OBJ_DIR)profile$(OBJ) \
	$(OBJ_DIR)locfile$(OBJ) \
	$(OBJ_DIR)version$(OBJ)

include ../GNUMakefile.tool

REALALL : $(OBJ_DIR) $(PROGRAM)$(EXE) rmuvdump$(EXE)

#PIGS_LIBS += $(PIGSYS) $(STREAMS) $(MEMORY) $(CPPLIB) $(MATH) $(TOOLSTUB) $(IFFWRITE)
INCLUDE += -I .                 # for unistd.h placeholder

$(PROGRAM)$(EXE): $(OBJS) $(PIGS_LIBS)

##############################################################################

clean : tool-clean

################################################################################

#.cc:	.;readers;writers

# kts temporary rule for link, since I can't get the included makefiles to do it for me
rmuvdump$(EXE): $(OBJ_DIR)rmuvdump$(OBJ) $(OBJ_DIR)rmuv$(OBJ) $(PIGS_LIBS)
	@echo ------------------------------------------------------------------------------
	@echo Creating \> \> $@ \< \<
#	@echo ------------------------------------------------------------------------------
ifeq ($(WF_TARGET),linux)
	@echo ------------------------------------------------------------------------------
	gcc -o $@  $(OBJS) -Xlinker --start-group $(PIGS_LIBS) -Xlinker --end-group $(LNARGS) $(SYS_LIBS)
#	The line below doesn't work -- why?
#	link /nologo /out:$(.TARGET) -libpath:"$(MSVCDIR)/lib" /subsystem:$(SUBSYSTEM) /debug /stack:1050624 $^
endif




#        cl /TP /c version.cc
#        link /LIBPATH:"$(MSVCDir)\lib" /OUT:$(.TARGET) $(.SOURCES) version$(OBJ)

##############################################################################

#test .PHONY : textile.exe
#	textile.exe -Tpsx -transparent=0,255,0 -colourcycle=ccycle.ini -powerof2size -ini=test.ini %1 %2 %3 %4 %5 %6 %7 %8 %9
#	start textile.log.htm

##############################################################################

#debug .PHONY : textile.exe
#	msdev textile.exe -Tpsx -transparent=0,255,0 -colourcycle=ccycle.ini -powerof2size -ini=test.ini %1 %2 %3 %4 %5 %6 %7 %8 %9

##############################################################################

#clean .PHONY : test-clean
#	-del /q room*.* perm*.* palette.tga >& nul

##############################################################################
