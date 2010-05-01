

TARGETS = Makefile.inc renderer.inc renderer.ext renderer.mak renderermake

PREP = ..\..\..\bin\prep

all: $(TARGETS)

Makefile.inc : makefile.ins
	$(PREP) makefile.ins Makefile.inc
	
renderer.inc : renderer.ins
	$(PREP) renderer.ins renderer.inc
	
renderer.ext : renderer.exs
	$(PREP) renderer.exs renderer.ext
	
renderer.mak : renderer.linux.bas
	$(PREP) renderer.linux.bas renderer.mak
	
.PHONY : renderermake
renderermake :
	make -f renderer.mak $(MFLAGS)

clean:
	$(PREP) rendererclean.windows.bas rendererclean.bat
	rendererclean.bat
	del $(TARGETS) tmp.txt
