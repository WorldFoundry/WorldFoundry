
PLUGIN_BASE = max2iff
!include "../Makefile.def"
THE_PLUGIN = $(WF_DIR)\install\gdk\3dsmax$(MAX_VERSION)\$(PLUGIN_BASE).dle

include "../Makefile.inc"

$(THE_PLUGIN) : $(STD_FILES) \
	loadfile.obj $(PLUGIN_BASE).obj \
	about.obj \
	iffconv.obj \
	registry.obj \
	levelnum.obj \
	objname.obj \
	assert.obj \
	fixed.obj \
	_iffwr.obj \
	binary.obj \
	text.obj \
	stdstrm.obj \
	strmnull.obj \
	$(PLUGIN_BASE).res
	link /nologo $(LINK_FLAGS) /subsystem:windows /dll /def:$(PLUGIN_BASE).def /out:$(.TARGET) $(.SOURCES)

################################################################################
