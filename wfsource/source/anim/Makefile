################################################################################
# makefile for anim library
################################################################################
# 'make all'		: Builds the anim library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = anim
!include ../../Makefile.env
TEST_PROG = preview

!if "$(BUILD_DO_TEST_CODE)" == "1"
CCCARGS   += -DTEST
!endif

TEST_LIBS += $(HAL) $(GFX)

################################################################################

LIB_OBJS += \
   	$(LIB_OBJ_DIR)path$(OBJ)     \
   	$(LIB_OBJ_DIR)anim$(OBJ)     \
   	$(LIB_OBJ_DIR)animcycl$(OBJ)     \
	$(LIB_OBJ_DIR)channel$(OBJ)	\
	$(LIB_OBJ_DIR)animmang$(OBJ)	\


################################################################################

!include ../Makefile.lib

################################################################################
