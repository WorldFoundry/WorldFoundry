# Makefile to create a World Foundry disc image suitable for the Sony PlayStation
# by William B. Norris IV
# Copyright 1998-99 World Foundry Group

SRC_DIR = \\t-rex\users\kevin\forcd
#SRC_DIR = \projects\velsrc\assets\psx
#SRC_DIR = d:\projects\wfsource\assets\psx
#SRC_DIR = c:\temp

ALL : clean disc.raw

disc.raw :
	@attrib -r bin\cdrdos.dat
	@rem =========================================================================
	@rem Make temp places
	-md wfsrc
	-md temp
	@rem =========================================================================
	@rem Copy World Foundry files required for CD distribution from standard place
	copy $(SRC_DIR)\cd.iff wfsrc
	copy $(SRC_DIR)\wfreleasepsxpsxcd.cpe temp\wfrc.cpe
	@rem =========================================================================
	@rem Convert .cpe into .exe, and rename it the standard "psx.exe"
	bin\cpe2x /CA temp\wfrc.cpe
	copy temp\wfrc.exe wfsrc\psx.exe
	@rem =========================================================================
	@rem Create a dummy 2K sector file
	$(WF_DIR)\bin\iffcomp -o=wfsrc\aaaaaaaa.iff sector.txt.iff
	@rem Copy any Windows files
	copy /s win\*.* wfsrc
	@rem =========================================================================
	@rem Convert the wfsrc directory into an ISO9660 disc image
	@%chdir wfsrc
	..\bin\makeiso d: ..\temp\wfdisc.iso /BATCH /VOLUME="WF"
	@%chdir ..
	@rem =========================================================================
	@rem Get sector 21 [Table of Contents]
	bin\xtract temp\wfdisc.iso temp\s21.bin 43008 2048
	@rem =========================================================================
	@rem Get sector 23 onward [our game data and executable]
	bin\xtract temp\wfdisc.iso temp\s23-.bin 47104 -1
	copy /b temp\s21.bin+temp\s23-.bin temp\wf.bin
	@rem =========================================================================
	@rem Merge into a single file [basically removes sector 22]
	@rem Prepend a standard PlayStation disc header [I got this one from "One"]
	copy /b one0-21.bin+temp\wf.bin disc.bin
	@rem =========================================================================
	@rem Convert the disc image from 2048 byte sectors to 2352 raw sectors
	bin\iso2raw disc.bin disc.raw /MODE2
	@del disc.bin
	@rem =========================================================================
	@pause Press any key to burn the CD
	bin\dao disc.cue disc.log /SPEED=1
	@type disc.log

clean .PHONY :
	@rem =========================================================================
	@rem Clean up temp files
	-del /Y wfsrc\*.*
	-del /Y temp\*.*
	-del disc.bin

realclean .PHONY : clean
	-del disc.raw disc.log
