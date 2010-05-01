################################################################################
## Makefile
## Copyright 1997 Recombinant Limited.  All Rights Reserved.
## by William B. Norris IV
################################################################################

WF_DIR ?= ..
WF_TARGET ?= win

ALL : SOURCE

################################################################################

SOURCE .PHONY :
	@%chdir $(WF_DIR)/source
	make
	@%chdir $(CWD)


NOTHING :

################################################################################

shell .PHONY : $(WF_DIR)\assets\$(WF_TARGET)\shell.aib

$(WF_DIR)\assets\$(WF_TARGET)\shell.aib : shell.s
	aicomp $(.SOURCE)
	@move /q shell.aib $(.TARGET)

################################################################################

cd .PHONY : $(WF_DIR)/assets/$(WF_TARGET)/cthug.wf

levels.txt : cd.pl

$(WF_DIR)/assets/$(WF_TARGET)/cd.iff.txt : levels.txt
	perl cd.pl <$(.SOURCE) >$(.TARGET)

$(WF_DIR)/assets/$(WF_TARGET)/cthug.wf : $(WF_DIR)\assets\$(WF_TARGET)\shell.aib $(WF_DIR)/assets/$(WF_TARGET)/cd.iff.txt
	@%chdir $(WF_DIR)/assets/$(WF_TARGET)
	iffcomp -o=cthug.wf cd.iff.txt
	@%chdir $(CWD)

################################################################################

clean .PHONY :
        -del $(WF_DIR)\assets\$(WF_TARGET)\cd.iff.txt
        -del $(WF_DIR)\assets\$(WF_TARGET)\cthug.wf
        -del $(WF_DIR)\assets\$(WF_TARGET)\shell.aib
