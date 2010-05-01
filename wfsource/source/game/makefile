#=============================================================================
# Makefile for game
#=============================================================================
# 'make all'		: Builds the memory library.
# 'make test'		: Builds a test application which exercises the memory library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = game

OVERRIDE-ALL :  TOP-ALL

!include ../../Makefile.env
#kts temp kludge
INCLUDE += -I$(PIGS_DIR)/game

TEST_PROG = game
CCCARGS	+= -D__GAME__=\"WorldFoundryEngine\"
CCCARGS += /Zm200

CCCARGS += -DTCL=1 -DPERL=0 -DSCHEME=1

################################################################################

!include $(PIGS_DIR)/oas/objects.ms

LIB_OBJS += \
	$(LIB_OBJ_DIR)callbacks$(OBJ) \
	$(LIB_OBJ_DIR)actor$(OBJ) \
	$(LIB_OBJ_DIR)main$(OBJ) \
	$(LIB_OBJ_DIR)level$(OBJ) \
	$(LIB_OBJ_DIR)toolngun$(OBJ) \
	$(LIB_OBJ_DIR)toolshld$(OBJ) \
	$(LIB_OBJ_DIR)movecam$(OBJ) \
	$(LIB_OBJ_DIR)mailbox$(OBJ) \
	\
	$(OADOBJS) \
	\
	$(LIB_OBJ_DIR)version$(OBJ) \



!if "$(BUILD_DBSTREAM)" != "0"
LIB_OBJS += $(LIB_OBJ_DIR)gamestrm$(OBJ)
!endif

################################################################################

TEST_LIBS +=   $(HAL)  $(CPPLIB) $(PIGSYS) $(MATH) $(MEMORY) $(STREAMS) $(GFX) $(ANIM) $(PHYSICS) $(MOVEMENT) $(IFF) $(AUDIO) $(PARTICLE) $(TIMER) $(SCRIPTING) $(INPUT) $(ASSET) $(ROOM) $(BASEOBJECT) $(MAILBOX) $(RENDERASSETS) $(TCL_LIB)
include ../Makefile.lib

################################################################################
