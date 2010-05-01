################################################################################
# Makefile for gfx library
################################################################################
# 'make all'		: Builds the gfx library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

LIB_NAME = gfx

#?DMPSX=yes
include ../../GNUMakefile.env

VPATH = $(RENDERER_PIPELINE_DIR):$(RENDERER_DIR)


SYS_LIBS += $(GL_LIB)

LIB_OBJS += \
	$(LIB_OBJ_DIR)callbacks$(OBJ) \
	$(LIB_OBJ_DIR)display$(OBJ) \
	$(LIB_OBJ_DIR)viewport$(OBJ) \
	$(LIB_OBJ_DIR)rendobj2$(OBJ) \
	$(LIB_OBJ_DIR)rendobj3$(OBJ) \
	$(LIB_OBJ_DIR)color$(OBJ) \
	$(LIB_OBJ_DIR)texture$(OBJ) \
	$(LIB_OBJ_DIR)pixelmap$(OBJ) \
	$(LIB_OBJ_DIR)vmem$(OBJ) \
	$(LIB_OBJ_DIR)material$(OBJ) \
	$(LIB_OBJ_DIR)camera$(OBJ) \
	$(LIB_OBJ_DIR)handle$(OBJ) \
	$(LIB_OBJ_DIR)rmuv$(OBJ) \
	$(LIB_OBJ_DIR)math$(OBJ) \
	$(LIB_OBJ_DIR)ccyc$(OBJ) \
	$(LIB_OBJ_DIR)ccycle$(OBJ) \
	$(LIB_OBJ_DIR)rendmatt$(OBJ) \


# kts make an include in each pipeline dir which adds to LIB_OBJS
# same for RENDERER_DIR

ifeq ($(RENDERER_PIPELINE),software)
LIB_OBJS +=	$(LIB_OBJ_DIR)otable$(OBJ)
endif


# kts for now, eventually glpipeline will NOT use order tables
ifeq ($(RENDERER_PIPELINE),gl)
#LIB_OBJS +=	$(LIB_OBJ_DIR)otable$(OBJ)
endif


ifeq ($(RENDERER_PIPELINE),psx)
LIB_OBJS +=	$(LIB_OBJ_DIR)otable$(OBJ)
endif

ifeq ($(RENDERER),directx)
LIB_OBJS +=	$(LIB_OBJ_DIR)scene$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)d3dtex$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)d3dutils$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)d3dwin$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)debug$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)drvmgr$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)flock$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)winmain$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)winproc$(OBJ)

#LIB_OBJS +=	$(LIB_OBJ_DIR)directx$(OBJ)
LIB_OBJS +=	$(LIB_OBJ_DIR)getdxver$(OBJ)
endif

LOCAL-PRECONDITIONS = PIPELINE

# Ah... to do a 'make clean' if this file is not built, it must build it to do
# the 'clean', ... build to clean? If prep is missing it stops one from doing
# a make clean... :( combined with the rule at the bottom to build it );
include $(RENDERER_PIPELINE_DIR)/Makefile.inc
# Ideally, one should not need to 'build' to 'clean'.

# if you want to link this into the game
#	$(LIB_OBJ_DIR)gfxtest$(OBJ) \

#.PATH.cc+=;.\$(RENDERER_PIPELINE_DIR);$(RENDERER_DIR);

#!if "$(BUILD_DO_DEBUG_VARIABLES)" == "1"
LIB_OBJS += $(LIB_OBJ_DIR)util$(OBJ)
#!endif
ifeq ($(BUILD_DO_TEST_CODE),1)
CCCARGS  += -DTEST
endif

TEST_LIBS += $(HAL) $(MEMORY) $(CPPLIB) $(IFF) $(MATH) $(STREAMS)
#TEST_LIBS += $(PROFILE)

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################

.PHONY: PIPELINE

PIPELINE : 
	cd $(RENDERER_PIPELINE_DIR) && $(MAKE) $(MFLAGS)

################################################################################

## Rules (for gfx only)

$(LIB_OBJ_DIR)display$(OBJ): $(RENDERER_DIR)/display.cc

$(LIB_OBJ_DIR)viewport$(OBJ): viewport.cc $(RENDERER_DIR)/viewport.cc

$(LIB_OBJ_DIR)rendobj3$(OBJ): rendobj3.cc handle.cc $(RENDERER_PIPELINE_DIR)/rendobj3.cc

$(LIB_OBJ_DIR)rendobj2$(OBJ): rendobj2.cc $(RENDERER_PIPELINE_DIR)/renderer.inc

#!if "$(WF_TARGET)" == "psx"
#.asm$(OBJ) :
#	asmpsx /o c+ /l /g $<, $@
#!endif

################################################################################
# These had done 'cd dir; make', but it broke targets like 'clean' as it would
# always try to build instead of clean! (Did not pass the 'clean' arg.)

$(RENDERER_PIPELINE_DIR)/Makefile.inc : $(RENDERER_PIPELINE_DIR)/makefile.ins
	$(MAKE) -C $(RENDERER_PIPELINE_DIR)

$(RENDERER_PIPELINE_DIR)/renderer.inc : $(RENDERER_PIPELINE_DIR)/renderer.ins
	$(MAKE) -C $(RENDERER_PIPELINE_DIR)

rendfcl.cc rendgcl.cc rendftl.cc rendgtl.cc rendfcp.cc rendgcp.cc rendftp.cc rendgtp.cc :
	$(MAKE) -C $(RENDERER_PIPELINE_DIR)
