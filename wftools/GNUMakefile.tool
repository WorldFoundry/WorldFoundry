# wftools/GNUMakefile.tool

#ALL : $(WF_DIR)\bin\$(PROGRAM)
#
#$(WF_DIR)\bin\$(PROGRAM) : $(PROGRAM)
#        copy $(.SOURCE) $(.TARGET)
#
#$(PROGRAM) : $(OBJS) $(PIGS_LIBS)
#        bumpver version.cc
#        cl /TP /c version.cc
#        link /debug /LIBPATH:"$(MSVCDir)\lib" /OUT:$(.TARGET) $(.SOURCES) version.obj

ifndef WF_DIR
	WF_DIR:=$(shell pwd | sed -e "s/^\(.*\)wftools.*/\1wfsource/")
	export WF_DIR
endif


ifndef PIGS_DIR
	PIGS_DIR=$(WF_DIR)/source
	export PIGS_DIR
endif

BUILDMODE=tool
LIB_NAME = $(PROGRAM)

include $(WF_DIR)/GNUMakefile.env
include $(WF_DIR)/GNUMakefile.rul
include $(PIGS_DIR)/GNUpigs.dep

#INCLUDE += -I ../recolib -I ..

# For greater readability during the build, I put these 
# directly into the gcc statment.
# This way each one starts a line and the gcc command is MUCH more readable!
# set my own veriable to get what is so far defined for use below.
# But keep these so we don't break something else.
PIGS_LIBS += $(PIGSYS) $(MATH) $(STREAMS) $(MEMORY) $(LOADFILE) $(TOOLSTUB)
PIGS_LIBS += $(CPPLIB) $(LIBS) $(IFF)
$(RECOLIB) = $(WF_DIR)/source/objdebugglsoftwarexss.linux/librecolib.a
$(EVAL) = $(WF_DIR)/source/objdebugglsoftwarexss.linux/libeval.a
$(REGEXP) = $(WF_DIR)/source/objdebugglsoftwarexss.linux/libregexp.a

# kts these are needed by prep
PIGS_LIBS += $(RECOLIB) $(EVAL) $(REGEXP) $(IFFWRITE)

# kts these are needed by attribedit
PIGS_LIBS += $(PIGS_LIB_DIR)libini.a



tool-clean :
	rm -f $(OBJ_DIR)*$(OBJ) $(PROGRAM)
ifneq ($(OBJ_DIR),)
	rm -r -f $(OBJ_DIR)
endif
	rm -f *.err err
	rm -f vc50.* *.pdb *.ilk *.pch
	rm -f *.res resource.h

# These are included in GNUMakefile.rul included above.
#include $(PIGS_DIR)/GNUMakefile.print
#include $(PIGS_DIR)/GNUMakefile.test

# kts temporary rule for link, since I can't get the included makefiles to do it for me
$(PROGRAM): $(OBJS) $(PIGS_LIBS)
#	echo ------------------------------------------------------------------------------
#	echo OBJS is $(OBJS)
#	echo ------------------------------------------------------------------------------
#	echo PIGS_LIBS IS $(PIGS_LIBS)
	echo ------------------------------------------------------------------------------
	@echo Creating \> \> $@ \< \<
#	@echo ------------------------------------------------------------------------------
ifeq ($(WF_TARGET),linux)
	@echo ------------------------------------------------------------------------------
# Leaving PIGS_LIBS in, as it is already defined.
	gcc -o $@  $(OBJS) \
	-Xlinker --start-group \
	$(LIBS) \
	$(PIGS_LIBS) \
	-Xlinker --end-group \
	$(LNARGS) \
	-lm -lX11 -lXext -lXmu -lXt -lXi -lSM -lICE -lpthread -lc -lstdc++
#	$(SYS_LIBS)

ifeq ($(PROGRAM),prep)
	cp -f prep $(WF_DIR)/bin
endif
endif
