#=============================================================================
# GNUmakefile: Makefile for pigsys library (lowest-level PIGS interface: pigsys)
#=============================================================================
# 'make all'		: Builds the pigsys library.
# 'make test'		: Builds a test application which exercises the pigsys library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

LIB_NAME = pigsys
TEST_PROG = psystest

include ../../GNUMakefile.env

LIB_OBJS = \
	$(LIB_OBJ_DIR)_cf_$(WF_TARGET)$(OBJ) \
	$(LIB_OBJ_DIR)pigsys$(OBJ) \
	$(LIB_OBJ_DIR)_atexit$(OBJ) \
	$(LIB_OBJ_DIR)new$(OBJ) \
	$(LIB_OBJ_DIR)endian$(OBJ) \

################################################################################

ifeq ($(WF_TARGET),psx)
#	LIB_OBJS += $(LIB_OBJ_DIR)vsprintf$(OBJ)
ifneq ($(BUILD_DO_IOSTREAMS),0)
 	LIB_OBJS += $(LIB_OBJ_DIR)iostream$(OBJ)
 	LIB_OBJS += $(LIB_OBJ_DIR)fstream$(OBJ)
endif
endif	# psx

#@echo BUILD_DO_ASSERTIONS = $(BUILD_DO_ASSERTIONS)

# kts I don't know how to make gnu make or together 2 conditionals

PIGSYS_TEMP=0
ifeq ($(BUILD_DO_ASSERTIONS),1)
PIGSYS_TEMP=1
endif
ifeq ($(BUILD_DO_DEBUG_FILE_SYSTEM),1)
PIGSYS_TEMP=1
endif

ifeq ($(PIGSYS_TEMP),1)
LIB_OBJS += $(LIB_OBJ_DIR)assert$(OBJ)
ifeq ($(WF_TARGET),psx)
LIB_OBJS += $(LIB_OBJ_DIR)strlwr$(OBJ)
LIB_OBJS += $(LIB_OBJ_DIR)scanf$(OBJ)
endif  # psx
ifeq ($(WF_TARGET),linux)
LIB_OBJS += $(LIB_OBJ_DIR)strlwr$(OBJ)
endif
endif

#include ../GNUMakefile.lib No longer used, merged with .rul
include ../../GNUMakefile.rul

################################################################################
