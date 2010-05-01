BLDMODE = tool

PLUGIN_BASE = attrib
!include "../Makefile.def"
THE_PLUGIN = $(WF_DIR)\install\gdk\3dsmax$(MAX_VERSION)\$(PLUGIN_BASE).dlu

include "../Makefile.inc"
CCCARGS += /D__ATTRIB__

.PATH.cc = .;buttons;..\lib

# Old buttons
#camera.obj radio.obj template.cpp waveform.obj

# Old files -- delete these
#class.* help.* tip.*

$(THE_PLUGIN) : $(STD_FILES) \
	$(PLUGIN_BASE).obj main.obj oad.obj oaddlg.obj util.obj \
		checkbox.obj classref.obj colorpck.obj \
		combobox.obj cycle.obj dropmenu.obj filename.obj fixed.obj \
		group.obj int.obj mailbox.obj meshname.obj objref.obj \
		propshet.obj string.obj xdata.obj \
		assert.obj \
		registry.obj levelnum.obj objname.obj \
		$(PLUGIN_BASE).res
	link /nologo $(LINK_FLAGS) /subsystem:windows /dll /def:$(PLUGIN_BASE).def /out:$(.TARGET) $(.SOURCES)

################################################################################

test : test.exe

test.exe : test.obj
	link /nologo $(LINK_FLAGS) /subsystem:console /out:$(.TARGET) $(.SOURCES)
