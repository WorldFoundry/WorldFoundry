/***************************************************************************
                         cssprfile.h  -  description
                            -------------------
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

                        
#ifndef GeometryExportFILE_H
#define GeometryExportFILE_H

#include <Plugins/fileplugin.h>
#include <vector>

//==============================================================================
  
struct VertexAndUV
{
    Vector4 position;
    Vector4 uv;
    unsigned int color;

    inline bool operator==( const VertexAndUV& ) const;

};


inline bool 
VertexAndUV::operator==( const VertexAndUV& other) const
{
    return (
                position == other.position &&
                uv == other.uv &&
                color == other.color
           );
}


//==============================================================================

class GeometryExportFile : public FilePlugin
{
   public:
      GeometryExportFile();
      ~GeometryExportFile();

      void importData( ifstream & );
      void exportData( ofstream & );
      void writeModel( Object *o, ofstream &out );
      void writeSprite( Object *o, ofstream &out );

      // protected:
private:
    void AddVertUVEntry(const VertexAndUV& vuv);
    void CreateVertUVList(Object* o);
    void WriteVertexList(ofstream& out);

    int LookupVertUVEntry(const VertexAndUV& vuv);

   vector<VertexAndUV> vertexList;

};

//==============================================================================
#endif
//==============================================================================

