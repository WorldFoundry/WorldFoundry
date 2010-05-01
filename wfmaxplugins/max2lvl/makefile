
PLUGIN_BASE = max2lvl
!include "../Makefile.def"
THE_PLUGIN = $(WF_DIR)\install\gdk\3dsmax$(MAX_VERSION)\$(PLUGIN_BASE).dle

include "../Makefile.inc"

CCCARGS += /I"../lib/stl" -DDO_DEBUG_FILE_SYSTEM

.PATH.cpp = .;..\lib
.PATH.cc = .;..\lib

$(THE_PLUGIN) : $(STD_FILES) \
	About.obj \
	3d.obj \
	assert.obj \
	Asset.obj \
	channel.obj \
	Colbox.obj \
	dlgstrm.obj \
	Hdump.obj \
	Level.obj \
	Level2.obj \
	Level3.obj \
	Level4.obj \
	Level5.obj \
	Levelcon.obj \
	Loadfile.obj \
	lvlexp.obj \
	Oad.obj \
	Object.obj \
	Path.obj \
	Point6.obj \
	registry.obj \
	Room.obj \
	Stdstrm.obj \
	strmnull.obj \
	Symbol.obj \
	Template.obj \
	..\eval\expr_tab.obj \
	..\eval\lexyy.obj \
	$(PLUGIN_BASE).res
	link /nologo $(LINK_FLAGS) /NODEFAULTLIB:LIBC /subsystem:windows /dll /def:$(PLUGIN_BASE).def /out:$(.TARGET) $(.SOURCES)

################################################################################

