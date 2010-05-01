#=============================================================================
# Makefile for math library
#=============================================================================
# 'make all'		: Builds the math library.
# 'make test'		: Builds a test application which exercises the math library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = math
TEST_PROG = mathtest
!include ../../Makefile.env
TEST_LIBS += $(CPPLIB) $(STREAMS) $(HAL) $(MEMORY)

!if "$(WF_TARGET)" == "win"
CCCARGS += -Zm500
!endif

.PATH.cc+=;.\$(WF_TARGET)\msvc

################################################################################

LIB_OBJS += \
	$(LIB_OBJ_DIR)angle$(OBJ) \
	$(LIB_OBJ_DIR)scalar$(OBJ) \
	$(LIB_OBJ_DIR)scalarps$(OBJ) \
	$(LIB_OBJ_DIR)vector2$(OBJ) \
	$(LIB_OBJ_DIR)vector3$(OBJ) \
	$(LIB_OBJ_DIR)vector3p$(OBJ) \
	$(LIB_OBJ_DIR)matrix34$(OBJ) \
	$(LIB_OBJ_DIR)euler$(OBJ) \
	$(LIB_OBJ_DIR)plane$(OBJ) \

!if "$(RENDERER_PIPELINE)" != "software"
LIB_OBJS += $(LIB_OBJ_DIR)matrixps$(OBJ)
!endif

!if "$(WF_TARGET)" != "psx"
LIB_OBJS +=
!endif

################################################################################

!include ../Makefile.lib

################################################################################
