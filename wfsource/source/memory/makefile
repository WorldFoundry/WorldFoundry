#=============================================================================
# Makefile for memory library
#=============================================================================
# 'make all'		: Builds the memory library.
# 'make test'		: Builds a test application which exercises the memory library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = memory
!include ../../Makefile.env

TEST_PROG = memtest
TEST_LIBS += $(PIGSYS) $(CPPLIB) $(HAL)

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)memory$(OBJ) \
	$(LIB_OBJ_DIR)lmalloc$(OBJ) \
	$(LIB_OBJ_DIR)dmalloc$(OBJ) \
	$(LIB_OBJ_DIR)mempool$(OBJ) \
	
!if "$(BUILDMODE)" == "tool"
LIB_OBJS += $(LIB_OBJ_DIR)realmalloc$(OBJ) 
!endif

################################################################################

!include ../Makefile.lib

################################################################################
