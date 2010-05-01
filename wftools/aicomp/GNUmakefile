#==============================================================================
# Makefile for aicomp (AI Compiler) : Copyright (c) 1996-1999, World Foundry Group  
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

LANGUAGE ?= scheme
LANGUAGE ?= c

PROGRAM = aicomp_$(LANGUAGE)

OBJ_DIR = $(PROGRAM).obj/

ALL : $(OBJ_DIR) $(LANGUAGE) $(PROGRAM) airun aidump

OBJS = \
        $(OBJ_DIR)lex$(LANGUAGE)$(OBJ) \
        $(OBJ_DIR)$(LANGUAGE)ai_tab$(OBJ) \
        $(OBJ_DIR)ailexer$(OBJ) \
        $(OBJ_DIR)grammar$(OBJ) \
        $(OBJ_DIR)backpatch$(OBJ) \
        $(OBJ_DIR)function$(OBJ) \
        $(OBJ_DIR)strtable$(OBJ) \
        $(OBJ_DIR)fileline$(OBJ) \
        $(OBJ_DIR)beep$(OBJ) \
        $(OBJ_DIR)aicomp$(OBJ) \
        $(OBJ_DIR)symbol$(OBJ) \
        $(OBJ_DIR)flex$(OBJ) \
        $(OBJ_DIR)filesize$(OBJ) \
        $(OBJ_DIR)version$(OBJ) \

include ../GNUMakefile.tool

CCCARGS += -DYYDEBUG=1 -DYYERROR_VERBOSE=1
PIGS_LIBS += $(IFFWRITE)

$(LANGUAGE) :
	mkdir -p $(LANGUAGE)

##############################################################################

$(PROGRAM) : $(OBJ_DIR) obj/tokens_tab.h $(OBJS) $(PIGS_LIBS)

airun :     $(OBJ_DIR) obj/tokens_tab.h $(OBJ_DIR)airun$(OBJ) $(OBJ_DIR)filesize$(OBJ) $(OBJ_DIR)version$(OBJ) $(PIGS_LIBS)

aidump :    $(OBJ_DIR) obj/tokens_tab.h $(OBJ_DIR)aidump$(OBJ) $(OBJ_DIR)filesize$(OBJ) $(OBJ_DIR)version$(OBJ) $(PIGS_LIBS)

##############################################################################

ai.l : ai_tab.cc.h

$(LANGUAGE)ai_tab.cc $(LANGUAGE)ai_tab.cc.h : $(LANGUAGE).y ../bin/bison.simple
	bison --defines --verbose --output=$(LANGUAGE)ai_tab.cc $(LANGUAGE).y
	

obj/tokens_tab.h : $(LANGUAGE)ai_tab.cc.h
	cp $(LANGUAGE)ai_tab.cc.h obj/tokens_tab.h

lex$(LANGUAGE).cc : $(LANGUAGE).l
	flex -8 -Caf -p -t -+ $(LANGUAGE).l >lexyy.cc
	@mv lexyy.cc lex$(LANGUAGE).cc
	@perl -i~ -p -n ../GNUfixlex.pl lex$(LANGUAGE).cc
	@-rm lex$(LANGUAGE).cc~

ailexer.cc : 

symbol.cc : 

ailexer.hp : 

##############################################################################

clean : tool-clean
	-rm $(OBJ_DIR)*
	-rm lex$(LANGUAGE).cc
	-rm $(LANGUAGE)ai_tab.cc $(LANGUAGE)ai_tab.cc.h
	-rm *.*~
	-rm *.out
	-rm *.dll *.lib *$(OBJ) $(PROGRAM)
	-rm *.opt *.lcf *.pdb *.ilk *.ncb *.plg
	-rm *.aib *.log.htm


.PHONY : realclean test-scheme test-c debug-scheme test testall printlocal

realclean  :
	make clean LANGUAGE=scheme
	make clean LANGUAGE=c

test-scheme : aicomp_$(LANGUAGE).exe
	aicomp_scheme.exe test/test.scm
	start test/test.log.htm

debug-scheme : aicomp_$(LANGUAGE).exe
	msdev aicomp_scheme.exe test/test.scm

test-c : aicomp_$(LANGUAGE).exe
	aicomp_c.exe test/test.c
	start test/test.log.htm

test  : test-$(LANGUAGE)

testall : test-scheme test-c

printlocal :
	@echo LANGUAGE = $(LANGUAGE) [*scheme*,c]

###############################################################################
# kts this sucks, figure out a way to make a rule to do this (kinda hard since programs have no extension)

airun: 
	echo ------------------------------------------------------------------------------
	@echo Creating \> \> $@ \< \<
	@echo ------------------------------------------------------------------------------
# Leaving PIGS_LIBS in, as it is already defined.
	gcc -o $@  $(OBJS) \
	-Xlinker --start-group \
	$(RECOLIB) \
	$(EVAL) \
	$(REGEXP) \
	$(PIGSYS) \
	$(MATH) \
	$(STREAMS) \
	$(MEMORY) \
	$(IFF) \
	$(IFFWRITE) \
	$(LOADFILE) \
	$(TOOLSTUB) \
	$(CPPLIB) \
	$(LIBS) \
	-Xlinker --end-group \
	$(LNARGS) -L/lib -L/usr/lib \
	-lm -lX11 -lXext -lXmu -lXt -lXi -lSM -lICE -lpthread -lc -lstdc++ 	


aidump:
	echo ------------------------------------------------------------------------------
	@echo Creating \> \> $@ \< \<
	@echo ------------------------------------------------------------------------------
# Leaving PIGS_LIBS in, as it is already defined.
	gcc -o $@  $(OBJS) \
	-Xlinker --start-group \
	$(RECOLIB) \
	$(EVAL) \
	$(REGEXP) \
	$(PIGSYS) \
	$(MATH) \
	$(STREAMS) \
	$(MEMORY) \
	$(IFF) \
	$(IFFWRITE) \
	$(LOADFILE) \
	$(TOOLSTUB) \
	$(CPPLIB) \
	$(LIBS) \
	-Xlinker --end-group \
	$(LNARGS) -L/lib -L/usr/lib \
	-lm -lX11 -lXext -lXmu -lXt -lXi -lSM -lICE -lpthread -lc -lstdc++ 	



