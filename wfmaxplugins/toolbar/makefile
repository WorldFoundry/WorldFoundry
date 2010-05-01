BLDMODE = tool

PLUGIN_BASE = toolbar
!include "../Makefile.def"
THE_PLUGIN = $(WF_DIR)\install\gdk\3dsmax2\$(PLUGIN_BASE).dlu

include "../Makefile.inc"

.PATH.cpp = .;..\lib

$(THE_PLUGIN) : \
		$(PLUGIN_BASE).obj main.obj \
		clean.obj lint.obj run.obj make.obj expobj.obj \
		registry.obj levelnum.obj objname.obj loaddll.obj \
		$(PLUGIN_BASE).res
	link /nologo $(LINK_FLAGS) /subsystem:windows /dll /def:$(PLUGIN_BASE).def /out:$(.TARGET) $(.SOURCES)

################################################################################
