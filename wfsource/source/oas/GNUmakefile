#==============================================================================
# Makefile: primary Makefile to build everything in the oas directory
#==============================================================================

PREP =  ../../bin/prep

include ../../GNUMakefile.env

#==============================================================================

COMMONBLOCKSHT = movebloc.ht toolset.ht activate.ht common.ht mesh.ht shadowp.ht actor.ht
COMMONBLOCKSDEF = $(OAD_DIR)/movebloc.def $(OAD_DIR)/toolset.def $(OAD_DIR)/activate.def $(OAD_DIR)/common.def $(OAD_DIR)/mesh.def $(OAD_DIR)/shadowp.def $(OAD_DIR)/actor.def

ALL: $(OAD_DIR) objects.p objects $(COMMONBLOCKSHT) $(COMMONBLOCKSDEF) objects.c $(OAD_DIR)/objects.lc objects.inc objects.e objects.mi objects.ms objects.col objects.ctb objects.car $(OAD_DIR)/assets.txt $(OAD_DIR)/objects.iff $(OAD_DIR)/objects.xml

$(OAD_DIR) :
	mkdir $(OAD_DIR)

objects.mak : GNUobjects.mas objects.mac 
	$(PREP) GNUobjects.mas objects.mak

$(OAD_DIR)/objects.iff : objects.ifs objects.mac
	$(PREP) objects.ifs $(OAD_DIR)/objects.iff

$(OAD_DIR)/objects.xml : objects.xms objects.mac
	$(PREP) objects.xms $(OAD_DIR)/objects.xml

objects.c : objects.s objects.mac
	$(PREP) objects.s objects.c

objects.e : objects.es objects.mac
	$(PREP) objects.es objects.e

objects.p : objects.ps objects.mac
	$(PREP) objects.ps objects.p

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
	perl coltab.pl

$(OAD_DIR)/objects.lc : objects.lcs objects.mac
	$(PREP) objects.lcs $(OAD_DIR)/objects.lc
#       $(PREP) objects.lcs $^@

objects : objects.mak
	make -f objects.mak


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
	perl cstruct.pl %|fF.tmp >%|fF.ht
	del %|fF.tmp > /dev/null


# create the .def files for the .inc files
$(OAD_DIR)/movebloc.def :       movebloc.oas movebloc.inc oaddef.s

$(OAD_DIR)/toolset.def :        toolset.oas toolset.inc oaddef.s

$(OAD_DIR)/activate.def :       activate.oas activate.inc oaddef.s

$(OAD_DIR)/common.def : common.oas common.inc oaddef.s

$(OAD_DIR)/mesh.def :   mesh.oas mesh.inc oaddef.s

$(OAD_DIR)/shadow.def : shadow.oas shadow.inc oaddef.s

$(OAD_DIR)/actor.def :  actor.oas actor.inc oaddef.s

#==============================================================================

# create header file with C structures in it for game to include
%.ht : %.oas 
	$(PREP)  -DTYPEFILE_OAS=$(*F) oadtypes.s $(*F).pp
	perl cstruct.pl $(*F).pp >$@
# .ht is oad header file extension

#==============================================================================

# create header file for the scripting language
$(OAD_DIR)/%.def : %.oas 
	$(PREP)  -DTYPEFILE_OAS=$(*F) oaddef.s $(*F).pp
	perl cstruct.pl $(*F).pp >$(*F).tmp
	perl offset.pl $(*F).tmp >$@
	rm $(*F).tmp 

#==============================================================================

assets.inc : ../streams/assets.inc
	cp $< $@

$(OAD_DIR)/assets.txt : assets.inc assets.s
	$(PREP) assets.s $(OAD_DIR)/assets.txt

#==============================================================================
.PHONY:	print
print:
	@echo +----------------------------------------------------------------------------+
	@echo OAD_DIR = $(OAD_DIR)
	@echo PREP = $(PREP)
	@echo +----------------------------------------------------------------------------+

#==============================================================================
clean:
	-rm -f *.obj *.ht *.pp *.tmp *.iff.txt
	-rm -f objects.mak objects.e objects.mi  objects.inc objects.c objects.p objects.ms
	-rm -f $(OAD_DIR)/objects.lc $(OAD_DIR)/*.iff $(OAD_DIR)/*.tmp
	-rm -f $(OAD_DIR)/*.def
	-rm -f $(OAD_DIR)/*.iff
	-rm -f $(OAD_DIR)/*.xml
	-rm -f $(OAD_DIR)/assets.txt
	-rm -f $(OAD_DIR)/*.iff.txt
	
#==============================================================================

install:
	echo install not implemented for oas dir yet
	
