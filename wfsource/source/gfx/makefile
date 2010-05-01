################################################################################
# Makefile for gfx library
################################################################################
# 'make all'		: Builds the gfx library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################


%if $(OS) == OS2 || $(OS) == NT
; = &
%else
; = ;
%endif


.MS_NMAKE

LIB_NAME = gfx

#?DMPSX=yes

REALTOP:  PIPELINE_MAKEFILE_INC PIPELINE_MAKEFILE TOP-ALL

!include ../../Makefile.env



PIPELINE_MAKEFILE_INC : $(RENDERER_PIPELINE_DIR)/Makefile.inc

PIPELINE_MAKEFILE: 
        ( cd $(RENDERER_PIPELINE_DIR) $; make /nologo $(MFLAGS) )

!include $(RENDERER_PIPELINE_DIR)/Makefile.inc

LIB_OBJS += \
	$(LIB_OBJ_DIR)callbacks.obj \
	$(LIB_OBJ_DIR)display.obj \
	$(LIB_OBJ_DIR)viewport.obj \
	$(LIB_OBJ_DIR)vmem.obj \
	$(LIB_OBJ_DIR)rendobj2.obj \
	$(LIB_OBJ_DIR)rendobj3.obj \
	$(LIB_OBJ_DIR)color.obj \
	$(LIB_OBJ_DIR)texture.obj \
	$(LIB_OBJ_DIR)pixelmap.obj \
	$(LIB_OBJ_DIR)material.obj \
	$(LIB_OBJ_DIR)camera.obj \
	$(LIB_OBJ_DIR)handle.obj \
	$(LIB_OBJ_DIR)rmuv.obj \
	$(LIB_OBJ_DIR)math.obj \
	$(LIB_OBJ_DIR)ccyc.obj \
	$(LIB_OBJ_DIR)ccycle.obj \
	$(LIB_OBJ_DIR)rendmatt.obj \


# kts make an include in each pipeline dir which adds to LIB_OBJS
# same for RENDERER_DIR

!if "$(RENDERER_PIPELINE)" == "software" || "$(RENDERER_PIPELINE)" == "psx"
LIB_OBJS +=	$(LIB_OBJ_DIR)otable.obj
!endif

!if "$(RENDERER)" == "directx"

LIB_OBJS +=	$(LIB_OBJ_DIR)scene.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)d3dtex.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)d3dutils.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)d3dwin.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)debug.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)drvmgr.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)flock.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)winmain.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)winproc.obj

#LIB_OBJS +=	$(LIB_OBJ_DIR)directx.obj
LIB_OBJS +=	$(LIB_OBJ_DIR)getdxver.obj
!endif

# if you want to link this into the game
#	$(LIB_OBJ_DIR)gfxtest.obj \

#.PATH.cc+=;.\$(RENDERER_PIPELINE_DIR);$(RENDERER_DIR);

#!if "$(BUILD_DO_DEBUG_VARIABLES)" == "1"
LIB_OBJS += $(LIB_OBJ_DIR)util.obj
#!endif
!if "$(BUILD_DO_TEST_CODE)" == "1"
CCCARGS  += -DTEST
!endif

TEST_LIBS += $(HAL) $(MEMORY) $(CPPLIB) $(IFF) $(MATH) $(STREAMS)
#TEST_LIBS += $(PROFILE)

################################################################################

!include ../Makefile.lib

################################################################################

## Rules (for gfx only)

$(RENDERER_PIPELINE_DIR)/Makefile.inc : $(RENDERER_PIPELINE_DIR)/makefile.ins
        ( cd $(RENDERER_PIPELINE_DIR) $; make /nologo $(MFLAGS) )
	%restart

$(LIB_OBJ_DIR)display.obj: display.cc $(RENDERER_DIR)/display.cc  $(RENDERER_PIPELINE_DIR)/renderer.inc

$(LIB_OBJ_DIR)viewport.obj: viewport.cc $(RENDERER_DIR)/viewport.cc

$(LIB_OBJ_DIR)rendobj3.obj: rendobj3.cc handle.cc $(RENDERER_PIPELINE_DIR)/rendobj3.cc

$(LIB_OBJ_DIR)rendobj2.obj: rendobj2.cc $(RENDERER_PIPELINE_DIR)/renderer.inc

#!if "$(WF_TARGET)" == "psx"
#.asm.obj :
#	asmpsx /o c+ /l /g $<, $@
#!endif

################################################################################

#software/renderer.inc: software/renderer.ins
#	cd software
#	call m
#	cd ..
#
#psx/renderer.inc: psx/renderer.ins
#	cd psx
#	call m
#	cd ..
