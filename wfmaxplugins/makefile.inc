################################################################################
## Makefile.inc
##
## Standardized common part of 3D Studio MAX PlugIns
## Copyright 1998 Recombinant Limited.  All Rights Reserved.
##
################################################################################
##
## Make directives:
##      clean:
##              delete object files for the current build configuration
##      realclean:
##              delete object files for all build configurations
##      run:
##              run 3d studio max with default project
##      debug:
##              run visual c++ debugger on this plugin
##			print:
##
################################################################################

.PATH.obj: max$(MAX_VERSION).obj
.PATH.cc = .;..\lib

ALL : $(.PATH.obj) $(THE_PLUGIN)

BUILDMODE = max
#!include $(WF_DIR)/Makefile.env

#PIGS_LIBS += \
#		pigsys.lib   \
#	    cpplib.lib   \
#	    math.lib     \
#	    streams.lib  \
#	    memory.lib   \
#	    toolstub.lib \


PIGS_TOOL = 1
#!include "$(WF_DIR)/source/pigs.dep"

.cc.obj :
	echo $(CPP) $(.SOURCE) /TP /nologo /c $(CCCARGS) /Fo$(.TARGET)
	$(CPP) $(.SOURCE) @<<
/TP /nologo /c $(CCCARGS) /Fo$(.TARGET)
<<

################################################################################


$(.PATH.obj):
	@if not exist $(.PATH.obj) mkdir $(.PATH.obj)

#STD_FILES = \
#	$(WF_TOOLS_DIR)\eval\eval.lib \
#	$(PIGS_LIBS)


CCCARGS += /G5 /Zp4 /MD /W3 /GX /Od /Zi \
	/I. /I"$(3DSMAX$(MAX_VERSION)_DIR)\maxsdk\include" /I"$(WF_DIR)\source" \
	/I ../lib -DWIN32 -D_WINDOWS -D__WIN__

MAX_LIB = $(3DSMAX$(MAX_VERSION)_DIR)\maxsdk\lib

LINK_FLAGS = \
	comctl32.lib comdlg32.lib user32.lib gdi32.lib kernel32.lib advapi32.lib \
	$(MAX_LIB)\core.lib $(MAX_LIB)\util.lib $(MAX_LIB)\geom.lib $(MAX_LIB)\mesh.lib \
	$(STD_FILES) \
	/debug /version:1.2 \
	/incremental:no /pdb:"$(OUTDIR)\$(PLUGIN_BASE).pdb" \
	/machine:I386

#	MSVCPRT.LIB \
#	msvcrt.lib \

################################################################################
################################################################################

$(PLUGIN_BASE).res : $(PLUGIN_BASE).rc
	rc $(PLUGIN_BASE).rc

################################################################################

#kts forcing BUILDMODE on command line prevents it from being modified in Makefile.bld
#	make BUILDMODE=max MULTITHREADED=m
	%chdir $(CWD)

################################################################################

$(WF_TOOLS_DIR)\eval\eval.lib :
	%chdir $(WF_TOOLS_DIR)\eval
	set BUILDMODE=max
	make 
#kts forcing BUILDMODE on command line prevents it from being modified in Makefile.bld
#	make BUILDMODE=max MULTITHREADED=m
	%chdir $(CWD)

################################################################################

$(PIGS_LIB_DIR)pigsys\pigsys.lib :
	%chdir $(PIGS_DIR)\pigsys
	make COMMANDLINE_BUILDMODE=dll
	%chdir $(CWD)

#-------------------------------------------------------------------------------

$(PIGS_LIB_DIR)math\math.lib :
	%chdir $(PIGS_DIR)\math
	make WF_TARGET=win BUILDMODE=dll STREAMING=off MULTITHREADED=m
	%chdir $(CWD)

#-------------------------------------------------------------------------------

$(PIGS_LIB_DIR)streams\streams.lib :
	%chdir $(PIGS_DIR)\streams
	make WF_TARGET=win BUILDMODE=dll STREAMING=off MULTITHREADED=m
	%chdir $(CWD)

#-------------------------------------------------------------------------------

$(PIGS_LIB_DIR)cpplib\cpplib.lib :
	%chdir $(PIGS_DIR)\cpplib
	make COMMANDLINE_BUILDMODE=dll
	%chdir $(CWD)

#-------------------------------------------------------------------------------

$(PIGS_LIB_DIR)toolstub\toolstub.lib :
	%chdir $(PIGS_DIR)\toolstub
	make WF_TARGET=win BUILDMODE=dll STREAMING=off MULTITHREADED=m
	%chdir $(CWD)

################################################################################

clean :
	del max$(MAX_VERSION).obj\*.obj *.res
	del $(THE_PLUGIN)
	del *.pdb *.ncb *.plg

#-------------------------------------------------------------------------------

realclean : clean
	del /S *.obj *.res

################################################################################

debug : $(THE_PLUGIN)
	msdev $(PLUGIN_BASE).dsw

#-------------------------------------------------------------------------------

run : $(THE_PLUGIN)
	$(3DSMAX$(MAX_VERSION)_DIR)\3dsmax.exe "d:\wfsrc\levels.src\snowgoons\snowgoons R$(MAX_VERSION).max"

################################################################################

print :
	%echo +-----------------------------------------------------------------------------+
	%echo + PIGS -----------------------------------------------------------------------+
	%echo PIGS_INCLUDES     = "$(PIGS_INCLUDES)"
	%echo BUILDMODE         = "$(BUILDMODE)"
	%echo BUILDMODE_LETTERS = "$(BUILDMODE_LETTERS)"
	%echo OBJ_LETTERS       = "$(OBJ_LETTERS)"
	%echo MULTITHREADED     = "$(MULTITHREADED)"
	%echo PIGS_LIBS         = "$(PIGS_LIBS)"
	%echo +-----------------------------------------------------------------------------+
	%echo PIGS_LIBS_DIR = "$(PIGS_LIBS_DIR)"
	%echo PIGS_OBJS_DIR = "$(PIGS_OBJS_DIR)"
	%echo WF_DIR        = "$(WF_DIR)"
	%echo WF_OBJ_DIR    = "$(WF_OBJ_DIR)"
	%echo PIGS_DIR      = "$(PIGS_DIR)"
	%echo PIGS_OBJ_DIR  = "$(PIGS_OBJ_DIR)"
	%echo +-----------------------------------------------------------------------------+
	%echo STD_FILES         = "$(STD_FILES)"
	%echo LIBS         = "$(LIBS)"
	@echo [ CCCARGS = ]
	@echo $(CCCARGS)

################################################################################

