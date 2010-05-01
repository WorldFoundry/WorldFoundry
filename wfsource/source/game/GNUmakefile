#=============================================================================
# Makefile for game
#=============================================================================
# 'make all'		: Builds the memory library.
# 'make test'		: Builds a test application which exercises the memory library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = game

include ../../GNUMakefile.env
#kts temp kludge
INCLUDE += -I$(PIGS_DIR)/game

TEST_PROG = game
TEST_LIBS += $(HAL) $(CPPLIB) $(MATH) $(MEMORY) $(STREAMS) $(GFX) $(ANIM) $(PHYSICS) $(MOVEMENT) $(IFF) $(AUDIO) $(PARTICLE) $(TIMER) $(SCRIPTING) $(INPUT) $(ASSET) $(ROOM) $(BASEOBJECT) $(MAILBOX) $(RENDERASSETS)

CCCARGS	+= -D__GAME__=\"WorldFoundryEngine\"

SYS_LIBS += $(TCL_LIB)
SYS_LIBS += $(PERL_LIB)
SYS_LIBS += $(GL_LIB)
ifeq ($(PHYSICS_ENGINE),ode)
SYS_LIBS += $(ODE_LIB)
endif

################################################################################

OVERRIDE-ALL: TOP-ALL

################################################################################
# Ahug... if this does not exist, it breaks the 'clean' target!
include $(PIGS_DIR)/oas/objects.ms

LIB_OBJS += \
	$(LIB_OBJ_DIR)callbacks$(OBJ) \
	$(LIB_OBJ_DIR)actor$(OBJ) \
	$(LIB_OBJ_DIR)main$(OBJ) \
	$(LIB_OBJ_DIR)game$(OBJ) \
	$(LIB_OBJ_DIR)level$(OBJ) \
	$(LIB_OBJ_DIR)toolngun$(OBJ) \
	$(LIB_OBJ_DIR)toolshld$(OBJ) \
	$(LIB_OBJ_DIR)movecam$(OBJ) \
	$(LIB_OBJ_DIR)mailbox$(OBJ) \
	$(OADOBJS) \
	$(LIB_OBJ_DIR)version$(OBJ) \



ifneq ($(BUILD_DBSTREAM),0)
LIB_OBJS += $(LIB_OBJ_DIR)gamestrm$(OBJ)
endif

################################################################################

#include ../GNUMakefile.lib
include ../../GNUMakefile.rul

################################################################################
