#=============================================================================
# Makefile for profile library
#=============================================================================
# 'make all'		: Builds the profile library.
# 'make test'		: Builds a test application which exercises the profile library.
# 'make clean'		: Deletes all of the results of this makefile.
#=============================================================================

.MS_NMAKE

LIB_NAME = profile
TEST_PROG = proftest
TEST_LIBS += $(LIB_OBJ_DIR)profile.lib $(PIGS_OBJ_DIR)\hal\hal.lib

!include ../../Makefile.env

################################################################################

!if "$(WF_TARGET)" == "psx"
LIB_OBJS += $(LIB_OBJ_DIR)sampprof.obj
!endif

################################################################################

!include ../Makefile.inc

################################################################################

# specific rule for profile to prevent optimizations

!if "$(WF_TARGET)" == "psx"
$(LIB_OBJ_DIR)sampprof.obj : sampprof.cc
	@echo -------------------------------------------------------------------------------
	@echo      $(.SOURCE) =] $(.TARGET)
	@echo -------------------------------------------------------------------------------
	%setenv LIBRARY_PATH=$(PSX_DIR)/lib
	%setenv PATH $(PSX_DIR)\bin;$(PATH)
	%echo $(CCCARGS) $(INCLUDE,S/\;/-I/) -O0 >cccargs.occ
	$(CCC) -c @cccargs.occ -o $(.TARGET) $(.SOURCE)
!endif
