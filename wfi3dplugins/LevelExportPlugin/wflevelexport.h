/***************************************************************************
                          ac3dfile.h  -  description
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

#ifndef WFLEVELEXPORT_H
#define WFLEVELEXPORT_H

#include <Plugins/fileplugin.h>

/**
  *@author Jon Anderson
  */

class WFLevelExport : public FilePlugin  {
public: 
	WFLevelExport();
	~WFLevelExport();
	
	void exportData(ofstream &);

protected:
	void getToken(char *, ifstream &);

	void readMaterial(ifstream &);
	void writeMaterials(ofstream &);

	void writeObject(Object *, ofstream &);
	void writeObjectVerts(Object *, ofstream &);
	void writeObjectSurfaces(Object *, ofstream &);
	void writeObjectSurfaceRefs(Face *, ofstream &);
	void writeObjectLocation(Object *, ofstream &);
	void writeObjectRotation(Object *, ofstream &);
	void writeObjectTexture(Object *, ofstream &);
	void writeObjectTextureRep(Object *, ofstream &);
	void writeObjectKids(Object *, ofstream &);
	void writeObjectName(Object *, ofstream &);
	void writeObjectData(Object *, ofstream &);
	void writeObjectURL(Object *, ofstream &);

	float m_texOffsetU;
	float m_texOffsetV;
	float m_texRepeatU;
	float m_texRepeatV;


};

#endif
