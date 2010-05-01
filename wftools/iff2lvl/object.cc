//============================================================================
// object.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of level object data
	History:
			Created 05-05-95 10:26am Kevin T. Seghetti
			Added object scaling	10/10/95 ptorre

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================

#include "global.hp"
//#include <stdio.h>
//#include <string.h>
//#include <iostream.h>
//#include <fstream.h>
//#include <strstrea.h>

#include "hdump.hp"

//#include "pigtool.h"
#include "object.hp"
#include <oas/levelcon.h>		// included from wfsource

//============================================================================

//extern Interface* gMaxInterface;

//============================================================================

QObject::QObject(const string newName, int16 newType,
				 const Point3& newPosition,
				 const Point3& newScale,
				 const Euler& newRotation,
				 const QColBox& newColBox,
				 int32 newOadFlags, int16 newPathIndex,
				 const QObjectAttributeData& newOad
				)
{
	assert(newName.length() < NAMELEN);
	assert(newType);				// object type of zero is invalid

	strncpy(name,newName.c_str(),NAMELEN);
	type = newType;
	position = newPosition;
	scale = newScale;
	rotation.a = newRotation.a;
	rotation.b = newRotation.b;
	rotation.c = newRotation.c;
	collision = newColBox;
	oadFlags = newOadFlags;
	pathIndex = newPathIndex;
	oad = newOad;
}

//============================================================================

QObject::~QObject()
{
}

//============================================================================

size_t
QObject::SizeOfOnDisk() const
{
	size_t dataSize = oad.SizeOfOnDisk();
	if(dataSize % 4)
		dataSize += 4-(dataSize % 4);
	assert((dataSize % 4) == 0);
	return sizeof( _ObjectOnDisk ) + dataSize;
}

//============================================================================

char zeros[4] = {0,0,0,0};

void
QObject::Save(ostream& lvl, const QLevel& level)
{
	DBSTREAM3( cdebug << "QObject::Save" << endl; )

	_ObjectOnDisk diskObject;

	diskObject.type = type;

	diskObject.x = WF_FLOAT_TO_SCALAR(position.x);
	diskObject.y = WF_FLOAT_TO_SCALAR(position.y);
	diskObject.z = WF_FLOAT_TO_SCALAR(position.z);

	diskObject.x_scale = WF_FLOAT_TO_SCALAR(scale.x);
	diskObject.y_scale = WF_FLOAT_TO_SCALAR(scale.y);
	diskObject.z_scale = WF_FLOAT_TO_SCALAR(scale.z);

	diskObject.rotation.a = WF_FLOAT_TO_SCALAR(rotation.a / (2 * PI));
	diskObject.rotation.b = WF_FLOAT_TO_SCALAR(rotation.b / (2 * PI));
	diskObject.rotation.c = WF_FLOAT_TO_SCALAR(rotation.c / (2 * PI));

//	diskObject.x = x;
//	diskObject.y = y;
//	diskObject.z = z;
	diskObject.oadFlags = oadFlags;

	collision.Write(&diskObject.coarse);
	diskObject.pathIndex = pathIndex;
	diskObject.OADSize = oad.SizeOfOnDisk();

//#warning kts not done here
//	fwrite(&diskObject,sizeof(_ObjectOnDisk),1,fp);
	lvl.write( (const char*)&diskObject, sizeof(_ObjectOnDisk) );

#define OADDATASIZE 4096
//	char oadBuffer[OADDATASIZE];
//	ostrstream objOADOutputStream(oadBuffer,OADDATASIZE,ios::out|ios::binary);
//	AssertMessageBox(objOADOutputStream.good(), "QObject::Save: Error creating strstream");
	if(!oad.SaveStruct(lvl, level, name, cerror))
	 {
		AssertMessageBox(0, "QObject::Save: Error saving oad data");
	 }

//	int bytesToWrite = objOADOutputStream.tellp();
//	assert(bytesToWrite < OADDATASIZE);
//	assert(oadBuffer == objOADOutputStream.str());
//	// ensure that amount of data saved was amount I was told to save
//	DBSTREAM3( cdebug << "QObject::Save: Writting out OAD data for object <" << name << "> at offset <" << lvl.tellp() << ">, size = <" << bytesToWrite << ">" << endl; )
//	DBSTREAM3( cdebug << "\tExpected length was " << diskObject.OADSize << endl; )
//	assert(bytesToWrite == diskObject.OADSize);
//	DBSTREAM3( HDump(objOADOutputStream.str(),bytesToWrite,4," ",cdebug); )
//	lvl.write
//	int bytesWritten = fwrite(objOADOutputStream.str(),1,bytesToWrite,fp);
//	assert(bytesWritten == bytesToWrite);
	if(diskObject.OADSize % 4)
	{
//		bytesToWrite = fwrite(zeros,1,4-(diskObject.OADSize%4),fp);				// long word align
		lvl.write( (char*)zeros, 4-(diskObject.OADSize%4) );
//		assert(bytesToWrite < 4);
//		assert(bytesToWrite == 4-(diskObject.OADSize%4));
	}
}

//============================================================================
// Printing

ostream& operator<<(ostream& s, const QObject &object)
{
	s << "QObject Dump" << endl;

	s << "Name = <" << object.name << ">" << endl;
	s << object.position << endl;
	s << "Scale = " << object.scale << endl;
//	s << "X = " << BrScalarToFloat(object.x) << "  Y = " << BrScalarToFloat(object.y) << "  Z = " << BrScalarToFloat(object.z) << endl;
//	s << "A = " << object.a << "  B = " << object.b << "  C = " << object.c << endl;
	s << "Type = " << object.type << endl;

	s << object.oad;
//	if(object.typeData)
//	 {
//		s << "Type Data:" << endl;
//		HDump(object.typeData,object.typeDataSize,0,"",s);
//	 }

	return s;
}

//============================================================================
