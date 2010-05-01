#=============================================================================
# Makefile for pigsys library (lowest-level PIGS interface: pigsys)
#=============================================================================
# 'make all'		: Builds the pigsys library.
# 'make test'		: Builds a test application which exercises the pigsys library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

#TOP-ALL : LIBRARY_NAMES_DEFINES OBJ_DIR PIGS_LIB

LIB_NAME = pigsys
TEST_PROG = psystest

!include ../../Makefile.env

LIB_OBJS = \
	$(LIB_OBJ_DIR)_cf_$(WF_TARGET)$(OBJ) \
	$(LIB_OBJ_DIR)pigsys$(OBJ) \
	$(LIB_OBJ_DIR)_atexit$(OBJ) \
	$(LIB_OBJ_DIR)new$(OBJ) \
	$(LIB_OBJ_DIR)endian$(OBJ) \

################################################################################

!if "$(WF_TARGET)" == "psx"
#	LIB_OBJS += $(LIB_OBJ_DIR)vsprintf$(OBJ)
!if "$(BUILD_DO_IOSTREAMS)" != "0"
 	LIB_OBJS += $(LIB_OBJ_DIR)iostream$(OBJ)
 	LIB_OBJS += $(LIB_OBJ_DIR)fstream$(OBJ)
!endif
!endif	# psx

#%echo BUILD_DO_ASSERTIONS = $(BUILD_DO_ASSERTIONS)
!if "$(BUILD_DO_ASSERTIONS)" == "1" || "$(BUILD_DO_DEBUG_FILE_SYSTEM)"
#!error got here
LIB_OBJS += $(LIB_OBJ_DIR)assert$(OBJ)
!if "$(WF_TARGET)" == "psx"
LIB_OBJS += $(LIB_OBJ_DIR)strlwr$(OBJ)
LIB_OBJS += $(LIB_OBJ_DIR)scanf$(OBJ)
!endif  # psx
!endif

!include ../Makefile.lib

################################################################################
