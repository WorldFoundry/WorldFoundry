

TARGETS = Makefile.inc renderer.inc renderer.ext renderer.mak renderermake

all: $(TARGETS)

Makefile.inc : makefile.ins
	$(WF_DIR)/bin/prep makefile.ins Makefile.inc
	
renderer.inc : renderer.ins
	$(WF_DIR)/bin/prep renderer.ins renderer.inc
	
renderer.ext : renderer.exs
	$(WF_DIR)/bin/prep renderer.exs renderer.ext
	
renderer.mak : renderer.linux.bas
	$(WF_DIR)/bin/prep renderer.linux.bas renderer.mak
	
.PHONY : renderermake
renderermake :
	$(MAKE) -f renderer.mak $(MFLAGS)

clean:
	$(WF_DIR)/bin/prep rendererclean.linux.bas rendererclean
	chmod a+x rendererclean
	./rendererclean
	rm $(TARGETS) tmp.txt
