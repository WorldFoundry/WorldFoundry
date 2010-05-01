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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>

#include <pclib/stdstrm.hp>
#include <pclib/hdump.hp>

#include "pigtool.h"
#include "object.hp"
#include <source\levelcon.h>		// included from velocity\source

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//============================================================================

QObject::QObject(const char* newName, int16 newType, int16 newModelIndex,
				 const QPoint& newPosition,
				 const QPoint& newScale,
				 const br_euler& newRotation,
				 const QColBox& newColBox,
				 int32 newOadFlags, int16 newPathIndex,
				 const QObjectAttributeData& newOad
				)
{
	assert(newName);
	assert(strlen(newName) < NAMELEN);
	assert(newType);				// object type of zero is invalid

	strncpy(name,newName,NAMELEN);
	type = newType;
	modelIndex = newModelIndex;
	position = newPosition;
	scale = newScale;
	rotation.a = newRotation.a;
	rotation.b = newRotation.b;
	rotation.c = newRotation.c;
	rotation.order = BR_EULER_XYZ_S;
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
QObject::SizeOfOnDisk(void) const
{
	size_t dataSize = oad.SizeOfOnDisk();
	if(dataSize % 4)
		dataSize += 4-(dataSize % 4);
	assert((dataSize % 4) == 0);
	return(sizeof(_ObjectOnDisk)+dataSize);
}

//============================================================================

char zeros[4] = {0,0,0,0};

QObject::Save(FILE* fp, const QLevel& level)
{
	DBSTREAM3( cdebug << "QObject::Save" << endl; )

	_ObjectOnDisk diskObject;

	diskObject.type = type;
	diskObject.modelIndex = modelIndex;

	diskObject.x = position.x();
	diskObject.y = position.y();
	diskObject.z = position.z();

	diskObject.x_scale = scale.x();
	diskObject.y_scale = scale.y();
	diskObject.z_scale = scale.z();

	diskObject.rotation.a = rotation.a;
	diskObject.rotation.b = rotation.b;
	diskObject.rotation.c = rotation.c;
	diskObject.rotation.order = rotation.order;

//	diskObject.x = x;
//	diskObject.y = y;
//	diskObject.z = z;
	diskObject.oadFlags = oadFlags;

	collision.Write(&diskObject.coarse);
	diskObject.pathIndex = pathIndex;
	diskObject.OADSize = oad.SizeOfOnDisk();

//#warning kts not done here
	fwrite(&diskObject,sizeof(_ObjectOnDisk),1,fp);

#define OADDATASIZE 4096
	char oadBuffer[OADDATASIZE];
	ostrstream objOADOutputStream(oadBuffer,OADDATASIZE,ios::out|ios::binary);
	if(objOADOutputStream.rdstate() != ios::goodbit)
	 {
		cerror << "QObject::Save: Error creating strstream" << endl;
		exit(1);
	 }
	if(!oad.SaveStruct(objOADOutputStream, level, name, cerror))
	 {
		cerror << "QObject::Save: Error saving oad data" << endl;
		exit(1);
	 }

	int bytesToWrite = objOADOutputStream.tellp();
	assert(bytesToWrite < OADDATASIZE);
	assert(oadBuffer == objOADOutputStream.str());
	// insure that amount of data saved was amount I was told to save
	DBSTREAM3( cdebug << "QObject::Save: Writting out OAD data for object <" << name << "> at offset <" << ftell(fp) << ">, size = <" << bytesToWrite << ">" << endl; )
	DBSTREAM3( cdebug << "\tExpected length was " << diskObject.OADSize << endl; )
	assert(bytesToWrite == diskObject.OADSize);
	DBSTREAM3( HDump(objOADOutputStream.str(),bytesToWrite,4," ",cdebug); )
	int bytesWritten = fwrite(objOADOutputStream.str(),1,bytesToWrite,fp);
	assert(bytesWritten == bytesToWrite);
	if(diskObject.OADSize % 4)
	 {
		bytesToWrite = fwrite(zeros,1,4-(diskObject.OADSize%4),fp);				// long word align
		assert(bytesToWrite < 4);
		assert(bytesToWrite == 4-(diskObject.OADSize%4));
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
