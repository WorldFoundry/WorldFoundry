################################################################################
## GNUmakefile : unix version of makefile to build levels & cd.iff
## Copyright 2001 World Foundry Group
## by William B. Norris IV & Kevin T. Seghetti
################################################################################

WF_TARGET ?= linux

ifndef WF_DIR
	WF_DIR:=$(shell pwd | sed -e "s/^\(.*wfsource\).*/\1/")
	export WF_DIR
endif


ASSET_DIR = $(WF_DIR)/assets/$(WF_TARGET)

ALL : cd

NOTHING :

################################################################################

$(ASSET_DIR) :  
	@echo Creating directory $(ASSET_DIR)
	mkdir -p $(ASSET_DIR)

$(ASSET_DIR)/shell.tcl : shell.tcl
	cp  shell.tcl $@

################################################################################
# rule to manufacture makefile.levels 

makefile.levels : $(ASSET_DIR) levels.txt unixmakefile.levels.pl
	perl unixmakefile.levels.pl <levels.txt >makefile.levels 

################################################################################

.PHONY : LEVELS
LEVELS :  makefile.levels
	@echo Building levels
	@echo -------------------------------------------------------------------------------
	make -f makefile.levels



.PHONY : cd
cd : LEVELS CDBANNER $(ASSET_DIR)/cd.iff

.PHONY : CDBANNER
CDBANNER :
	@echo Creating CD image
	@echo -------------------------------------------------------------------------------

levels.txt : cd.pl

$(ASSET_DIR)/cd.iff.txt : levels.txt $(ASSET_DIR)/shell.tcl
	@perl cd.pl <$< >$@

$(ASSET_DIR)/cd.iff : $(ASSET_DIR)/cd.iff.txt
	cd $(ASSET_DIR) ; $(WF_DIR)/bin/iffcomp -o=cd.iff cd.iff.txt

################################################################################

clean .PHONY :
	-rm $(ASSET_DIR)/cd.iff.txt
	-rm $(ASSET_DIR)/cd.iff
	-rm $(ASSET_DIR)/shell.tcl
	make -f makefile.levels clean
	-rm makefile.levels
	
