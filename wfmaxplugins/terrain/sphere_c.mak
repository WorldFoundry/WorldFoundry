# Microsoft Developer Studio Generated NMAKE File, Based on Sphere_c.dsp
!IF "$(CFG)" == ""
CFG=Sphere_c - Win32 Hybrid
!MESSAGE No configuration specified. Defaulting to Sphere_c - Win32 Hybrid.
!ENDIF 

!IF "$(CFG)" != "Sphere_c - Win32 Release" && "$(CFG)" !=\
 "Sphere_c - Win32 Debug" && "$(CFG)" != "Sphere_c - Win32 Hybrid"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sphere_c.mak" CFG="Sphere_c - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sphere_c - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Sphere_c - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Sphere_c - Win32 Hybrid" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sphere_c - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\..\maxsdk\plugin\sphere_c.dlo"

!ELSE 

ALL : "..\..\..\..\maxsdk\plugin\sphere_c.dlo"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Sphere_c.obj"
	-@erase "$(INTDIR)\Sphere_c.res"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\Sphere_c.exp"
	-@erase "$(OUTDIR)\Sphere_c.lib"
	-@erase "..\..\..\..\maxsdk\plugin\sphere_c.dlo"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G6 /MT /W3 /GX- /O2 /I "\maxsdk\include" /D "WIN32" /D\
 "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\Sphere_c.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sphere_c.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sphere_c.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=COMCTL32.LIB kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)\sphere_c.pdb" /machine:I386 /def:".\Sphere_c.def"\
 /out:"\maxsdk\plugin\sphere_c.dlo" /implib:"$(OUTDIR)\sphere_c.lib" 
DEF_FILE= \
	".\Sphere_c.def"
LINK32_OBJS= \
	"$(INTDIR)\Sphere_c.obj" \
	"$(INTDIR)\Sphere_c.res" \
	"..\..\..\..\MAXSDK\Lib\core.lib" \
	"..\..\..\..\MAXSDK\Lib\edmodel.lib" \
	"..\..\..\..\MAXSDK\Lib\geom.lib" \
	"..\..\..\..\MAXSDK\Lib\gfx.lib" \
	"..\..\..\..\MAXSDK\Lib\mesh.lib" \
	"..\..\..\..\MAXSDK\Lib\patch.lib" \
	"..\..\..\..\MAXSDK\Lib\util.lib"

"..\..\..\..\maxsdk\plugin\sphere_c.dlo" : "$(OUTDIR)" $(DEF_FILE)\
 $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Sphere_c - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\..\maxsdk\plugin\sphere_c.dlo"

!ELSE 

ALL : "..\..\..\..\maxsdk\plugin\sphere_c.dlo"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Sphere_c.obj"
	-@erase "$(INTDIR)\Sphere_c.res"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\Sphere_c.exp"
	-@erase "$(OUTDIR)\Sphere_c.lib"
	-@erase "$(OUTDIR)\Sphere_c.pdb"
	-@erase "..\..\..\..\maxsdk\plugin\sphere_c.dlo"
	-@erase "..\..\..\..\maxsdk\plugin\sphere_c.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G6 /MDd /W3 /Gm /GX- /Zi /Od /I "\maxsdk\include" /D "WIN32"\
 /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\Sphere_c.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sphere_c.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sphere_c.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=COMCTL32.LIB kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)\sphere_c.pdb" /debug /machine:I386 /def:".\Sphere_c.def"\
 /out:"\maxsdk\plugin\sphere_c.dlo" /implib:"$(OUTDIR)\sphere_c.lib"\
 /pdbtype:sept 
DEF_FILE= \
	".\Sphere_c.def"
LINK32_OBJS= \
	"$(INTDIR)\Sphere_c.obj" \
	"$(INTDIR)\Sphere_c.res" \
	"..\..\..\..\MAXSDK\Lib\core.lib" \
	"..\..\..\..\MAXSDK\Lib\edmodel.lib" \
	"..\..\..\..\MAXSDK\Lib\geom.lib" \
	"..\..\..\..\MAXSDK\Lib\gfx.lib" \
	"..\..\..\..\MAXSDK\Lib\mesh.lib" \
	"..\..\..\..\MAXSDK\Lib\patch.lib" \
	"..\..\..\..\MAXSDK\Lib\util.lib"

"..\..\..\..\maxsdk\plugin\sphere_c.dlo" : "$(OUTDIR)" $(DEF_FILE)\
 $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Sphere_c - Win32 Hybrid"

OUTDIR=.\Hybrid
INTDIR=.\Hybrid

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\..\maxsdk\plugin\sphere_c.dlo"

!ELSE 

ALL : "..\..\..\..\maxsdk\plugin\sphere_c.dlo"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Sphere_c.obj"
	-@erase "$(INTDIR)\Sphere_c.res"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\sphere_c.exp"
	-@erase "$(OUTDIR)\sphere_c.lib"
	-@erase "$(OUTDIR)\sphere_c.pdb"
	-@erase "..\..\..\..\maxsdk\plugin\sphere_c.dlo"
	-@erase "..\..\..\..\maxsdk\plugin\sphere_c.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G6 /MD /W3 /Gm /GX- /Zi /Od /I "\maxsdk\include" /D "WIN32"\
 /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\Sphere_c.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Hybrid/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sphere_c.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sphere_c.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=COMCTL32.LIB kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)\sphere_c.pdb" /debug /machine:I386 /def:".\Sphere_c.def"\
 /out:"\maxsdk\plugin\sphere_c.dlo" /implib:"$(OUTDIR)\sphere_c.lib"\
 /pdbtype:sept 
DEF_FILE= \
	".\Sphere_c.def"
LINK32_OBJS= \
	"$(INTDIR)\Sphere_c.obj" \
	"$(INTDIR)\Sphere_c.res" \
	"..\..\..\..\MAXSDK\Lib\core.lib" \
	"..\..\..\..\MAXSDK\Lib\edmodel.lib" \
	"..\..\..\..\MAXSDK\Lib\geom.lib" \
	"..\..\..\..\MAXSDK\Lib\gfx.lib" \
	"..\..\..\..\MAXSDK\Lib\mesh.lib" \
	"..\..\..\..\MAXSDK\Lib\patch.lib" \
	"..\..\..\..\MAXSDK\Lib\util.lib"

"..\..\..\..\maxsdk\plugin\sphere_c.dlo" : "$(OUTDIR)" $(DEF_FILE)\
 $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(CFG)" == "Sphere_c - Win32 Release" || "$(CFG)" ==\
 "Sphere_c - Win32 Debug" || "$(CFG)" == "Sphere_c - Win32 Hybrid"
SOURCE=.\Sphere_c.cpp
DEP_CPP_SPHER=\
	"..\..\..\..\maxsdk\include\acolor.h"\
	"..\..\..\..\maxsdk\include\animtbl.h"\
	"..\..\..\..\maxsdk\include\appio.h"\
	"..\..\..\..\maxsdk\include\assert1.h"\
	"..\..\..\..\maxsdk\include\bitarray.h"\
	"..\..\..\..\maxsdk\include\box2.h"\
	"..\..\..\..\maxsdk\include\box3.h"\
	"..\..\..\..\maxsdk\include\buildver.h"\
	"..\..\..\..\maxsdk\include\channels.h"\
	"..\..\..\..\maxsdk\include\cmdmode.h"\
	"..\..\..\..\maxsdk\include\color.h"\
	"..\..\..\..\maxsdk\include\control.h"\
	"..\..\..\..\maxsdk\include\coreexp.h"\
	"..\..\..\..\maxsdk\include\custcont.h"\
	"..\..\..\..\maxsdk\include\dbgprint.h"\
	"..\..\..\..\maxsdk\include\dpoint3.h"\
	"..\..\..\..\maxsdk\include\euler.h"\
	"..\..\..\..\maxsdk\include\evuser.h"\
	"..\..\..\..\maxsdk\include\export.h"\
	"..\..\..\..\maxsdk\include\gencam.h"\
	"..\..\..\..\maxsdk\include\genhier.h"\
	"..\..\..\..\maxsdk\include\genlight.h"\
	"..\..\..\..\maxsdk\include\genshape.h"\
	"..\..\..\..\maxsdk\include\geom.h"\
	"..\..\..\..\maxsdk\include\geomlib.h"\
	"..\..\..\..\maxsdk\include\gfloat.h"\
	"..\..\..\..\maxsdk\include\gfx.h"\
	"..\..\..\..\maxsdk\include\gfxlib.h"\
	"..\..\..\..\maxsdk\include\gutil.h"\
	"..\..\..\..\maxsdk\include\hitdata.h"\
	"..\..\..\..\maxsdk\include\hold.h"\
	"..\..\..\..\maxsdk\include\impapi.h"\
	"..\..\..\..\maxsdk\include\impexp.h"\
	"..\..\..\..\maxsdk\include\imtl.h"\
	"..\..\..\..\maxsdk\include\inode.h"\
	"..\..\..\..\maxsdk\include\interval.h"\
	"..\..\..\..\maxsdk\include\ioapi.h"\
	"..\..\..\..\maxsdk\include\iparamb.h"\
	"..\..\..\..\maxsdk\include\iparamm.h"\
	"..\..\..\..\maxsdk\include\ipoint2.h"\
	"..\..\..\..\maxsdk\include\ipoint3.h"\
	"..\..\..\..\maxsdk\include\lockid.h"\
	"..\..\..\..\maxsdk\include\log.h"\
	"..\..\..\..\maxsdk\include\matrix2.h"\
	"..\..\..\..\maxsdk\include\matrix3.h"\
	"..\..\..\..\maxsdk\include\max.h"\
	"..\..\..\..\maxsdk\include\maxapi.h"\
	"..\..\..\..\maxsdk\include\maxcom.h"\
	"..\..\..\..\maxsdk\include\maxtess.h"\
	"..\..\..\..\maxsdk\include\maxtypes.h"\
	"..\..\..\..\maxsdk\include\mesh.h"\
	"..\..\..\..\maxsdk\include\meshlib.h"\
	"..\..\..\..\maxsdk\include\mouseman.h"\
	"..\..\..\..\maxsdk\include\mtl.h"\
	"..\..\..\..\maxsdk\include\nametab.h"\
	"..\..\..\..\maxsdk\include\object.h"\
	"..\..\..\..\maxsdk\include\objmode.h"\
	"..\..\..\..\maxsdk\include\partclib.h"\
	"..\..\..\..\maxsdk\include\particle.h"\
	"..\..\..\..\maxsdk\include\patch.h"\
	"..\..\..\..\maxsdk\include\patchlib.h"\
	"..\..\..\..\maxsdk\include\patchobj.h"\
	"..\..\..\..\maxsdk\include\plugapi.h"\
	"..\..\..\..\maxsdk\include\plugin.h"\
	"..\..\..\..\maxsdk\include\point2.h"\
	"..\..\..\..\maxsdk\include\point3.h"\
	"..\..\..\..\maxsdk\include\point4.h"\
	"..\..\..\..\maxsdk\include\ptrvec.h"\
	"..\..\..\..\maxsdk\include\quat.h"\
	"..\..\..\..\maxsdk\include\ref.h"\
	"..\..\..\..\maxsdk\include\render.h"\
	"..\..\..\..\maxsdk\include\rtclick.h"\
	"..\..\..\..\maxsdk\include\sceneapi.h"\
	"..\..\..\..\maxsdk\include\simpobj.h"\
	"..\..\..\..\maxsdk\include\snap.h"\
	"..\..\..\..\maxsdk\include\soundobj.h"\
	"..\..\..\..\maxsdk\include\stack.h"\
	"..\..\..\..\maxsdk\include\stack3.h"\
	"..\..\..\..\maxsdk\include\strbasic.h"\
	"..\..\..\..\maxsdk\include\strclass.h"\
	"..\..\..\..\maxsdk\include\surf_api.h"\
	"..\..\..\..\maxsdk\include\tab.h"\
	"..\..\..\..\maxsdk\include\trig.h"\
	"..\..\..\..\maxsdk\include\triobj.h"\
	"..\..\..\..\maxsdk\include\units.h"\
	"..\..\..\..\maxsdk\include\utilexp.h"\
	"..\..\..\..\maxsdk\include\utillib.h"\
	"..\..\..\..\maxsdk\include\vedge.h"\
	"..\..\..\..\maxsdk\include\winutil.h"\
	

"$(INTDIR)\Sphere_c.obj" : $(SOURCE) $(DEP_CPP_SPHER) "$(INTDIR)"


SOURCE=.\Sphere_c.rc

"$(INTDIR)\Sphere_c.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

