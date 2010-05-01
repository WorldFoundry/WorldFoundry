/***************************************************************************
                         geometryexport.cpp  -  description
                            -------------------
   begin                : Sat Dec 08 2001
   copyright            : (C) 2001 World Foundry Group
   based on sample code : (C) 2000 by Jon Anderson
   written by           : Kevin T. Seghetti
   email                : kts@worldfoundry.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Purpose: World Foundry geometry exporter
 * 
 * Limitations:
 *  - only doing one frame.
 *  - only triangles.
 *  - no skeleton/bone/limb info.
 * 
 * NOTES:
 * 
 * CS sprites can only have triangles.  We will probably automatically
 *  triangulate, or ask the user(or both).
 * 
 * The CS sprite format uses uv coordinates tied to verticies.
 * This is not good because most(all?) uv texture mappers work on with
 *  faces.  Ie they have a different uv coordinate at each vertex of a face.
 *
 * The problem with doing this is that for 3D cards
 * 
 * 
 * Assumptions about I3D:
 *  - +z is forward, -z is backward.
 *  - +y is up, -y is down.
 *  - polygons are anti-clockwise.
 * 
 * 
 * TODO:
 * 
 * 
 *  like to add some options to the exporter.
 *   - do per face uv mapping.
 *   - do per vertex uv mapping.
 *   - base directory for images in CS's VFS.
 *   - export sprite into a world.
 *   - export sprite template.
 *   - 
 *  Save the name for the sprite and sprite template into the I3D file.
 * 
 *  Add animation support.  Not sure how far along I3D is in this area yet.
 *   Would like to be able to save crystal space ACTIONS in I3D some how.
 * 
 *  Add import capability.  Would like to reuse some crystal space code for
 *   this.
 */




#include "geometryexport.h"
#include <vector>
#include <iostream.h>
#include <qmessagebox.h>

extern "C"
{
    FilePlugin * i3d_createInstance()
    {
        return new GeometryExportFile();
    }
}

GeometryExportFile::GeometryExportFile() : FilePlugin( "World Foundry GeometryExport", "iff" )
{
    setImportable( false );
    setExportable( true );

}

GeometryExportFile::~GeometryExportFile()
{
}

void GeometryExportFile::importData( ifstream &in )
{
    assert(0);
   // not implemented!!!

}

//==============================================================================

void 
GeometryExportFile::exportData( ofstream &out )
{
    vector<Selectable *>* list;
    list = ObjectDB::getInstance()->getSelected();

    //list = getMeshes();

    // kts for now just edit the last one selected
    Entity* entity = NULL;

    for(int i=0; i<(int)list->size(); i++)
    {
        if((*list)[i]->isA(Object::TYPE))
        {
            entity = (Entity *) (*list)[i];
        }
    }

    if(entity)  
        writeModel( ( Object * ) ( entity ), out );
    else
    {
       int n = 1;
       QString strMessage = "You must select at least ";
       QString num;
       num.setNum( n );
       strMessage += num;
       strMessage += " items first";
    
       QMessageBox::information( 0, "World Foundry Geometry Exporter", strMessage );
    }
   //writeSprite( ( Object * ) ( entity ), out );
}

//==============================================================================

void
GeometryExportFile::AddVertUVEntry(const VertexAndUV& vuv)
{
    vector<VertexAndUV>::iterator iter = find(vertexList.begin(),vertexList.end(),vuv);

    if(iter == vertexList.end())
    {
        // not found, add it
        vertexList.push_back(vuv);
    }
}

//==============================================================================

int
GeometryExportFile::LookupVertUVEntry(const VertexAndUV& vuv)
{
    vector<VertexAndUV>::iterator iter = find(vertexList.begin(),vertexList.end(),vuv);

    assert(iter != vertexList.end());

    int index = iter - vertexList.begin();
    return index;
}

//==============================================================================


inline int
ColorVectorToInt(const Vector4& color)
{
    return 
    ((((unsigned int)(color.x*256))<<16) |
     (((unsigned int)(color.y*256))<<8) |
     ((unsigned int)(color.z*256)));
}

//==============================================================================


inline int
ColorToInt(const Color& color)
{
    return 
    ((((unsigned int)color.r)<<16) |
     (((unsigned int)color.g)<<8) |
     ((unsigned int)color.b));
}

//==============================================================================

VertexAndUV
CreateVUV(Face& face,vector<int> *vlist , int vertexIndex, TextureMaterial* tm)
{
    VertexAndUV vuv;                                                  
    vector<int> *uvlist = face.getUVs();
    Vertex& vertex = *face.getParentObject()->getVertex( ( *vlist )[vertexIndex]);
    vuv.uv = face.getParentObject()->getUVCoord((*uvlist)[vertexIndex])->getPosition();     
    vuv.position = vertex.getPosition();

    if(vertex.isColored())
        vuv.color = ColorVectorToInt(vertex.getColor());
    else
    {
        if(tm)
            vuv.color = ColorToInt(tm->cDiffuse);
        else
            vuv.color = 0;
    }
    return vuv;
}

//==============================================================================

void 
GeometryExportFile::CreateVertUVList(Object* o)
{
    int size = (int)o->numFaces();
    VertexList& vlist = *o->getVerts();

    for(int i=0; i<size; i++)
    {

        Face& face = *o->getFace(i);
        vector<int> *vlist = face.getVerts();
        vector<int> *uvlist = face.getUVs();

        int size = (int)vlist->size();

        Vector4 uv;
        for(int i=0; i<size; i++)
        {
//             uv = face.getParentObject()->getUVCoord((*uvlist)[i])->getPosition();
//
//             VertexAndUV vuv;
//
//             vuv.position = vlist[(*vlist)[i]]->getPosition();
//             vuv.uv = uv;
//
            TextureMaterial* m = o->getTextureMaterial();
//
//             vuv.color = ColorToInt(m->cDiffuse);

            VertexAndUV vuv = CreateVUV(face,vlist,i,m);
            AddVertUVEntry(vuv);
        }
    }
}

//==============================================================================

void
GeometryExportFile::WriteVertexList(ofstream& out)
{
    out <<
    "   { 'VRTX'  \n"
    "			 // count: " << vertexList.size() << "       u,v,color,x,y,z" << endl;


    int index=0;
    for(vector<VertexAndUV>::iterator iter = vertexList.begin();iter != vertexList.end(); iter++,index++)
    {
        const VertexAndUV& vuv = *iter;

        out << "        " << vuv.uv.x <<"(1.15.16) "<< vuv.uv.y <<"(1.15.16) "<< vuv.color <<"l ";
        out << vuv.position.x <<"(1.15.16) " << -vuv.position.z << "(1.15.16) " << vuv.position.y << "(1.15.16)  //vertex #" << index << endl;

    }
    out << 
    "   }" << endl;        

}

//==============================================================================


const int MATERIAL_NAME_LEN = 256;

struct _MaterialOnDisk
{
    int _materialFlags;
    int _color;
//	int32 _opacity;					// actually a fixed32
    char textureName[MATERIAL_NAME_LEN];
};


enum
{
    // bits which change which renderer runs
    FLAT_SHADED = 0,
    GOURAUD_SHADED = 1,

    SOLID_COLOR = 0,
    TEXTURE_MAPPED = 2,

    LIGHTING_LIT = 0,
    LIGHTING_PRELIT = 4,

    SINGLE_SIDED = 0,
    DOUBLE_SIDED = 8,
};

void GeometryExportFile::writeModel( Object *o, ofstream &out )
{

    out << 
    "{ 'MODL'\n"
    "   { 'SRC' \"Innovation 3D exported by WF geometry exporter V0.11\"" 
    "   }\n"
    "   { 'TRGT' \"World Foundy Game Engine (www.worldfoundry.org)\"\n" 
    "   }\n" << endl;

    out <<
    "   { 'NAME' \"" << o->getName() << "\"\n" 
    "   }\n";


    out.setf(std::ios::showpoint);

     //Vector4 pos;
    float texturex=0;
    float texturey=0;
    float color=0;
     //pos = o->getPosition();

    int oldWidth = out.width(8);
    char oldFill = out.fill('0');

    // kts i3d is a left handed coordinate system with positive y up, WF is left handed with positive z up
    // so I rotate -90 degrees around x putting y up (swap y&z, negate the new y)

    VertexList *vlist = o->getVerts();
    int size = (int)vlist->size();
    Vector4 vertpos;
    Vector4 uv;

    for(int i=0; i<size; i++)
    {
        vertpos = (*vlist)[i]->getPosition();
        VertexAndUV vuv;
        vuv.position = vertpos;

    }

    CreateVertUVList(o);
    WriteVertexList(out);

    out << "    { 'MATL' //flags: [FLAT_SHADED=0, GOURAUD_SHADED=1] [SOLID_COLOR=0, TEXTURE_MAPPED=2] [SINGLE_SIDED=0, TWO_SIDED=8]" << endl;

        //save materials here.

    ObjectDB *odb = I3D::getDB();

    TextureMaterial *m;
    for(int materialIndex=0; materialIndex<odb->numMaterials(); materialIndex++)
    {
        m = odb->getMaterial(materialIndex);

        _MaterialOnDisk mod;

        mod._materialFlags = 0;
        if(m->enable_texture)
            mod._materialFlags |= TEXTURE_MAPPED;

        for(int temp=0;temp<MATERIAL_NAME_LEN;temp++)
            mod.textureName[temp] = 0;

        if(m->texture)
            strncpy(mod.textureName, m->texture->getFilename()->ascii(), MATERIAL_NAME_LEN);
        mod.textureName[MATERIAL_NAME_LEN-1] = 0;    // make sure it is 0 terminated

        mod._color = ((((unsigned int)m->cDiffuse.r)<<16) |
                      (((unsigned int)m->cDiffuse.g)<<8) |
                      ((unsigned int)m->cDiffuse.b));

        out << "            //Material " << materialIndex << ": flags: " << mod._materialFlags << ", color: " << hex << mod._color << dec << ", texturename: \"" << mod.textureName << "\"" << endl;
        out << "        ";
                                                                  
        for(int index=0;index<sizeof(_MaterialOnDisk);index++)
        {
            out << (unsigned int)(((unsigned char*)&mod)[index]) << "y ";
        }
        out << " // #" << materialIndex << endl;

    }
    out << "    }" << endl;        

    // faces
    size = (int)o->numFaces();

    out << "    { 'FACE' // count = " << size << endl;

        
    for(int faceIndex=0; faceIndex<size; faceIndex++)
    {
        Face* face = o->getFace(faceIndex);
        vector<int> *vlist = face->getVerts();
        vector<int> *uvlist = face->getUVs();

        assert(vlist->size() >= 3);              // must be at least a triangle

        // output list of triangles for this face
        
        // make a fan of triangles, first vertex is the same for all polys
        int vert1 = LookupVertUVEntry(CreateVUV(*face,vlist, 0, o->getTextureMaterial()));
        // second point                                         
        int vert2 = LookupVertUVEntry(CreateVUV(*face,vlist, 1, o->getTextureMaterial()));
        
        for(int vertexIndex=2;vertexIndex<vlist->size();vertexIndex++)
        {
            VertexAndUV vuv3 = CreateVUV(*face,vlist, vertexIndex, o->getTextureMaterial());
            int vert3 = LookupVertUVEntry(vuv3);
            int materialIndex = -1;
    
            if(o->getTextureMaterial() != 0)
            {
                materialIndex = I3D::getDB()->getMaterialIndex(o->getTextureMaterial());
            }
            else
                cerr << "material not set!" << endl;
            
            out << "        " << vert1 << "w " << vert2 << "w " << vert3 << "w " << materialIndex << "w   // face #" << faceIndex << ", poly #" << vertexIndex << endl;    
            
            // vert2 on next fan is vert3 from this fan
            vert2 = vert3;
        }
    }

    out << "    }" << endl;


    out << "}" << endl;
}

#if 0
void GeometryExportFile::writeSprite( Object *o, ofstream &out )
{

   //  out << "OBJECT poly"<<endl;
    out << "  WORLD (" << endl;
    out << "  TEXTURES (" << endl;
    out << "    MAX_TEXTURES (10)" << endl;
    out << "    TEXTURE 'abstract_a032.gif' ()" << endl;
    out << "    TEXTURE 'andrew_wood.gif' ()" << endl;
    out << "    TEXTURE 'sydney.gif' ()" << endl;
    out << "    TEXTURE 'bla.png' ()" << endl;
    out << "    TEXTURE 'bla2.gif' ()" << endl;
    out << "  )" << endl;


   // Don't know how to get the name.
    out << "SPRITE 'sydney'(" << endl;


   // Add the texture.
   // We also need to add the texture to the top of the world file.
    TextureMaterial *tm = o->getTextureMaterial();

    if(tm != 0 && tm->texture != 0)
    {
      // Need to get the file name that the image will be in CS's VFS.
      //    out << "TEXNR('"<<tm->texture->getFilename()->ascii()\
      //      << "')" <<endl;
        out << "TEXNR('bla2.gif')" << endl;

    }

   // Write the frames.  err... "frame".  We done needs verts, and
   //  uv coordinates.

   // We are going to waste some vertices here.
   //
   // We are going to add an extra set of vertices
   //
   // for each face:
   //   add to the new vertex list, the vertices in the face.
   //   add to the new uv coords list, the uvs for the face.
   //
   // The triangle list will be made by consecutively using three vertices
   //  from the list of vertices.


    VertexList *vlist = o->getVerts();

    UVList *uvlist = o->getUVs();

    vector<Vector4> new_vert_list;

    vector<Vector4> new_uv_list;



    int num_verts = ( int ) vlist->size();

    int num_uvs = ( int ) uvlist->size();

   //DEBUG
   //  out << "num_verts is:" << num_verts << " num_uvs is:" << num_uvs <<endl;


    int num_faces = ( int ) o->numFaces();

    int tris = -1;


    for(int i = 0; i < num_faces; i++)
    {
        Face *face = o->getFace( i );
        vector<int> *face_vlist = face->getVerts();
        vector<int> *face_uvlist = face->getUVs();


        int v_size = ( int ) face_vlist->size();
        int uv_size = ( int ) face_uvlist->size();

        for(int ii = 0; ii < uv_size; ii++)
        {
            new_uv_list.push_back( face->getParentObject() ->getUVCoord(
                                                                       ( *face_uvlist ) [ ii ] ) ->getPosition() );
        }

        for(int ii = 0; ii < v_size; ii++)
        {
            new_vert_list.push_back( face->getParentObject() ->getVertex(
                                                                        ( *face_vlist ) [ ii ] ) ->getPosition() );
        }
    }


   // write out the frames... er just one frame for now.

   // DEBUG
   //  out << "new_vert_list size:" << new_vert_list.size() << endl;
   //  out << "new_uv_list size:" << new_uv_list.size() << endl;

   // Just place the name of the frame for now.
    out << "FRAME 'stand1' (";

    for(uint i = 0; i < new_vert_list.size(); i++)
    {
        out << "V(" << new_vert_list[ i ].x << "," << new_vert_list[ i ].y << ",";

      // The z axis are oposite for I3D and CS.
        out << new_vert_list[ i ].z * -1 << ":";
      // Have to convert systems, as cs uses x = 0, y = 0 at top left.
      //  Inovation 3d uses bottom left.
        out << new_uv_list[ i ].x << "," << ( 1.0 - new_uv_list[ i ].y ) << ")";
        out << endl;
    }

    out << ")" << endl;

   // write out the list of triangles.
    for(int i = 0; i < new_vert_list.size(); i += 3)
    {
        out << "TRIANGLE(" << i + 2 << "," << i + 1 << "," << i << ")" << endl;
    }


}
#endif
