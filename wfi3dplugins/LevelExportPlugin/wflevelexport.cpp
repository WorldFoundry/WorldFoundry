/***************************************************************************
                          WFLevelExport.cpp  -  description
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

#include "wflevelexport.h"
#include "../AttribPlugin/wfattribdata.hp"
#include <Math/quat.h>
#include <vector>
#include <iostream>
#include <sstream>

extern "C" {
FilePlugin * i3d_createInstance(){
	return new WFLevelExport();
}
}
WFLevelExport::WFLevelExport() : FilePlugin("WFLevelExport", "lev")
{
	//setImportable(true);
	setExportable(true);
	
}
WFLevelExport::~WFLevelExport(){
}

void WFLevelExport::exportData(ofstream &out)
{
	vector<Selectable *> *list;
	list = getMeshes();

	int size = (int)list->size();


   out << "// created by wflevelexport V0.2" << endl;

   out <<  "{ 'LVL'" << endl;
	
	//writeMaterials(out);

	for(int i=0; i<size; i++){
		writeObject((Object *)(*list)[i], out);
	
	}

   out << "}" << endl;

}
void  WFLevelExport::writeObject(Object *o, ofstream &out)
{
	out << "    { 'OBJ'" << endl;
      
   string name = o->getName();
   out << "        { 'NAME' \"" << name << "\"\n        }" << endl;


   // output position
   out << "        { 'VEC3' \n            { 'NAME' \"Position\"\n            }\n            { 'DATA' ";


   out.setf(std::ios::showpoint);

	Vector4 pos;
	pos = o->getPosition();
   int oldWidth = out.width(8);
   char oldFill = out.fill('0');

   // kts i3d is a left handed coordinate system with positive y up, WF is left handed with positive z up
   // so I rotate -90 degrees around x putting y up (swap y&z, negate the new y)

	out << pos.x<<"(1.15.16) "<< -pos.z<<"(1.15.16) "<< pos.y<<"(1.15.16)" << "  //x,y,z\n            }\n        }" << endl;

   Quat orientation = o->getOrientation();

   Vector4 eulerOrientation = orientation.getEuler();

   out << "        { 'EULR'\n            { 'NAME' \"Orientation\"\n            }\n            { 'DATA' ";
	out << eulerOrientation.x<<"(1.15.16) "<< -eulerOrientation.z<<"(1.15.16) "<< eulerOrientation.y<<"(1.15.16)  //a,b,c\n            }\n        }" << endl;

   Vector4 bboxMin,bboxMax;
	o->getBoundingMin(&bboxMin);
   o->getBoundingMax(&bboxMax);
   out << "        { 'BOX3'\n            { 'NAME' \"Global Bounding Box\"\n            }\n            { 'DATA' ";
	out << bboxMin.x << "(1.15.16) " << -bboxMax.z << "(1.15.16) " << bboxMin.y<<"(1.15.16) ";
	out << bboxMax.x << "(1.15.16) " << -bboxMin.z << "(1.15.16) " << bboxMax.y<<"(1.15.16)  //min(x,y,z)-max(x,y,z)\n            }\n        }" << endl;

   out.width(oldWidth);
   out.fill(oldFill);

//    writeObjectName(o, out);
//    writeObjectData(o, out);
//    writeObjectTexture(o, out);
//    writeObjectTextureRep(o, out);
//    writeObjectRotation(o, out);
//    writeObjectLocation(o, out);
//    writeObjectURL(o, out);
//    writeObjectVerts(o, out);
//    writeObjectSurfaces(o, out);
//    writeObjectKids(o, out);


   IData* origData = (o->getData(ATTRIBUTEDATANAME));
   WorldFoundryAttributeData* data = dynamic_cast<WorldFoundryAttributeData*>(origData);

   if(data)
   {
      const string& dStr = data->toString();
      cout << "data found:\n" << dStr << endl;

      std::istringstream is(dStr);

      string line;
      while(getline(is,line))
      {
         if(line == "{ 'OBJ' " || line  == "}")
         {
            // skip it
         }
         else
            out << "    " << line << endl;
      }
   }
   out << "    }" << endl;
}

void WFLevelExport::writeObjectVerts(Object *o, ofstream &out)
{
	VertexList *vlist = o->getVerts();
	int size = (int)vlist->size();
	Vector4 pos;

	out << "numvert "<<size<<endl;
	for(int i=0; i<size; i++){
		pos = (*vlist)[i]->getPosition();
		out << pos.x << " " <<pos.y<<" "<<pos.z<<endl;		
	}
}
void WFLevelExport::writeObjectSurfaces(Object * o, ofstream & out)
{
	int size = (int)o->numFaces();
//	Vector4 pos;

	out << "numsurf "<<size<<endl;
	for(int i=0; i<size; i++){
		out << "SURF 0x10"<<endl;
		if(o->getTextureMaterial() != 0){
			out << "mat "<<I3D::getDB()->getMaterialIndex(o->getTextureMaterial())<<endl;
		}
		writeObjectSurfaceRefs(o->getFace(i), out);
	}
}
void WFLevelExport::writeObjectSurfaceRefs(Face *f, ofstream &out)
{
	vector<int> *flist = f->getVerts();
	vector<int> *uvlist = f->getUVs();

	int size = (int)flist->size();
	
	out << "refs "<<size<<endl;
	Vector4 uv;
	for(int i=0; i<size; i++){
                uv = f->getParentObject()->getUVCoord((*uvlist)[i])->getPosition();    	
		out << (*flist)[i]<< " "<<uv.x<<" "<<uv.y<<endl;
	}
}
void WFLevelExport::writeObjectLocation(Object *o, ofstream &out)
{
	Vector4 pos;
	pos = o->getPosition();
	out << "loc " << pos.x<<" "<<pos.y<<" "<<pos.z<<endl;
}
void WFLevelExport::writeObjectRotation(Object *o, ofstream &out)
{
}
void WFLevelExport::writeObjectTexture(Object *o, ofstream &out)
{
	TextureMaterial *tm = o->getTextureMaterial();
	if(tm != 0 && tm->texture != 0){
        	out << "texture \""<<tm->texture->getFilename()->ascii()<<"\""<<endl;
	}
}
void WFLevelExport::writeObjectTextureRep(Object *o, ofstream &out)
{
} 
void WFLevelExport::writeObjectKids(Object *o, ofstream &out)
{
	out << "kids 0"<<endl;
}
void WFLevelExport::writeObjectName(Object *o, ofstream &out)
{


	out << "name \"jon\""<<endl;
}
void WFLevelExport::writeObjectData(Object *o, ofstream &out)
{
	out << "data \"jon\""<<endl;
}
void WFLevelExport::writeObjectURL(Object *o, ofstream &out)
{

}

void WFLevelExport::readMaterial(ifstream &in)
{
	char line[1024];

        in.getline(line, sizeof(line), '\n');
	/*char buffer[256];

	TextureMaterial *tm = new TextureMaterial();

	float r, g, b;

	in >> buffer >> ws; //read in the material tag
	in >> buffer >> ws; //read in the name.

	in >> buffer >> ws; //read in the color tag;
	in >> r >> g >> b;
	tm->cDiffuse.r = r*255;
	tm->cDiffuse.g = g*255;
	tm->cDiffuse.b = b*255;

	in >> buffer >> ws; //read in the color tag;
	in >> r >> g >> b;
	tm->cAmbient.r = r*255;
	tm->cAmbient.g = g*255;
	tm->cAmbient.b = b*255;

	in >> buffer >> ws; //read in the color tag;
	in >> r >> g >> b;
	tm->cEmissive.r = r*255;
	tm->cEmissive.g = g*255;
	tm->cEmissive.b = b*255;

	in >> buffer >> ws; //read in the color tag;
	in >> r >> g >> b;
	tm->cSpecular.r = r*255;
	tm->cSpecular.g = g*255;
	tm->cSpecular.b = b*255;

	in >> buffer >> ws; //read in shininess tag;
	in >> tm->shininess >> ws;

	in >> buffer >> ws; //read in shininess tag;
	in >> r >> ws;
	tm->alpha = r * 255;

          */
}

void WFLevelExport::writeMaterials(ofstream &out)
{
        //save materials here.
   ObjectDB* odb = I3D::getDB();
   assert(odb);

	TextureMaterial *m;
 	for(int i=0; i<odb->numMaterials(); i++){
		m = odb->getMaterial(i);
		out << "MATERIAL \"\"";// << (*materials)[i]->getName()<<" ";
		out << "rgb " << (float)m->cDiffuse.r/255 <<" "<<(float)m->cDiffuse.g/255 <<" "<<(float)m->cDiffuse.b/255 <<" ";
		out << "amb " << (float)m->cAmbient.r/255 <<" "<<(float)m->cAmbient.g/255 <<" "<<(float)m->cAmbient.b/255 <<" ";
		out << "emis "<< (float)m->cEmission.r/255<<" "<<(float)m->cEmission.g/255<<" "<<(float)m->cEmission.b/255<<" ";
		out << "spec "<< (float)m->cSpecular.r/255<<" "<<(float)m->cSpecular.g/255<<" "<<(float)m->cSpecular.b/255<<" ";
		out << "shi " << (float)m->shininess<<" ";
		out << "trans "<<(float)(255-m->alpha)/255<<endl;
	  }

}
