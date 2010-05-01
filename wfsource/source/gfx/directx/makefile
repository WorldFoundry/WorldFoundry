# boids makefile

# =========================================================================
# WIN32.MAK - Win32 application master NMAKE definitions file for the
#               Microsoft Win32 SDK for Windows programming samples

# -------------------------------------------------------------------------
# Get Target Operating System - Default to WINNT
# -------------------------------------------------------------------------

TARGETOS = WINNT
APPVER = 4.0

# binary declarations common to all platforms
cc     = cl
rc     = rc
link   = link
implib = lib
hc     = hcrtf -xn

# declarations common to all compiler options
ccommon = -c -W3 -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl -nologo

cflags = $(ccommon) -D_X86_=1 /TP -Id:\projects\wfsource\pigs2 -D__WIN__ -DSW_DBSTREAM=0 -DDO_ASSERTIONS=1
scall  = -Gz

!IF "$(TARGETOS)" == "WINNT"
cflags = $(cflags) -D_WINNT -D_WIN32_WINNT=0x0400
!ELSE
!IF "$(TARGETOS)" == "WIN95"
cflags = $(cflags) -D_WIN95 -D_WIN32_WINDOWS=0x0400
!ENDIF
!ENDIF

# For this release, for all values of APPVER, WINVER is 0x0400
cflags = $(cflags) -DWINVER=0x0400

!IFDEF NODEBUG
cdebug = -Ox
!ELSE
!IFDEF PROFILE
cdebug = -Gh -Zd -Ox
!ELSE
!IFDEF TUNE
cdebug = -Gh -Zd -Ox
!ELSE
cdebug = -Z7 -Od
!ENDIF
!ENDIF
!ENDIF

# for Windows applications that use the C Run-Time libraries
cvars      = -DWIN32 $(noansi) -D_WIN32
cvarsmt    = $(cvars) -D_MT -MT
cvarsdll   = $(cvars) -D_MT -D_DLL -MD

# resource compiler
rcflags = /r
rcvars = -DWIN32 -D_WIN32 -DWINVER=0x0400 $(noansi)

# declarations common to all linker options
lflags  = /NODEFAULTLIB /INCREMENTAL:NO /PDB:NONE /RELEASE /NOLOGO

# declarations for use on Intel i386, i486, and Pentium systems
DLLENTRY = @12

# -------------------------------------------------------------------------
# Target Module Dependent Link Debug Flags - must be specified after $(link)
#
# These switches allow the inclusion of the necessary symbolic information
# for source level debugging with WinDebug, profiling and/or performance
# tuning.
#
# Note: Debug switches are on by default.
# -------------------------------------------------------------------------

!IFDEF NODEBUG
ldebug = /RELEASE
!ELSE
!IFDEF PROFILE
ldebug = -debug:mapped,partial -debugtype:coff
!ELSE
!IFDEF TUNE
ldebug = -debug:mapped,partial -debugtype:coff
!ELSE
ldebug = -map -debug:full -debugtype:cv
!ENDIF
!ENDIF
!ENDIF

# -------------------------------------------------------------------------
# Subsystem Dependent Link Flags - must be specified after $(link)
#
# These switches allow for source level debugging with WinDebug for local
# and global variables.  They also provide the standard application type and
# entry point declarations.
#
# Note that on x86 screensavers have a WinMain entrypoint, but on RISC
# platforms it is main.  This is a Win95 compatibility issue.
#
# -------------------------------------------------------------------------

# for Windows applications that use the C Run-Time libraries
conlflags = $(lflags) -subsystem:console,$(APPVER)
guilflags = $(lflags) -subsystem:windows,$(APPVER)
dlllflags = $(lflags) -entry:_DllMainCRTStartup$(DLLENTRY) -dll

savlflags = $(lflags) -subsystem:windows,$(APPVER) -entry:WinMainCRTStartup

# for POSIX applications
psxlflags = $(lflags) -subsystem:posix -entry:__PosixProcessStartup

# for compatibility with older-style makefiles
conflags  = $(conlflags)
guiflags  = $(guilflags)
psxflags  = $(psxlflags)

libc = libc.lib oldnames.lib
libcmt = libcmt.lib oldnames.lib
libcdll = msvcrt.lib oldnames.lib

# for POSIX applications
#psxlibs    = libcpsx.lib psxdll.lib psxrtl.lib oldnames.lib


# optional profiling and tuning libraries
!IFDEF PROFILE
optlibs =  cap.lib
!ELSE
!IFDEF TUNE
optlibs = wst.lib
!ELSE
optlibs =
!ENDIF
!ENDIF

# basic subsystem specific libraries, less the C Run-Time
baselibs   = kernel32.lib $(optlibs) advapi32.lib
winlibs    = $(baselibs) user32.lib gdi32.lib comdlg32.lib winspool.lib

guilibs    = $(libc) $(winlibs)

#=============================================================================

!if "$(nodebug)" == "1"
OBJ_DIR = Retail
!else
OBJ_DIR = Debug
!endif

proj=boids
cflags = $(cflags)

OBJ_LIST = $(OBJ_DIR)\$(proj).obj $(OBJ_DIR)\d3dwin.obj \
           $(OBJ_DIR)\drvmgr.obj $(OBJ_DIR)\winproc.obj \
           $(OBJ_DIR)\debug.obj $(OBJ_DIR)\winmain.obj \
           $(OBJ_DIR)\d3dutils.obj $(OBJ_DIR)\getdxver.obj \
           $(OBJ_DIR)\flock.obj $(OBJ_DIR)\d3dtex.obj

mycflag=/I "..\misc"

all: mkdir $(OBJ_DIR)\$(proj).exe

$(OBJ_DIR)\$(proj).obj:  $(proj).cc resource.h
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\$(proj).obj $(mycflag) \
    $(cvars) $(proj).cc

$(OBJ_DIR)\d3dwin.obj:  d3dwin.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\d3dwin.obj $(mycflag) \
    $(cvars) d3dwin.cc

$(OBJ_DIR)\drvmgr.obj:  drvmgr.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\drvmgr.obj $(mycflag) \
    $(cvars) drvmgr.cc

$(OBJ_DIR)\debug.obj:  debug.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\debug.obj $(mycflag) \
    $(cvars) debug.cc

$(OBJ_DIR)\winmain.obj:  winmain.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\winmain.obj $(mycflag) \
    $(cvars) winmain.cc

$(OBJ_DIR)\winproc.obj:  winproc.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\winproc.obj $(mycflag) \
    $(cvars) winproc.cc

$(OBJ_DIR)\d3dutils.obj:  d3dutils.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\d3dutils.obj $(mycflag) \
    $(cvars) d3dutils.cc

$(OBJ_DIR)\getdxver.obj:  getdxver.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\getdxver.obj $(mycflag) \
    $(cvars) getdxver.cc

$(OBJ_DIR)\flock.obj:  flock.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\flock.obj $(mycflag) \
    $(cvars) flock.cc

$(OBJ_DIR)\d3dtex.obj:  d3dtex.cc
    $(cc) $(cdebug) $(cflags) -Fo$(OBJ_DIR)\d3dtex.obj $(mycflag) \
    $(cvars) d3dtex.cc

$(OBJ_DIR)\$(proj).res: boids2.rc resource.h
   rc -r -fo $(OBJ_DIR)\$(proj).res boids2.rc

$(OBJ_DIR)\$(proj).exe: $(OBJ_LIST) $(OBJ_DIR)\$(proj).res
    $(link) $(linkdebug) $(guiflags) -out:$(OBJ_DIR)\$(proj).exe \
           $(OBJ_LIST) $(OBJ_DIR)\$(proj).res \
           $(guilibs) ddraw.lib winmm.lib
mkdir:
    if not exist $(OBJ_DIR)\NUL md $(OBJ_DIR)

# Rules for cleaning out those old files
clean:
    -@echo y | del Retail
    -@echo y | del Debug
    -@rd Retail
    -@rd Debug

