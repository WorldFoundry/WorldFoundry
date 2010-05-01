################################################################################
# makefile for hal library (the other lowest-level PIGS interface: hal.h)
################################################################################
# 'make all'		: Builds the hal library
# 'make test'		: Builds a test application which exercises the library
# 'make clean'		: Deletes all of the results of this Makefile
################################################################################

.MS_NMAKE

LIB_NAME = hal
!include ../../Makefile.env

CCCARGS += -DHAL_MAX_MESSAGES=300

!if "$(BUILD_DO_TEST_CODE)" == "1"
CCCARGS   += -DTEST
TEST_LIBS += $(MEMORY)
!endif

!if "$(MULTITHREADED)"=="m"
!error cannot build hal multi-threaded
!endif

!if "$(BUILD_STREAMING)"=="cd"
CCCARGS += -DDO_CD_STREAMING
!endif

################################################################################
## TODO: compile dfhd$(OBJ), dfcd$(OBJ), dfoff$(OBJ), but don't add to hal.lib

#   	$(LIB_OBJ_DIR)time$(OBJ)	\

LIB_OBJS += \
	$(LIB_OBJ_DIR)message$(OBJ)   \
	$(LIB_OBJ_DIR)_list$(OBJ) 	\
	$(LIB_OBJ_DIR)general$(OBJ) 	\
	$(LIB_OBJ_DIR)sjoystic$(OBJ) 	\
	$(LIB_OBJ_DIR)hal$(OBJ)       \
	$(LIB_OBJ_DIR)mempool$(OBJ) 	\
   	$(LIB_OBJ_DIR)platform$(OBJ)	\
   	$(LIB_OBJ_DIR)salloc$(OBJ)	\
   	$(LIB_OBJ_DIR)halconv$(OBJ)	\
	$(LIB_OBJ_DIR)diskfile$(OBJ) \
	$(LIB_OBJ_DIR)df$(BUILD_STREAMING,LC)$(OBJ) \


!if "$(BUILD_DO_MULTITASKING)" == "1"
LIB_OBJS += \
	$(LIB_OBJ_DIR)tasker$(OBJ) 	\
	$(LIB_OBJ_DIR)_signal$(OBJ) 	\
   	$(LIB_OBJ_DIR)timer$(OBJ)     \
        $(LIB_OBJ_DIR)_procsta$(OBJ)
!endif

!if "$(BUILD_DO_VALIDATION)" == "1"
LIB_OBJS += \
        $(LIB_OBJ_DIR)item$(OBJ)
!endif

!if "$(WF_TARGET)" == "win"
LIB_OBJS += \
	$(LIB_OBJ_DIR)taskwin$(OBJ) 	\
	$(LIB_OBJ_DIR)input$(OBJ)
!endif

!if "$(WF_TARGET)" == "psx"
LIB_OBJS += \
	$(LIB_OBJ_DIR)psxjoy$(OBJ) \
	$(LIB_OBJ_DIR)taskpsx$(OBJ) \
	$(LIB_OBJ_DIR)callback$(OBJ) \
	$(LIB_OBJ_DIR)except$(OBJ) \
	$(LIB_OBJ_DIR)vsex$(OBJ) \
	$(PSX_DIR)\lib\mmgm$(OBJ)
!endif

################################################################################

!include ../Makefile.lib

################################################################################

# specific rule for callback to prevent optimizations

!if "$(WF_TARGET)" == "psx"
$(LIB_OBJ_DIR)callback$(OBJ) : psx\callback.cc
	@echo -------------------------------------------------------------------------------
	@echo      $(.SOURCE) =] $(.TARGET)
	@echo -------------------------------------------------------------------------------
	%setenv LIBRARY_PATH=$(PSX_DIR)/lib
	%setenv PATH $(PSX_DIR)\bin;$(PATH)
	%echo $(CCCARGS) $(INCLUDE,S/\;/-I/) -O0 >cccargs.occ
	$(CCC) -c @cccargs.occ -o $(.TARGET) $(.SOURCE)
!endif
