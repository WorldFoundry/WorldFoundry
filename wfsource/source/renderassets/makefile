#=============================================================================
# Makefile for memory library
#=============================================================================
# 'make all'		: Builds the memory library.
# 'make test'		: Builds a test application which exercises the memory library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = renderassets
!include ../../Makefile.env

TEST_PROG = renderassets
TEST_LIBS += $(PIGSYS) $(CPPLIB) $(HAL) $(GFX) $(ASSET)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)rendacto$(OBJ) \
	$(LIB_OBJ_DIR)rendcrow$(OBJ) \
	

################################################################################

!include ../Makefile.lib

################################################################################
