
PLUGIN_BASE = select

ALL : $(THE_PLUGIN)

BUILDMODE=dll
!include "../Makefile.def"
THE_PLUGIN = $(WF_DIR)\install\gdk\3dsmax$(MAX_VERSION)\$(PLUGIN_BASE).dlu

include "../Makefile.inc"

.PATH.cpp = .;..\lib

$(THE_PLUGIN) : $(STD_FILES) \
	about.obj \
	column.obj \
	inscol.obj \
	item.obj \
	main.obj \
	oad.obj \
	select.obj \
	util.obj \
	registry.obj levelnum.obj objname.obj \
	$(LIBS) \
	$(PLUGIN_BASE).res
	link /nologo $(LINK_FLAGS) /subsystem:windows /dll /def:$(PLUGIN_BASE).def /out:$(.TARGET) $(.SOURCES)

################################################################################
