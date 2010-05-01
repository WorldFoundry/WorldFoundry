
PLUGIN_BASE = max2lev
!include "../Makefile.def"
THE_PLUGIN = $(WF_DIR)\install\gdk\3dsmax$(MAX_VERSION)\$(PLUGIN_BASE).dle

include "../Makefile.inc"

CCCARGS += /I"//lib/stl"

$(THE_PLUGIN) : $(STD_FILES) \
	$(PLUGIN_BASE).obj \
	main.obj \
	about.obj \
	assert.obj \
	box.obj \
	channel.obj \
	hdump.obj \
	loadfile.obj \
	oad.obj \
	oadentry.obj \
	oadobj.obj \
	path.obj \
	point6.obj \
	scene.obj \
	ui.obj \
	fixed.obj \
	text.obj \
	binary.obj \
	_iffwr.obj \
	stdstrm.obj \
	strmnull.obj \
	registry.obj \
	loaddll.obj \
	$(PLUGIN_BASE).res
	link /nologo $(LINK_FLAGS) /subsystem:windows /dll /def:$(PLUGIN_BASE).def /out:$(.TARGET) $(.SOURCES)

################################################################################
