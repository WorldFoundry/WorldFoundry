/***************************************************************************
                          AttribPlugin.h  -  description
                             -------------------
    begin                : Sun Aug 20 2000
    copyright            : (C) 2000 by Jon Anderson
    email                : janderson@onelink.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AttribPlugin_H
#define AttribPlugin_H

#include "attribdialog.h"
#include <iostream.h>
#include <fstream.h>
#include <qvariant.h>
#include <qdialog.h>
#include <qwidget.h>
#include <qcheckbox.h>

#include <Plugins/dialogplugin.h>
#include <Entities/entitylib.h>

class DialogPlugin;

class AttribPlugin : public DialogPlugin, public AttribEditDialog
{ 

public:
    AttribPlugin();
    ~AttribPlugin();

    void startPlugin();
    void stopPlugin();

 protected:

    void DoEdit();

    ofstream out;
};

#endif // RIBDLG_H
