################################################################################################################################
# Makefile.inc
################################################################################################################################

TOP-ALL :       LIBRARY_NAMES_DEFINES LIB_DIR PIGS_LIB

INCLUDE += -I..\hal\$(WF_TARGET) -I$(WF_TARGET) -I..

TARGET           = $(WF_TARGET,UC)


%if $(os) == OS2 || $(OS) == NT
; = &
%else
; = ;
%endif

################################################################################################################################
# Windows 95/Windows NT
!if "$(WF_TARGET)" == "win"

LIBS    += "$(MSVCDIR)/lib/user32.lib" "$(MSVCDIR)/lib/gdi32.lib" "$(MSVCDIR)/lib/winmm.lib" "$(MSVCDIR)/lib/advapi32.lib"
CCCARGS += -DSTRICT -DNOMINMAX -DWIN32_LEAN_AND_MEAN -DD3D_OVERLOADS


!if "$(RENDERER)" == "gl"
LIBS +=	"$(MSVCDIR)/lib/opengl32.lib" "$(MSVCDIR)/lib/glu32.lib" "$(MSVCDIR)/lib/glaux.lib"
!endif


!if "$(RENDERER)" == "directx"
LIBS +=	"$(DIRECTX5_DIR)/lib/ddraw.lib"
LIBS +=	"$(DIRECTX5_DIR)/lib/dxguid.lib"
!endif

## C Compiler
CC                              = cl /nologo -TC
CCC                             = cl /nologo -TP
CCCARGS                         += /Zp4 /Gi /G5 /YX /GX $(CDEFS)

!if "$(BUILD_DO_DEBUGGING_INFO)" == "1"
CCCARGS += -Yd -Zi -Gm
!endif

!if "$(BUILD_DO_OPTIMIZATIONS)" == "1"
CCCARGS += -O2 -Ox -Ob2 -GA -Zd
!endif

## Librarian
AR                                                      = lib /nologo
LIBOUT                                  = /out:

## Executable
EXE = .exe
## Resources
RESOURCE                                = $(LIB_OBJ_DIR)$(TEST_PROG).res
!endif

#################################################################################################################################
## Sony Playstation
!if "$(WF_TARGET)" == "psx"

## C Compiler
CC       = ccpsx
CCC      = ccpsx
CCCARGS  += $(CDEFS) -Wall -Wpointer-arith -Wcast-align -comments-c++
CCCARGS  += -DUSE_3T_MATRICES=1
LIBS    += $(PSX_DIR)\lib\libsn.lib $(PSX_DIR)\lib\libapi.lib $(PSX_DIR)\lib\libetc.lib $(PSX_DIR)\lib\libgpu.lib $(PSX_DIR)\lib\libgs.lib $(PSX_DIR)\lib\libgte.lib $(PSX_DIR)\lib\libcd.lib $(PSX_DIR)\lib\libsnd.lib $(PSX_DIR)\lib\libspu.lib $(PSX_DIR)\lib\libmath.lib $(PSX_DIR)\lib\libcard.lib $(PSX_DIR)\lib\libpress.lib
LIBS += $(PSX_DIR)\lib\libc2\libc2.lib
#$(PSX_DIR)\lib\libc2.lib

INCLUDE += -I..\pigsys\psx -I$(PSX_DIR)/include

## Linker
LINKPOST = ,$(TEST_PROG)$(EXE),$(TEST_PROG).sym,$(TEST_PROG).map
## Librarian
AR                      = $(PSX_DIR)\bin\psylib /u
LIBOUT  =
## Executable
EXE = .cpe
RUN = run

!if $(BUILD_DO_DEBUGGING_INFO) == "1"
CCCARGS += -g
!endif

!if $(BUILD_DO_OPTIMIZATIONS) == "1"
CCCARGS += -O3 -mgpopt -G 1000
!endif
!endif          # PSX

################################################################################################################################
## BUILDMODE (Debug vs. Release switches)

CCCARGS += -DDO_DEBUGGING_INFO=$(BUILD_DO_DEBUGGING_INFO) -DDEBUG=$(BUILD_DO_DEBUG_VARIABLES)
CCCARGS += -DDO_ASSERTIONS=$(BUILD_DO_ASSERTIONS) -DDEBUG_VARIABLES=$(BUILD_DO_DEBUG_VARIABLES)
CCCARGS += -DDO_VALIDATION=$(BUILD_DO_VALIDATION) -DDO_TEST_CODE=$(BUILD_DO_TEST_CODE)
CCCARGS += -DDO_DEBUG_FILE_SYSTEM=$(BUILD_DO_DEBUG_FILE_SYSTEM)
CCCARGS += -DDO_IOSTREAMS=$(BUILD_DO_IOSTREAMS)


!if "$(BUILD_DO_ASSERTIONS)" == "0"
CCCARGS += -DNDEBUG
!endif

################################################################################################################################
## Multithreaded switches [Windows only]
!if "$(WF_TARGET)" == "win"
CCCARGS_MULTITHREADED_S =
CCCARGS_MULTITHREADED_M = -MD
CCCARGS += $(CCCARGS_MULTITHREADED_$(MULTITHREADED,UC))
!endif

################################################################################################################################

CCCARGS += -D__$(TARGET)__

################################################################################################################################
## The library

LIB_DIR .ALWAYS :
	@-if not exist $(PIGS_OBJS_DIR) mkdir $(PIGS_OBJS_DIR) >&nul
	@-if not exist $(PIGS_OBJ_DIR) mkdir $(PIGS_OBJ_DIR) >&nul
	@-if not exist $(PIGS_LIB_DIR) mkdir $(PIGS_LIB_DIR) >&nul
	@-if not exist $(LIB_OBJ_DIR) mkdir $(LIB_OBJ_DIR) >&nul

PIGS_LIB :   $(PIGS_LIB)

#.RESPONSE.PSYQ : pre=@ suf=.rsp in=128 psylib

$(PIGS_LIB) :   $(LIB_OBJS)
	$(AR)   $(LIBOUT)$(.TARGET)  @<<
		$(.SOURCES)
	<<

################################################################################################################################
## Test program

# kts wish the loop executed in pass 1, not as a rule
LIBRARY_NAMES_DEFINES .ALWAYS .SILENT :
	%foreach Dir pigsys cpplib iff math streams anim attrib audio audiofmt gfxfmt console gfx hal input savegame toolstub version ini fuzzy physical movement loadfile audiofmt gfxfmt attrib particle timer menu
		%set $(Dir,uc) $(PIGS_OBJ_DIR)\$(Dir,lc).lib
	%end

reset_target :
!if "$(WF_TARGET)" == "psx"
	$(PSX_DIR)\bin\resetps 1
	$(PSX_DIR)\bin\run /w2 $(PSX_DIR)\bin\snpatch
	$(COMSPEC) delay 2^exit
!endif

run_test : $(LIB_OBJ_DIR)$(TEST_PROG)$(EXE)
	-$(RUN) $(.SOURCE) $(TEST_PROG_PARAMS)

post_run :
!if "$(WF_TARGET)" == "psx"
	testmess
!endif

link : LIB_DIR $(PIGS_LIB) $(LIB_OBJ_DIR)$(TEST_PROG)$(EXE)


test .NOCOMPARE : LIBRARY_NAMES_DEFINES link reset_target run_test post_run


#TEST_LIBS += $(PIGSYS) $(PIGS_OBJ_DIR)\menu.lib $(PIGS_OBJ_DIR)\$(LIB_NAME).lib
TEST_LIBS += $(PIGSYS)  $(PIGS_OBJ_DIR)\$(LIB_NAME).lib
TEST_LIBS += $(LIBS)
#TEST_LIBS += $(PIGSYS) $(CPPLIB) $(IFF) $(MATH) $(STREAMS) $(PIGS_OBJ_DIR)\$(LIB_NAME)\$(LIB_NAME).lib

################################################################################################################################

debug : $(LIB_OBJ_DIR)$(TEST_PROG)$(EXE)
!if "$(WF_TARGET)" == "win"
	msdev $(.SOURCE) $(TEST_PROG_PARAMS)
!endif

################################################################################################################################

$(LIB_OBJ_DIR)$(TEST_PROG)$(EXE) : LIBRARY_NAMES_DEFINES $(LIB_OBJ_DIR)$(TEST_PROG)$(OBJ) $(TEST_OBJS) $(TEST_LIBS)
	%echo ------------------------------------------------------------------------------
	%echo Creating \> \> $(.TARGET) \< \<
#	%echo ------------------------------------------------------------------------------
!if "$(WF_TARGET)" == "win"
#	%echo TEST_LIBS = $(TEST_LIBS)
#	%echo ------------------------------------------------------------------------------
	link /nologo /out:$(.TARGET) /libpath:"$(MSVCDIR)\lib" /subsystem:$(SUBSYSTEM) /debug /stack:1050624 $(LIB_OBJ_DIR)$(TEST_PROG)$(OBJ) $(TEST_OBJS) $(TEST_LIBS)
#	The line below doesn't work -- why?
#	link /nologo /out:$(.TARGET) /libpath:"$(MSVCDIR)\lib" /subsystem:$(SUBSYSTEM) /debug /stack:1050624 $(.SOURCES)
!endif
!if "$(WF_TARGET)" == "psx"
	%echo $(TEST_LIBS)
	$(PSX_DIR)\bin\psylink /g /wm /m /c /n 1024 @<<
;
; Linker control file for psx version 
;
	org     $$80010000
	;
	; Library files
	;

text    group                   ; Group containing defined data
bss     group   bss             ; Group containing uninitialized data

	;
	; These sections are created by the C compiler.  Specifying them
	; here is not strictly necessary but does ensure that they appear
	; in the correct order in the linked output file.
	;
	section .rdata,text
	section .text,text
	section .data,text
	section .sdata,text

	section .sbss,bss       ; The bss sections
	section .bss,bss

	; The following line initializes the program counter register so
	; that the downloader sets the entry point before the program
	; is executed.  SN_ENTRY_POINT is defined in a module in
	; ligsn.lib and clears the bss, sets up the heap, etc., before
	; calling main().

	regs    pc=__SN_ENTRY_POINT

	INCLUDE $(LIB_OBJ_DIR)$(TEST_PROG)$(OBJ)

	%foreach Lib $(TEST_LIBS)
		INCLIB $(Lib)
	%end
	<< $(LINKPOST)
	move /q $(LIB_OBJ_DIR)$(EXE) $(PIGS_LIB_DIR)
!endif

################################################################################################################################
## Rules

.PATH.cc        = .;.\$(RENDERER_PIPELINE_DIR);$(RENDERER_DIR);.\$(WF_TARGET)\;.\$(WF_TARGET)\msvc          ## TODO: msvc part to go away
.PATH.c = .;$(WF_TARGET)
.PATH.asm       = .;$(WF_TARGET)

.c{$(LIB_OBJ_DIR)}$(OBJ) :
	@echo -------------------------------------------------------------------------------
	%echo      $(.SOURCE) =] $(.TARGET)
!if "$(WF_TARGET)" == "psx"
	%setenv LIBRARY_PATH=$(PSX_DIR)/lib
	%setenv PATH $(PSX_DIR)\bin;$(PATH)
	@$(CC) -c @<<
		$(CCCARGS) $(INCLUDE)
	<< -o$(.TARGET) $(.SOURCE)
!else
	$(CC) -c $(INCLUDE) -Fo$(.TARGET) $(CCCARGS) { $(.SOURCE) }
!endif


.cc{$(LIB_OBJ_DIR)}$(OBJ) :
	@echo -------------------------------------------------------------------------------
	@echo      $(.SOURCE) =] $(.TARGET)
!if "$(WF_TARGET)" == "psx"
	%setenv LIBRARY_PATH=$(PSX_DIR)/lib
	%setenv PATH $(PSX_DIR)\bin;$(PATH)
	$(CCC) -v -c @<<
		-D__cplusplus $(CCCARGS) $(INCLUDE) -o$(.TARGET)
	<<  $(.SOURCE)
!if "$(DMPSX)" == "yes"
	copy $(.TARGET) temp$(OBJ)
	$(PSX_DIR)\bin\dmpsx -b temp$(OBJ)
	copy temp$(OBJ) $(.TARGET)
!endif
!else
	$(CCC) -c $(CCCARGS) $(INCLUDE) -Fo$(.TARGET) { $(.SOURCE) }
!endif

#	$(PSX_DIR)\bin\dmpsx -b $(.TARGET)

## Rules (for HAL only)
!if "$(WF_TARGET)" == "psx"
.asm{$(LIB_OBJ_DIR)}$(OBJ) :
	$(PSX_DIR)\bin\asmpsx /o c+ /l /g $<, $@
!endif

################################################################################################################################
## Clean

clean :
	-rd /s $(LIB_OBJ_DIR) >&nul
	-del $(PIGS_LIB) >&nul

clean-lib :
	-del /y $(PIGS_LIB) >&nul

################################################################################################################################
# print: print all switch settings

print .PHONY :
	%echo +----------------------------------------------------------------------------+
	%echo + Platform ------------------------------------------------------------------+
	%echo WF_TARGET        = $(WF_TARGET)
!if "$(WF_TARGET)" == "win"
	%echo +----------------------------------------------------------------------------+
	%echo + Windows -------------------------------------------------------------------+
	%echo SUBSYSTEM        = $(SUBSYSTEM)
	%echo MULTITHREADED  = $(MULTITHREADED)
!endif
!if "$(WF_TARGET)" == "psx"
	%echo +----------------------------------------------------------------------------+
	%echo + Sony Playstation ----------------------------------------------------------+
	%echo PSX_DIR                           = $(PSX_DIR)
!endif
	%echo +----------------------------------------------------------------------------+
	%echo + Debugging -----------------------------------------------------------------+
	%echo MEMORY_TRACKING        = $(MEMORY_TRACKING)
	%echo +----------------------------------------------------------------------------+
	%echo + Switches ------------------------------------------------------------------+
	%echo BUILDMODE               = $(BUILDMODE)
	%echo SCALAR_TYPE             = $(SCALAR_TYPE)
	%echo BUILD_DO_DEBUGGING_INFO = $(BUILD_DO_DEBUGGING_INFO)
	%echo BUILD_DO_ASSERTIONS     = $(BUILD_DO_ASSERTIONS)
	%echo BUILD_DO_VALIDATION     = $(BUILD_DO_VALIDATION)
	%echo BUILD_DO_DEBUG_VARIABLES= $(BUILD_DO_DEBUG_VARIABLES)
	%echo BUILD_DO_TEST_CODE      = $(BUILD_DO_TEST_CODE)
	%echo BUILD_DO_OPTIMIZATIONS  = $(BUILD_DO_OPTIMIZATIONS)
	%echo BUILD_DO_IOSTREAMS      = $(BUILD_DO_IOSTREAMS)
	%echo BUILD_DBSTREAM          = $(BUILD_DBSTREAM)
	%echo RENDERER                = $(RENDERER)
	%echo RENDERER_PIPELINE       = $(RENDERER_PIPELINE)
	%echo RENDERER_DIR            = $(RENDERER_DIR)
	%echo RENDERER_PIPELINE_DIR   = $(RENDERER_PIPELINE_DIR)
	%echo
	%echo +----------------------------------------------------------------------------+
	%echo + Library -------------------------------------------------------------------+
	%echo LIB_NAME       = $(LIB_NAME)
	%echo PIGS_OBJ_DIR  = $(PIGS_OBJ_DIR)
	%echo PIGS_LIB_DIR  = $(PIGS_LIB_DIR)
	%echo PIGS_LIB            = $(PIGS_LIB)
	%echo LIB_OBJS       = [$(LIB_OBJS)]
	%echo TEST_PROG      = $(TEST_PROG)
	%echo +----------------------------------------------------------------------------+
	%echo + Compilation ---------------------------------------------------------------+
	%echo CCCARGS         = $(CCCARGS)
	%echo INCLUDE        = $(INCLUDE)
#       %echo CCCARGS_$(BUILDMODE,UC) = $(CCCARGS_$(BUILDMODE,UC))
	%echo PIGSYS = $(PIGSYS)

################################################################################################################################

print_short .PHONY :
	%echo ===============================================================================
	%echo WF_TARGET = $(WF_TARGET)
	%echo BUILDMODE = $(BUILDMODE)

################################################################################################################################

dir_obj .PHONY :
	%echo $(PIGS_LIB_DIR)
	%echo ===========================================
	@dir /K /H $(PIGS_LIB_DIR)*.*

################################################################################################################################

lib .PHONY :
	%echo Modules in $(PIGS_LIB)
	%echo ======================================================
	@lib /nologo /list $(PIGS_LIB)

################################################################################################################################
