#=============================================================================
# Makefile for game
#=============================================================================
# 'make all'		: Builds the memory library.
# 'make test'		: Builds a test application which exercises the memory library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = mailbox

include ../../GNUMakefile.env

TEST_PROG = mailboxgtest
# Move the TEST_LIBS from the bottom to here to be consistant with other make files.
TEST_LIBS +=   $(HAL)  $(CPPLIB) $(MATH) $(MEMORY) $(STREAMS) $(GFX) $(ANIM) $(PHYSICS) $(IFF) $(AUDIO) $(PARTICLE) $(TIMER)

OVERRIDE-ALL: $(OAD_DIR)/../mailbox.def TOP-ALL

#==============================================================================

$(OAD_DIR)/../mailbox.def : mailbox.des mailbox.inc
	../../bin/prep mailbox.des $(OAD_DIR)/../mailbox.def

################################################################################
# Ahug... if this does not exist, it breaks the 'clean' target!
include $(PIGS_DIR)/oas/objects.ms


LIB_OBJS += \
	$(LIB_OBJ_DIR)mailbox$(OBJ) \

################################################################################

include ../../GNUMakefile.rul

################################################################################
