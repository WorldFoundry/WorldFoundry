
DIRS = eval colour handle max2iff max2lev max2lvl select slope attrib toolbar 
#terrain
#oad2txt
#txt2oad

ALL :
	%foreach dir $(DIRS)
		@%chdir $(dir)
		@-mkdir max1.obj max2.obj >& nul
		@echo Building $(dir)
		@echo ================================================================================
		@if exist $(3DSMAX2_DIR)\maxsdk\include\max.h make MAX_VERSION=2
		@if exist $(3DSMAX1_DIR)\maxsdk\include\max.h make MAX_VERSION=1
		@%chdir $(CWD)
	%end


clean :
	%foreach dir $(DIRS)
		@%chdir $(dir)
		@echo Building $(dir)
		@echo ================================================================================
		@make clean
		@%chdir $(CWD)
	%end


realclean :
	%foreach dir $(DIRS)
		@%chdir $(dir)
		@echo Building $(dir)
		@echo ================================================================================
		@make realclean
		@%chdir $(CWD)
	%end
