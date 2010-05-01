
PLUGIN_BASE = handle
!include "../Makefile.def"
THE_PLUGIN = $(WF_DIR)\install\gdk\3dsmax$(MAX_VERSION)\$(PLUGIN_BASE).dlm

include "../Makefile.inc"

.PATH.cpp = .;..\lib

$(THE_PLUGIN) : \
		$(PLUGIN_BASE).obj main.obj \
		$(PLUGIN_BASE).res
	link /nologo $(LINK_FLAGS) /subsystem:windows /dll /def:$(PLUGIN_BASE).def /out:$(.TARGET) $(.SOURCES)

################################################################################
