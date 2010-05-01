################################################################################
## Makefile
## Copyright 1997 Recombinant Limited.  All Rights Reserved.
## by William B. Norris IV
################################################################################

WF_DIR ?= ..
WF_TARGET ?= win
BINDIR = $(WF_DIR)\bin\				# just here so that \ is not the last character on the line

ALL : cd

NOTHING :

################################################################################

shell .PHONY : $(WF_DIR)\assets\$(WF_TARGET)\shell.tcl

$(WF_DIR)\assets\$(WF_TARGET)\shell.tcl : shell.tcl
	copy shell.tcl $(.TARGET)

################################################################################
# rule to manufacture makefile.levels 

makefile.levels : levels.txt makefile.levels.pl
	$(BINDIR)perl makefile.levels.pl <$(.SOURCE) >makefile.levels 


################################################################################

cd .PHONY : CDBANNER $(WF_DIR)\assets\$(WF_TARGET)\cd.iff


# kts I can't get this to work

LEVELS .PHONY : makefile.levels
	@echo Building levels
	@echo -------------------------------------------------------------------------------
	make -f makefile.levels


CDBANNER .PHONY :
	@echo Creating CD image
	@echo -------------------------------------------------------------------------------

levels.txt : cd.pl

$(WF_DIR)\assets\$(WF_TARGET)\cd.iff.txt : levels.txt  $(WF_DIR)\assets\$(WF_TARGET)\shell.tcl
	$(BINDIR)perl cd.pl <$(.SOURCE) >temp.out
	copy temp.out $(.TARGET)
	del temp.out

$(WF_DIR)\assets\$(WF_TARGET)\cd.iff : $(WF_DIR)\assets\$(WF_TARGET)\cd.iff.txt
	@%chdir $(WF_DIR)\assets\$(WF_TARGET)
	$(BINDIR)iffcomp -o=$(.TARGET) $(.SOURCE)
	@%chdir $(CWD)

################################################################################

clean .PHONY  :
	-del /q $(WF_DIR)\assets\$(WF_TARGET)\cd.iff.txt
	-del /q $(WF_DIR)\assets\$(WF_TARGET)\cd.iff
	make -f makefile.levels clean
	-del makefile.levels
