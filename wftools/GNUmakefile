#=============================================================================
# wftools/GNUmakefile: Copyright (c) 1996-2002, World Foundry Group  
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
#=============================================================================

ifndef WF_DIR
WF_DIR= ../wfsource
endif


#=============================================================================

# don't compile under linux yet
##       aicomp \
#        chargrab \
#        imagedir \
#        wfbind \
#        xcd \	   		// ncd clone, since OpenCD and wcd exist, abandoned
#        iffdb \

DIR_LIST = \
        eval \
        iffdump \
        oaddump \
        prep \
        lvldump \
        iffcomp \
        iff2lvl \
        textile \

#=============================================================================

make_lib = echo =============================================================================== ; echo $(dir) ; echo =============================================================================== ; cd $(dir) ; make $(MFLAGS); echo =============================================================================== ; cd ..
# what the ...? this make_lib MUST be a mistake?

.PHONY : ALL
ALL : $(WF_DIR)/bin
	cd eval		&& ${MAKE} ${MFLAGS}
	cd iffdump	&& ${MAKE} ${MFLAGS}
	cd oaddump	&& ${MAKE} ${MFLAGS}
	cd prep		&& ${MAKE} ${MFLAGS}
	cd lvldump	&& ${MAKE} ${MFLAGS}
	cd iffcomp	&& ${MAKE} ${MFLAGS}
	cd textile	&& ${MAKE} ${MFLAGS}
	cd iff2lvl	&& ${MAKE} ${MFLAGS}


.PHONY : clean
clean :
	cd eval		&& ${MAKE} ${MFLAGS} $@
	cd iffdump	&& ${MAKE} ${MFLAGS} $@
	cd oaddump	&& ${MAKE} ${MFLAGS} $@
	cd prep		&& ${MAKE} ${MFLAGS} $@
	cd lvldump	&& ${MAKE} ${MFLAGS} $@
	cd iffcomp	&& ${MAKE} ${MFLAGS} $@
	cd textile	&& ${MAKE} ${MFLAGS} $@

.PHONY :  print
print: 
	echo $(DIR_LIST)

$(WF_DIR)/bin :
	mkdir $(WF_DIR)/bin

copybinaries: $(WF_DIR)/bin ALL
	cp iffcomp/iffcomp $(WF_DIR)/bin
	cp iff2lvl/iff2lvl $(WF_DIR)/bin
	cp prep/prep $(WF_DIR)/bin
	cp iffdump/iffdump $(WF_DIR)/bin
	cp lvldump/lvldump $(WF_DIR)/bin
	cp textile/textile $(WF_DIR)/bin

#=============================================================================

# not really usefull under linux
#       psxprof \
#	link \					# playstation linker
#	psxdisc \
#disr3000 \     [ need to find dumpobj.cc ]

## NOTE: where is bumpver?

## Not built anymore?
##       stripper \

# template has been moved into the wfsource tree
#        template \

