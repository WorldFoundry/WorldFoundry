lib_LTLIBRARIES = libattrib.la

libdir = $(i3d_dir)/Plugins

libattrib_la_SOURCES =   attribdialog.cpp  moc_attribdialog.cpp profile.cc wfattribdata.cc attribplugin.cpp

INCLUDES= $(all_includes) $(I3D_CFLAGS) $(QT_CFLAGS) $(NURBS_CFLAGS)

moc_attribdialog.cpp : attribdialog.h
	$(MOC) -o moc_attribdialog.cpp attribdialog.h

attribdialog.h : attribplugin.ui
	$(UIC) -o attribdialog.h attribplugin.ui

attribdialog.cpp : attribplugin.ui attribdialog.h
	$(UIC) -i attribdialog.h -o attribdialog.cpp attribplugin.ui
