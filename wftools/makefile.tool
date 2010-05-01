# wftools/Makefile.tool

#ALL : $(WF_DIR)\bin\$(PROGRAM).exe
#
#$(WF_DIR)\bin\$(PROGRAM).exe : $(PROGRAM).exe
#        copy $(.SOURCE) $(.TARGET)
#

BUILDMODE=tool
!include "$(WF_DIR)\Makefile.env"
!include "$(PIGS_DIR)\pigs.dep"

#INCLUDE += -I ..\recolib -I ..

PIGS_LIBS += $(PIGSYS) $(CPPLIB) $(MATH) $(STREAMS) $(MEMORY) $(LOADFILE) $(TOOLSTUB)
#$(MENU) 
PIGS_LIBS += $(LIBS)

$(PROGRAM).exe : $(OBJS) $(PIGS_LIBS) 
	echo Linking tool
        link /debug /LIBPATH:"$(MSVCDir)\lib" /OUT:$(.TARGET) $(.SOURCES) 

tool-clean :
        -del $(OBJ_DIR)*.obj $(PROGRAM).exe
!if "$(OBJ_DIR)"
        -rd $(OBJ_DIR)
!endif
        -del *.err err
        -del vc50.* *.pdb *.ilk *.pch
	-del *.res resource.h

!include $(PIGS_DIR)\Makefile.print

