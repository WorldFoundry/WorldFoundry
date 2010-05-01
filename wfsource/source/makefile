#=============================================================================
# source/Makefile
#=============================================================================

TOPALL: REALALL


!include ../Makefile.env

WF_TARGET ?= win


DIR_LIST =
DIR_LIST += console cpplib eval iff iffwrite ini loadfile math memory pigsys scripting recolib regexp registry savegame streams timer template toolstub input

DIR_LIST_win = cdda
DIR_LIST_psx = bootscrn movie profiler
DIR_LIST += $(DIR_LIST_$(WF_TARGET))

!if "$(BUILDMODE)" != "tool"
DIR_LIST += mailbox oas 
DIR_LIST += physics movement baseobject

## Keep this last since it's the only library that can't be built in MULTITHREADED mode
DIR_LIST += hal

!if "$(RENDERER)" != "none"
# if building in tool mode, don't need these
DIR_LIST += gfx asset gfxfmt anim renderassets menu particle audio audiofmt room game
!endif
!endif

%if $(OS) == OS2 || $(OS) == NT
; = &
%else
; = ;
%endif


!if "$(BUILDMODE)" != "tool"
#rule to build tools
$(WF_DIR)/bin/prep.exe : 
	( cd $(WF_DIR)\..\wftools $; make /nologo copybinaries )

 
TOPDEPS += $(WF_DIR)/bin/prep.exe

!endif	

	       

REALALL : $(TOPDEPS)
	%foreach Dir $(DIR_LIST)
                ( cd $(Dir) $; make /nologo $(MFLAGS) )
	%end


print :
        ( cd pigsys $; make /nologo $(MFLAGS) $(.TARGET) )
	@echo ========================================================================
	@echo DIR_LIST = $(DIR_LIST)


print_short :
        ( cd pigsys $; make /nologo $(MFLAGS) $(.TARGET) )


test : ALL
	%foreach Dir $(DIR_LIST)
                ( cd $(Dir) $; make /nologo $(MFLAGS) $(.TARGET) )
	%end


clean :
	%foreach Dir $(DIR_LIST)
                ( cd $(Dir) $; make /nologo $(MFLAGS) $(.TARGET) )
	%end


clean-lib :
	%foreach Dir $(DIR_LIST)
                ( cd $(Dir) $; make /nologo $(MFLAGS) $(.TARGET) )
	%end


libstrm:
	del libstrm.inc
	%foreach Dir $(DIR_LIST)
                ( cd $(Dir) $; if exist libstrm.inc type libstrm.inc >>..\libstrm.inc )
	%end
