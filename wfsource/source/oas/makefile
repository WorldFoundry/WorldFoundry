#==============================================================================
# Makefile: primary Makefile to build everything in this directory
#==============================================================================

.MS_NMAKE

PREP =  ..\..\bin\prep.exe
TOPALL: ALL

#==============================================================================


!include ../../Makefile.env


COMMONBLOCKSHT = movebloc.ht toolset.ht activate.ht common.ht mesh.ht shadowp.ht actor.ht
COMMONBLOCKSDEF = $(OAD_DIR)\movebloc.def $(OAD_DIR)\toolset.def $(OAD_DIR)\activate.def $(OAD_DIR)\common.def $(OAD_DIR)\mesh.def $(OAD_DIR)\shadowp.def $(OAD_DIR)\actor.def


#==============================================================================

ALL : $(OAD_DIR) objects.p objects $(COMMONBLOCKSHT) $(COMMONBLOCKSDEF) objects.c $(OAD_DIR)\objects.lc objects.inc objects.e objects.mi objects.ms objects.col objects.ctb objects.car $(OAD_DIR)\assets.txt $(OAD_DIR)\objects.iff $(OAD_DIR)\objects.xml

#==============================================================================


.SUFFIXES :
.SUFFIXES :     .oad .ht .def .oas .txt

#==============================================================================

$(OAD_DIR) :
	md /S $(OAD_DIR)

objects.mak : objects.mas objects.mac
	$(PREP) objects.mas objects.mak

$(OAD_DIR)\objects.xml : objects.ifs objects.mac
	$(PREP) objects.xms $(OAD_DIR)\objects.xml
	   
$(OAD_DIR)\objects.iff : objects.ifs objects.mac
	$(PREP) objects.ifs $(OAD_DIR)\objects.iff

objects.c : objects.s objects.mac
	$(PREP) objects.s objects.c

objects.p : objects.ps objects.mac
	$(PREP) objects.ps objects.p

objects.e : objects.es objects.mac
	$(PREP) objects.es objects.e

objects.inc : objects.ins objects.mac
	$(PREP) objects.ins objects.inc

objects.mi : objects.mis objects.mac
	$(PREP) objects.mis objects.mi

objects.ms : objects.mss objects.mac
	$(PREP) objects.mss objects.ms

objects.col : objects.cos objects.mac
	$(PREP) objects.cos objects.col

objects.ctb : objects.cts objects.mac
	$(PREP) objects.cts objects.ctb

objects.car : objects.ctb objects.e objects.mac coltab.pl
	..\..\bin\perl coltab.pl

$(OAD_DIR)\objects.lc : objects.lcs objects.mac
	$(PREP) objects.lcs $(OAD_DIR)\objects.lc
#       $(PREP) objects.lcs $^@

objects : objects.mak
	make /EN /nologo /f objects.mak

# create header file with C structures in it for game to include
.oas.ht :
	..\..\bin\prep  -DTYPEFILE_OAS=%|fF oadtypes.s %|fF.pp
	..\..\bin\perl cstruct.pl %|fF.pp >%|fF.ht
# .ht is oad header file extension

# create header files for the scripting language
.oas{$(OAD_DIR)}.def :
	$(PREP)  -DTYPEFILE_OAS=%|fF oaddef.s %|fF.pp
	..\..\perl cstruct.pl %|fF.pp >%|fF.tmp
	perl offset.pl %|fF.tmp >$OAD_DIR)\%|fF.def
	rm %|fF.tmp %|fF.pp 



# create the .inc files
movebloc.ht :   movebloc.oas movebloc.inc

toolset.ht :    toolset.oas toolset.inc

activate.ht :   activate.oas activate.inc

common.ht :     common.oas common.inc

mesh.ht :       mesh.oas mesh.inc

shadow.ht :     shadow.oas shadow.inc

actor.ht :      actor.oas actor.inc

.oas.inc :
	$(PREP)  -DTYPEFILE_OAS=%|fF oadtypes.s %|fF.tmp
	..\..\bin\perl cstruct.pl %|fF.tmp >%|fF.ht
	del %|fF.tmp > nul


# create the .def files for the .inc files
$(OAD_DIR)\movebloc.def :       movebloc.oas movebloc.inc oaddef.s

$(OAD_DIR)\toolset.def :        toolset.oas toolset.inc oaddef.s

$(OAD_DIR)\activate.def :       activate.oas activate.inc oaddef.s

$(OAD_DIR)\common.def : common.oas common.inc oaddef.s

$(OAD_DIR)\mesh.def :   mesh.oas mesh.inc oaddef.s

$(OAD_DIR)\shadow.def : shadow.oas shadow.inc oaddef.s

$(OAD_DIR)\actor.def :  actor.oas actor.inc oaddef.s

.oas{$(OAD_DIR)}.def :
	..\..\bin\prep  -DTYPEFILE_OAS=%|fF oaddef.s %|fF.pp
	..\..\bin\perl cstruct.pl %|fF.pp >%|fF.tmp
	..\..\bin\perl offset.pl %|fF.tmp >$*.def
	del %|fF.tmp > nul

assets.inc : ..\streams\assets.inc
	copy $(.SOURCE) $(.TARGET)

$(OAD_DIR)\assets.txt : assets.inc assets.s
	$(PREP) assets.s $(OAD_DIR)\assets.txt

#==============================================================================

clean :
        del $(OAD_DIR)\*.oad > nul
	del $(OAD_DIR)\*.tmp > nul
	del $(OAD_DIR)\*.def > nul
	del $(OAD_DIR)\*.iff >nul
	del $(OAD_DIR)\*.xml >nul
	del $(OAD_DIR)\*.iff.txt > nul
	del $(OAD_DIR)\assets.txt > nul
	del *.pp > nul
        del *.obj > nul
        del *.ht > nul
        del *.pp > nul
        del *.exe > nul
        del objects.mak > nul
        del objects.e > nul
        del objects.p > nul
        del objects.mi > nul
        del $(OAD_DIR)\objects.lc > nul
        del objects.inc > nul
        del objects.c > nul
        del *.tmp > nul
	del assets.inc
