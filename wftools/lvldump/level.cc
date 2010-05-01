//============================================================================
// level.cc: level data for lvldump
// Copyright (c) 1995-1999, World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <pigsys/assert.hp>
#include <iostream>
#include <fstream>
#include <strstream>
using namespace std;

#include "hdump.hp"
//#include <pclib/boolean.hp>
#include <cpplib/stdstrm.hp>

#include <vector>

//#include "pigtool.h"
#include "level.hp"
#include "oad.hp"
#include "asset.hp"

#pragma pack(1)
extern "C" {
#include <oas/oad.h>			// from the velocity project
};
#pragma pack()

//extern "C" {
//#pragma pack(4);
//#include "brender.h"
//#pragma pack(1);
//#include "dosio.h"
//#pragma pack(1);
//};

extern bool bAnalysis;
const int NUM_OAD_TYPE_BUCKETS = 256;
int oadTypeBuckets[ NUM_OAD_TYPE_BUCKETS ];

//============================================================================

#define BR_ONE_LS 	(1<<16)
#define FixedToFloat(s)	((s)*(1.0/(float)BR_ONE_LS))
#define MAX_DOT_LC_LINELEN 160

//============================================================================

vector<string> objectTypes;
vector<QObjectAttributeData> objectOADs;

//============================================================================

void
ReadLineIntoString(istream& in, string& dest)
{
	char tempBuffer[MAX_DOT_LC_LINELEN];

	in.getline(tempBuffer, MAX_DOT_LC_LINELEN, '\n');
	dest = tempBuffer;
}

//============================================================================

void
LowerCaseString(string& str)
{
	for(unsigned int index=0;index<str.length();index++)
		str[index] = tolower(str[index]);
}


void
LoadLCFile(const char* name)
{
	ifstream file(name);
	if(file.rdstate() != ios::goodbit)
	 {
		cerror << "LvlDump Error: LC file <" << name << "> not found." << endl;
		exit(1);
	 }
// ok, verify it is an .lc file
	string line;

	ReadLineIntoString(file,line);
//	line = line(0,11);					// truncate
	line.assign(line,0,11);
	if(line != "Objects.lc:")
	 {
		cerror << "LvlDump Error: <" << name << "> is not a .LC file" << endl;
		exit(1);
	 }

// look for opening {
	while(line.find("{",0) == string::npos)
//	while(!line.contains("{"))
	 {
		ReadLineIntoString(file,line);
//		line.readLine(file);
		if(file.rdstate() != ios::goodbit)
		 {
			cerror << "LvlDump Error: problem parsing .LC file" << endl;
			exit(1);
		 }
	 }
	ReadLineIntoString(file,line);				// skip {

// ok, now create in memory enumeration of all of the game objects
//	while(!line.contains("}"))
	while(line.find("}",0) == string::npos)
	 {
		if(line.length())
		 {

			LowerCaseString(line);
			// trim off leading whitespace
			while( (line[0] == '\t') || (line[0] == ' ') )
				line.erase(0,1);

			objectTypes.push_back(line);
			cdebug << "QLevel::LoadLCFile: Inserting object type <" << line << ">" << endl;
		 }
		ReadLineIntoString(file,line);					// read next line
		if(file.rdstate() != ios::goodbit)
		 {
			cerror << "LvlDump Error: problem parsing .LC file" << endl;
			exit(1);
		 }
	 }
	assert(string("nullobject") == objectTypes[0]);
}

//============================================================================

void
DumpLCFile(ostream& output)
{
// now print in memory enumeration
	output << "Game Object Enumeration" << endl;
	for(unsigned int index=0;index<objectTypes.size();index++)
	 {
		output << "Object Index <" << index  << ">, Name <" << objectTypes[index] << ">" << endl;
	 }
}

//============================================================================
// note: LoadLCFile must have been called first

void
LoadOADFiles(const char* lcFileName)
{
// now load all of the .oad files for all of the objects
	cprogress << "Loading .OAD files" << endl;
#if 0
	char drive[_MAX_DRIVE];
	char path[_MAX_PATH];
	char result[_MAX_PATH];

	_splitpath(lcFileName,drive,path,NULL,NULL);
	_makepath(result,drive,path,NULL,NULL);
#else
#define LCFILENAME "objects.lc"
	char result[PATH_MAX];
    assert(strlen(lcFileName) < PATH_MAX);
    assert(strlen(lcFileName) > strlen(LCFILENAME));
    strncpy(result, lcFileName, strlen(lcFileName) - strlen(LCFILENAME));
    result[strlen(lcFileName) - strlen(LCFILENAME)] = 0;
#endif
	objectOADs.push_back(QObjectAttributeData());						// create 0 entry, which is never used
	string line;

	for(int index=1;index<objectTypes.size();index++)		// note: this starts at one since 0 is the null object
	 {
		line = result;
		line += objectTypes[index];
		line += ".oad";
		cdebug << "QLevel::LoadOADFiles: Line = <" << line << ">" << endl;

        cprogress << "opening OAD file " << line << endl;
		ifstream input(line.c_str(),ios::in|ios::binary);
		if(input.rdstate() != ios::goodbit)
		 {
			cerror << "LevelCon Error: problem reading .OAD file <" << line << ">" << endl;
			exit(1);

		 }
		QObjectAttributeData oad;
		if(oad.Load(input,cerror))
		 {
			cdebug << "QLevel::LoadOADFiles: Inserting object" << endl;
			objectOADs.push_back(oad);
			cdebug << "QLevel::LoadOADFiles: Done inserting object" << endl;
		 }
		else
			exit(1);						// since oad.Load will print whatever the error is
	 }
	cprogress << "  Done loading OAD files" << endl;


}

//============================================================================

//void
//DumpModels(_LevelOnDisk* levelData, ostream& output)
//{
//	_ModelOnDisk* modelData = (_ModelOnDisk*)((char*)levelData + (levelData->modelsOffset));

//	int _numModels = levelData->modelCount;
//	output << "  Model Dump:" << endl;
//	output << "  Number of models: " << _numModels << endl << endl;
//	for (int index = 0; index < _numModels; index++)
//	 {
//		output << "    Model index [" << index << "] = " << modelData[index].name << endl;
//	 }
//}

//============================================================================

void
DumpObjectHistogram( _LevelOnDisk* levelData, ostream& output )
	{
	for ( int i=0; i<NUM_OAD_TYPE_BUCKETS; ++i )
		{
		if ( oadTypeBuckets[ i ] )
			{
			if ( objectTypes.size() )
				output << objectTypes[i] << "(" << i << "): " << oadTypeBuckets[i] << endl;
			else
				output << i << ": " << oadTypeBuckets[i] << endl;
			}
		}
	}


void
DumpObjects(_LevelOnDisk* levelData, ostream& output)
{
	output << "  Object Dump:" << endl;

	int32* objArray = (int32*)((char*)levelData + (levelData->objectsOffset));
	assert(levelData->objectCount > 0);
	// start at 1 since object 0 is invalid
	for (int index = 1; index < levelData->objectCount; index++)
	 {
		output << "--------------------------------------------------------------------------------" << endl;

		output << "  Object " << index << " is at offset  " << objArray[index] << endl;
		_ObjectOnDisk* objdata = (_ObjectOnDisk*)(((char*)levelData) + objArray[index]);

		assert( objdata->type < NUM_OAD_TYPE_BUCKETS );
		++oadTypeBuckets[ objdata->type ];

		if(objectTypes.size())				// if not zero, lc file must be loaded
		 {
			output << "    Type = " << objectTypes[objdata->type] << " (" << (int)objdata->type << ")" << endl;
		 }
		else
			output << "    Type = " << (int)objdata->type << endl;
//		output << "    ModelIndex = " << objdata->modelIndex << endl;
		output << "    PathIndex = " << objdata->pathIndex << endl;
		if(objectOADs.size())
		 {
			// kts add code here to look up and print the name of this model
		 }
		output << endl;

		output << "    Position: X:" << FixedToFloat(objdata->x) << " Y:" << FixedToFloat(objdata->y);
		output << " Z:" << FixedToFloat(objdata->z) << endl;
		output << "    Coarse collision rect: " << endl;

		output << "    MinX:" << FixedToFloat(objdata->coarse.minX) << " MinY:" << FixedToFloat(objdata->coarse.minY);
		output << " MinZ:" << FixedToFloat(objdata->coarse.minZ) << endl;
		output << "    MaxX:" << FixedToFloat(objdata->coarse.maxX) << " MaxY:" << FixedToFloat(objdata->coarse.maxY);
		output << " MaxZ:" << FixedToFloat(objdata->coarse.maxZ) << endl;

//		output << "    Initial rotation:  X-Axis: " << BrScalarToFloat(BrAngleToRadian(objdata->rotation.a)) << endl;
//		output << "                       Y-Axis: " << BrScalarToFloat(BrAngleToRadian(objdata->rotation.b)) << endl;
//		output << "                       Z-Axis: " << BrScalarToFloat(BrAngleToRadian(objdata->rotation.c)) << endl;

		output << "    Collision rect offset by position: " << endl;
		output << "      MinX:" << FixedToFloat(objdata->coarse.minX+objdata->x) << " MinY:" << FixedToFloat(objdata->coarse.minY+objdata->y);
		output << " MinZ:" << FixedToFloat(objdata->coarse.minZ+objdata->z) << endl;
		output << "      MaxX:" << FixedToFloat(objdata->coarse.maxX+objdata->x) << " MaxY:" << FixedToFloat(objdata->coarse.maxY+objdata->y);
		output << " MaxZ:" << FixedToFloat(objdata->coarse.maxZ+objdata->z) << endl;

		output << "    OadFlags: " << objdata->oadFlags << endl;
		output << "    OADSize: " << objdata->OADSize << endl;

		void* oadBuffer = (void*)(objdata+1);

		char* commonBlockAddr = ((char*)levelData + (levelData->commonDataOffset));
		assert(commonBlockAddr);
//		assert(levelData->commonDataLength);

		if(objectOADs.size())
		 {
			assert(objectOADs.size() > objdata->type);								// insure we have an entry for this type
			QObjectAttributeData tempOAD = objectOADs[objdata->type];

			cdebug << "DO: dumping object # " << index << ", OADSize = " << objdata->OADSize << endl;
			assert(objdata->OADSize);
            cdebug << "creating objOADStream with a length of " << objdata->OADSize << endl;
			strstream objOADStream((char*)oadBuffer,objdata->OADSize,ios::in|ios::binary );
			if(!tempOAD.LoadStruct(objOADStream,cerror,commonBlockAddr,levelData->commonDataLength))
				cerror << "Error loading OAD for this object, might be incorrect" << endl;
//			if(!tempOAD.LoadStruct(objOADStream,cerror))
//				exit(1);
			output << tempOAD;
			cdebug << " SizeOfOnDisk = " << tempOAD.SizeOfOnDisk()<< endl;
			assert(tempOAD.SizeOfOnDisk() == objdata->OADSize);
			HDump(oadBuffer, objdata->OADSize, 4," ",cdebug);
		 }
		else
			HDump(oadBuffer, objdata->OADSize, 4," ",output);
	 }
}

//============================================================================

void
DumpPaths(_LevelOnDisk* levelData, ostream& output)
{
	output << "Paths Dump:" << endl;

	int32* pathArray = (int32*)((char*)levelData + (levelData->pathsOffset));

	for(int index = 0; index < levelData->pathCount; index++)
	 {
		output << "  Path # " << index << " is at offset  " << pathArray[index] << endl;
		_PathOnDisk* pathData = (_PathOnDisk*)(((char*)levelData) + pathArray[index]);
		output << "    Path base at position: " << FixedToFloat(pathData->base.x) << ","
			<< FixedToFloat(pathData->base.y) << "," << FixedToFloat(pathData->base.z) << endl;
		output << "    Rotation base: " << FixedToFloat(pathData->base.rot.a) << ","
			<< FixedToFloat(pathData->base.rot.b) << "," << FixedToFloat(pathData->base.rot.c) << endl;
		output << "    X position using channel #: " << pathData->PositionXChannel << endl;
		output << "    Y position using channel #: " << pathData->PositionYChannel << endl;
		output << "    Z position using channel #: " << pathData->PositionZChannel << endl;
		output << "    A rotation using channel #: " << pathData->RotationAChannel << endl;
		output << "    B rotation using channel #: " << pathData->RotationBChannel << endl;
		output << "    C rotation using channel #: " << pathData->RotationCChannel << endl;

//		_PathOnDiskEntry* entryArray = (_PathOnDiskEntry*)((char*)pathData + sizeof(_PathOnDisk));
//		for(int entry=0;entry<pathData->count;entry++)
//		 {
////			printf("  Path key at: %d, %d, %d, at time index: %d\n",entryArray[entry].x,entryArray[entry].y,entryArray[entry].z,entryArray[entry].time);
//				output << "  Frame = " << entryArray[entry].frame << endl;
//				output << "  Path key at position: "
//					<< FixedToFloat(entryArray[entry].x) << ","
//					<< FixedToFloat(entryArray[entry].y) << ","
//					<< FixedToFloat(entryArray[entry].z) << endl;
//				output << "  Path key + base =: "
//					<< FixedToFloat(entryArray[entry].x + pathData->base.x) << ","
//					<< FixedToFloat(entryArray[entry].y + pathData->base.y) << ","
//					<< FixedToFloat(entryArray[entry].z + pathData->base.z) << endl;
//				output << "  Rotation: "
//					<< FixedToFloat(entryArray[entry].rot.a) << ","
//					<< FixedToFloat(entryArray[entry].rot.b) << ","
//					<< FixedToFloat(entryArray[entry].rot.c) << endl;
//				output << "  Rotation + base =: "
//					<< FixedToFloat(entryArray[entry].rot.a + pathData->base.rot.a) << ","
//					<< FixedToFloat(entryArray[entry].rot.b + pathData->base.rot.b) << ","
//					<< FixedToFloat(entryArray[entry].rot.c + pathData->base.rot.c) << endl;
//		 }
	 }
}

//============================================================================

void
DumpChannels(_LevelOnDisk* levelData, ostream& output)
{
	output << "Channels Dump:" << endl;

	int32* channelArray = (int32*)((char*)levelData + (levelData->channelsOffset));

	for(int index = 0; index < levelData->channelCount; index++)
	{
		output << "  Channel # " << index << " is at offset  " << channelArray[index] << endl;
		_ChannelOnDisk* channelData = (_ChannelOnDisk*)(((char*)levelData) + channelArray[index]);
		output << "    Compressor Type = " << channelData->compressorType << endl;
		output << "    End Time = " << FixedToFloat( channelData->endTime ) << endl;
		output << "    Number of Keys = " << channelData->numKeys << endl;

		_ChannelOnDiskEntry* entryArray = (_ChannelOnDiskEntry*)((char*)channelData + sizeof(_ChannelOnDisk));
		for (int entryIndex=0; entryIndex < channelData->numKeys; entryIndex++)
		{
			output << "    Key #" << entryIndex;
			output << ": Time = " << FixedToFloat( entryArray[entryIndex].time );
			output << ", Value = " << entryArray[entryIndex].value << endl;
		}
	}

}

//============================================================================

void
DumpRooms(_LevelOnDisk* levelData, ostream& output)
{
	output << "Rooms Dump:" << endl;

	int32* roomArray = (int32*)((char*)levelData + (levelData->roomsOffset));
	for(int index = 0; index < levelData->roomCount; index++)
	 {
		output << "  Room # " << index << " is at offset  " << roomArray[index] << endl;
		_RoomOnDisk* roomData = (_RoomOnDisk*)(((char*)levelData) + roomArray[index]);

		output << "Room #" << index << ", contains " << roomData->count << " entries" << endl;
		output << "  And is object #" << roomData->roomObjectIndex << endl;

 		output << " MinX:" << FixedToFloat(roomData->boundingBox.minX) << " MinY:" << FixedToFloat(roomData->boundingBox.minY);
		output << " MinZ:" << FixedToFloat(roomData->boundingBox.minZ) << endl;

		output << " MaxX:" << FixedToFloat(roomData->boundingBox.maxX) << " MaxY:" << FixedToFloat(roomData->boundingBox.maxY);
		output << " MaxZ:" << FixedToFloat(roomData->boundingBox.maxZ) << endl;

		for(int adjRoomIndex=0; adjRoomIndex < MAX_ADJACENT_ROOMS; adjRoomIndex++)
		 {
			output << "  Room Reference #" << adjRoomIndex << ": " << roomData->adjacentRooms[adjRoomIndex] << endl;
		 }

		_RoomOnDiskEntry* entryArray = (_RoomOnDiskEntry*)((char*)roomData + sizeof(_RoomOnDisk));
		for(int entry=0;entry<roomData->count;entry++)
		 {
			output << "    Contains object #" << entryArray[entry].object << endl;
		 }
	 }
}

//============================================================================

void
DumpCommonBlock(_LevelOnDisk* levelData, ostream& output)
{
	output << "Common Block Dump:" << endl;
	char* commonBlock = ((char*)levelData + (levelData->commonDataOffset));
	HDump(commonBlock, levelData->commonDataLength, 4," ",output);
}

//============================================================================

void
DumpLevel(_LevelOnDisk* levelData, ostream& output)
{
//	assert(levelData->versionNum == LEVEL_VERSION);

	for ( int i=0; i<NUM_OAD_TYPE_BUCKETS; ++i )
		oadTypeBuckets[ i ] = 0;

	output << "================================================================================" << endl;
	if(objectOADs.size())
	 {
		DumpLCFile(output);
		output << "================================================================================" << endl;
	 }

	output << "Level Loaded: " << endl;
	output << "Version #: " << levelData->versionNum << endl;
	if(levelData->versionNum != LEVEL_VERSION)
		{
//		Beep();
		cerror << "Warning: level version # does not match, expected <" << LEVEL_VERSION << ">,found <" << levelData->versionNum << ">, data could be incorrect" << endl;
		}

	assert( (levelData->objectsOffset & 3) == 0 );
	output << "Object Count: " << levelData->objectCount << " at offset " << levelData->objectsOffset  << endl;
//	assert( (levelData->modelsOffset & 3) == 0 );
//	output << "Model Count: " << levelData->modelCount << " at offset " << levelData->modelsOffset  << endl;
	assert( (levelData->pathsOffset & 3) == 0 );
	output << "Path Count " << levelData->pathCount << " at offset " << levelData->pathsOffset  << endl;
	assert( (levelData->channelsOffset & 3) == 0 );
	output << "Channel Count " << levelData->channelCount << " at offset " << levelData->channelsOffset << endl;
	assert( (levelData->roomsOffset & 3) == 0 );
	output << "Room Count " << levelData->roomCount << " at offset " << levelData->roomsOffset  << endl;
	assert( (levelData->commonDataOffset & 3) == 0 );
	output << "Common Data Length " << levelData->commonDataLength << " at offset " << levelData->commonDataOffset  << endl;
	output << "================================================================================" << endl;
//	DumpModels(levelData, output);
//	output << "================================================================================" << endl;
	DumpObjects(levelData, output);
	output << "================================================================================" << endl;
	DumpPaths(levelData, output);
	output << "================================================================================" << endl;
	DumpChannels(levelData, output);
	output << "================================================================================" << endl;
	DumpRooms(levelData, output);
	output << "================================================================================" << endl;
	DumpCommonBlock(levelData, output);
	output << "================================================================================" << endl;
	if ( bAnalysis )
		{
		output << "Object Histogram:" << endl;
		DumpObjectHistogram( levelData, output );
		}
	output << "================================================================================" << endl;
}

//============================================================================
