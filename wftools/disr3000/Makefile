#==============================================================================
# Makefile for disr3000: Copyright (c) 1995-1999, World Foundry Group  
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

.EXTENSIONS:
.EXTENSIONS:	.obj .cc .c

PROGRAM = disr3000

OBJS = disr3000.obj field.obj mask.obj util.obj symbol.obj fn.obj patch.obj section.obj main.obj
#dumpobj.obj

VERSION = version


CFLAGS = -nologo -GX -TP -c

#LIB     = d:\velsrc\pigs2\mc21\source\mc21r.lib

.cc.obj :
	$(CC) $(CFLAGS) $(.SOURCE)

.cc:	.

$(PROGRAM).exe : $(OBJS)
	bumpver version.cc
#	wpp386 -zq version
	link /nologo /out:$(.TARGET) $(.SOURCES)
	wlink @$(LINKFILE)

##############################################################################

clean .PHONY :
	-del /q *.obj $(PROGRAM).exe >& nul
	-del /q *.err err >& nul
	-del /q tempfile.bin *.ln $(PROGRAM).map >& nul
	-del /q memcheck.* >& nul
	-del /q room*.* perm*.* palette.tga >& nul

##############################################################################

test .PHONY :
	disr3000 test\scalar.obj >&test3.txt

debug .PHONY :
	disr3000 -addr=0x802CB6D4 diss.bin mine.txt
