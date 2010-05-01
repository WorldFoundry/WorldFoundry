//============================================================================
// level3.cc
//============================================================================
//
// This file is a continuation of level.cc from levelcon; I'm breaking the source file into
// multiple chunks in hopes that Watcom will be able to compile it in debug mode.
//============================================================================

#include "global.hpp"
#include <iostream.h>
#include <fstream.h>
#include "levelcon.hpp"
#include "level.hpp"
#include "asset.hpp"
#include <stl/algo.h>
#include <stl/bstring.h>
#include "hdump.hpp"
#include "symbol.hpp"
#include "template.hpp"
#include "../lib/registry.h"

extern int parseStringIntoParts(  const char* _string, char* _xDataParameters[], int nMaxParameters );

//============================================================================

extern Interface* gMaxInterface;

//============================================================================
// This method walks the object OADs and creates an in-memory image of the
// common data area, patching the offsets in the OAD entries as it
// goes.  Called by QLevel::Save() after the _LevelOnDisk struct has been
// filled in, and before anything is written out to disk.  Returns size
// of data written to commonArea.

void
QLevel::CreateCommonData(void)
{
	DBSTREAM1( cprogress << "CreateCommonData: started" << endl; )

	QObjectAttributeDataEntry* td;
	QObjectAttributeDataEntry* temp_td;
	QObjectAttributeData *theOAD;

	char tempBlock[MAXBLOCKSIZE];
	int32 tempBlockSize;

	bool isInCommonBlock;

	for (unsigned int index=0; index < objects.size(); index++)
	{
		DBSTREAM3( cdebug << "QLevel::CreateCommonData: examining object #" << index << endl; )
		theOAD = (QObjectAttributeData *)&objects[index]->GetOAD();

		// walk entries in this OAD and build temporary common block
		isInCommonBlock = false;
		tempBlockSize = 0;
		for(unsigned int entryIndex=0;entryIndex < theOAD->entries.size(); entryIndex++)
		{
			DBSTREAM3( cdebug << "QLevel::CreateCommonData: examining index " << entryIndex << endl; )
			td = &theOAD->entries[entryIndex];

			if (isInCommonBlock == true)
			{
				DBSTREAM3( cdebug << "QLevel::CreateCommonData: isInCommonBlock = true, type = " << (int)td->GetType() << endl; )
				switch(td->GetType())
				{
					case BUTTON_INT16:
					case BUTTON_FIXED16:
						assert(0);		// only longwords allowed now
//						memcpy(&tempBlock[tempBlockSize], (char*)&td->GetDef(), 2);
//						tempBlockSize += 2;
						break;
					case BUTTON_FIXED32:
					case BUTTON_INT32:
					{
						int32 tempVal = td->GetDef();
						memcpy(&tempBlock[tempBlockSize], (char*)&tempVal, 4);
						tempBlockSize += 4;
						DBSTREAM3( cdebug << "  CCD: int32 added named " << td->GetName() << ", value = " << td->GetDef() << endl; )
						break;
					}
					case BUTTON_INT8:
						assert(0);		// only longwords allowed now
//						memcpy(&tempBlock[tempBlockSize], (char*)&td->GetDef(), 1);
//						tempBlockSize++;
						break;
					case BUTTON_STRING:
						assert(0);
//						memcpy(&tempBlock[tempBlockSize], td->GetString(), td->len);
//						tempBlockSize += td->len;
//						DBSTREAM3( cdebug << "  CCD: string added named " << td->name << ", value = <" << td->string << ">" << endl; )
						break;
					case BUTTON_PROPERTY_SHEET:
						break;								// not used by us

					case BUTTON_OBJECT_REFERENCE:
					 {
						int32 index = -1;
						if(td->GetString().length() != 0)
						 {
							index = FindObjectIndex(td->GetString());
							if(index < 0)
							 {
								cdebug << "object <" << td->GetString() << "> refered to by object in commonblock not found" << endl;
								DBSTREAM3( cdebug << "Object Name List:" << endl; )
								DBSTREAM3( PrintObjectList(cdebug); )
							 }
						 }
						DBSTREAM3( else )
							DBSTREAM3( cdebug << "LevelCon Warning: object reference <" << td->GetName() << "> contains empty string in common block" << endl; )

						memcpy(&tempBlock[tempBlockSize],&index,4);
						DBSTREAM3( cdebug << "  CCD: object reference added named " << td->GetName() << ", name = <" << td->GetString() << ">, index = " << index << endl; )
						tempBlockSize += 4;
						break;
					 }

					case LEVELCONFLAG_ENDCOMMON:
						isInCommonBlock = false;
						if (tempBlockSize > 0)				// skip if no common data
						 {
							assert (tempBlockSize < MAXBLOCKSIZE);
							temp_td->SetDef(AddCommonBlockData(tempBlock,tempBlockSize));
							DBSTREAM3( cdebug << "QLevel::CreateCommonData: Created common block, size = " << tempBlockSize << endl; )
							DBSTREAM3( HDump(tempBlock,tempBlockSize,4,">",cdebug); )
							tempBlockSize = 0;
						 }
						break;
					case BUTTON_FILENAME:
					{
						int32 tempVal = td->GetDef();
						memcpy(&tempBlock[tempBlockSize], (char*)&tempVal, 4);
						tempBlockSize += 4;
						DBSTREAM3( cdebug << "  CCD: filename added named " << td->GetName() << ", value = " << td->GetDef() << endl; )
						break;
					}
					case BUTTON_XDATA:
						if(td->GetXDataConversionAction() == XDATA_COPY ||
						   td->GetXDataConversionAction() == XDATA_SCRIPT ||
						   td->GetXDataConversionAction() == XDATA_OBJECTLIST ||
						   td->GetXDataConversionAction() == XDATA_CONTEXTUALANIMATIONLIST)					// if output flag
						{
							int32 tempVal = td->GetDef();
							memcpy(&tempBlock[tempBlockSize], (char*)&tempVal, 4);
							tempBlockSize += 4;
							DBSTREAM3( cdebug << "  CCD: xdata reference <" << td->GetName() << ">, string = <" << td->GetString() << ">, value = <" << td->GetDef() << ">" << endl; )
						}
						break;
					case BUTTON_WAVEFORM:
						break;
					case BUTTON_CLASS_REFERENCE:
					 {
						int32 typeIndex = -1;
						if(td->GetString().length() != 0)
						 {
							typeIndex = GetClassIndex(td->GetString());
							AssertMessageBox(typeIndex != -1, "Object reference type of <" << td->GetString() << "> not found in .LC file");
						 }
						DBSTREAM3( else )
//							cdebug << "LevelCon Warning: class reference <" << td->GetName() << "> contains empty string in object <" << name << ">" << endl;
							DBSTREAM3( cdebug << "LevelCon Warning: class reference <" << td->GetName() << "> contains empty string" << endl; )

						DBSTREAM3( cdebug << "QOAD::SaveStruct::CLASS_REFERENCE: writting out index of <" << index << ">" << endl; )

//						cout << "UNTESTED: commonblock: Wrote class index of " << typeIndex << endl;
						memcpy(&tempBlock[tempBlockSize], (char*)&typeIndex, 4);
						tempBlockSize += 4;
						break;
					 }
					case BUTTON_GROUP_START:
						break;
					case BUTTON_GROUP_STOP:
						break;
					case LEVELCONFLAG_NOMESH:
						break;
					case LEVELCONFLAG_SINGLEINSTANCE:
						break;
					case LEVELCONFLAG_TEMPLATE:
						break;
					case LEVELCONFLAG_EXTRACTCAMERA:
						break;
					case LEVELCONFLAG_EXTRACTCAMERANEW:
						break;
					case LEVELCONFLAG_EXTRACTLIGHT:
						break;
					case LEVELCONFLAG_SHORTCUT:
						AssertMsg( 0, "Invalid to have LEVELCONFLAG_SHORTCUT and COMDATA" );
						break;
					default:
						AssertMessageBox( 0, "ButtonType <" << (int)td->GetType() << "> found in common block.  I'm confused." );
					break;
				}
				assert (tempBlockSize < MAXBLOCKSIZE);
			}

			if (td->GetType() == LEVELCONFLAG_COMMONBLOCK)
			{
				isInCommonBlock = true;
				temp_td = td;
				DBSTREAM3( cdebug << "QLevel::CreateCommonData: starting common block at entry " << theOAD->entries[entryIndex] << endl; )
				DBSTREAM3( cdebug << "  Entry dump:" << endl << *theOAD << endl; )
			}
		}
	}

	DBSTREAM3( cdebug << "QLevel::CreateCommonData:  Done...Total common data size = " << commonAreaSize << endl << endl; )
}


//============================================================================

void
QLevel::SortIntoRooms()
{
	DBSTREAM3( cprogress << "SortIntoRooms:" << endl; )
	assert(rooms.size() == 0);
#if 1

	// loop through all objects, checking for rooms
	for(unsigned int roomIndex=0;roomIndex < objects.size(); roomIndex++)
	{
		unsigned int oadIndex = objects[roomIndex]->GetTypeIndex();
		assert(objectOADs.size() > oadIndex);

		if(objectOADs[oadIndex].GetEntryByType(LEVELCONFLAG_ROOM))
		{
			QRoom room;
			QColBox roomBox = objects[roomIndex]->GetColBox();
			roomBox += objects[roomIndex]->Position();
			room.SetBox(roomBox);
			room.SetOAD(objects[roomIndex]->GetOAD());
			room.SetName(objects[roomIndex]->GetName());
			DBSTREAM3( cdebug << "QLevel::SortIntoRooms: Found a room at index <" << roomIndex << "> named <" << objects[roomIndex]->GetName()  << ">" << endl; )
			DBSTREAM3( cdebug << "  at postion " << objects[roomIndex]->Position() << " with a colbox of " << objects[roomIndex]->GetColBox() << endl; )
			for(unsigned int objIndex=0;objIndex < objects.size(); objIndex++)
			{
				DBSTREAM3( cdebug << "  QLevel::SortIntoRooms:Checking Object <" << objects[objIndex]->GetName() << ">" << endl; )
				if(objIndex != roomIndex && room.InsideCheck(objects[objIndex]))
				{
					DBSTREAM3( cdebug << "  QLevel::SortIntoRooms:Object <" << objects[objIndex]->GetName() << "> is inside" << endl; )
					unsigned int roomOadIndex = objects[objIndex]->GetTypeIndex();
					assert(objectOADs.size() > roomOadIndex);
					if(objectOADs[roomOadIndex].GetEntryByType(LEVELCONFLAG_ROOM))
					{
						AssertMessageBox( 0, "Error: rooms <" << objects[roomIndex]->GetName()
						<< "> and <" << objects[objIndex]->GetName() << "> overlap" );
					}
					else
						room.Add(objects[objIndex]);
				}
			}
			if(room.length() == 0)
				cwarn << "Levelcon Warning: room <" << objects[roomIndex]->GetName() << "> contains 0 objects" << endl;
			rooms.push_back(room);					// add new room
		}
	}
#endif
	if(rooms.size() == 0)
	 {
		cwarn << "No rooms found in level, Creating one default room" << endl;
		// kts for now just create a single room and place everything in it

		QRoom room;
		QColBox roomBox
		(
			Point3(WF_INT_TO_SCALAR(-32767),WF_INT_TO_SCALAR(-32767),WF_INT_TO_SCALAR(-32767)),
			Point3(WF_INT_TO_SCALAR(32767),WF_INT_TO_SCALAR(32767),WF_INT_TO_SCALAR(32767))
		);
		DBSTREAM3( cdebug << "Generated Room ColBox: " << endl << roomBox << endl; )
		room.SetBox(roomBox);

		for(unsigned int roomIndex=0;roomIndex < objects.size(); roomIndex++)
		{
			DBSTREAM3( cdebug << "QLevel::SortIntoRooms: adding object <" << objects[roomIndex] << ">" << endl; )
			room.Add(objects[roomIndex]);
		}
		rooms.push_back(room);
	}
}

//============================================================================

#if 0			                        // kts 11/2/97 08:40
void
QLevel::CreateMergedObjects(const string& outputFileName)
{
	DBSTREAM1 (cprogress << "CreateMergedObjects" << endl; )
	for(unsigned int currentRoom = 0;currentRoom < rooms.size();currentRoom++)
		allMergedObjects.push_back(vector<mergedObject>());

	if(mergeObjects)
	{
		DBSTREAM3( cdebug << "QLevel::CreateMergedObjects" << endl; )
		char levelName[_MAX_FNAME];
		_splitpath(outputFileName.c_str(),NULL,NULL,levelName,NULL);

		int32 statplatClassIndex = GetClassIndex("statplat");
		int32 mergedClassIndex = GetClassIndex("merged");
		assert(mergedClassIndex > 0);
		assert(statplatClassIndex > 0);
		char path[_MAX_PATH];
		char name[_MAX_FNAME];
		char ext[_MAX_EXT];
		char result[_MAX_PATH];

		// loop through all objects in each room, collecting all statplats into one large merged object (per room)
		vector<QRoom>::iterator rIter = rooms.begin();
		while(rIter != rooms.end())
	 	{
			const char* modelName;
			vector<string> assetVect;
			allMergedObjects.push_back(vector<mergedObject>());

			for(int objIndex = 0; objIndex < (*rIter).length(); objIndex++)
		 	{
				const QObject& obj = (*rIter).GetObjectByIndex(objIndex);
				int16 modelIndex = obj.GetModelIndex();
				if(modelIndex != MODEL_NULL)
			 	{
					QModel& model = models[modelIndex];
					const string sname(model.GetName());
					modelName = sname.c_str();
					_splitpath(modelName,NULL,path,name,ext);
					strlwr(ext);
					for(map<string,string,less<string> >::iterator iter(extensionsMap.begin());iter != extensionsMap.end();iter++)
						if((*iter).first == ext)
							strcpy(ext,(*iter).second.c_str());
					_makepath(result,NULL,path,name,ext);

					const QObjectAttributeDataEntry* permFlagEntry = obj.GetOAD().GetEntryByName("Moves Between Rooms");
					if(!permFlagEntry)
						cwarn << "Levelcon Warning: object <"<< obj.GetName() << "> with mesh found with no MovesBetweenRooms flag" << endl;

					if(find(assetVect.begin(),assetVect.end(),result) == assetVect.end())
						assetVect.push_back(result);

					if(mergeObjects && obj.GetTypeIndex() == statplatClassIndex)
					{
						const QObjectAttributeDataEntry* permFlagEntry = obj.GetOAD().GetEntryByName("Moves Between Rooms");
						if(!permFlagEntry)
							cwarn << "Levelcon Warning: object <"<< obj.GetName() << "> with mesh found with no MovesBetweenRooms flag" << endl;
						assert(permFlagEntry);
						if((permFlagEntry && permFlagEntry->GetDef()))
					 	{
							cerror << "statplat <" << obj.GetName() << "> has MovesBetweenRooms set" << endl;
							assert(!permFlagEntry->GetDef());
					 	}

						mergedObject tempMO;
						tempMO.modelName = modelName;
						Euler euler = obj.Rotation();

						tempMO._mat.SetTrans( Point3(obj.Position().x, obj.Position().y, obj.Position().z) );
						assert((rIter - rooms.begin()) < allMergedObjects.size());
						allMergedObjects[rIter - rooms.begin()].push_back(tempMO);
						// ok, now erase model asset entry from object
						QObjectAttributeDataEntry& meshName = (QObjectAttributeDataEntry&)*obj.GetOAD().GetEntryByName("Mesh Name");
                        assert(meshName.GetType() == BUTTON_FILENAME);
						DBSTREAM3( cdebug << "  clearing oad entry " << meshName.GetName() << " in object "  << obj.GetName() << endl; )
						meshName.SetDef(0);
						meshName.SetString("");	// clear the meshname string
						assert(meshName.GetString().length() == 0);
						QObjectAttributeDataEntry& modelType = (QObjectAttributeDataEntry&)*obj.GetOAD().GetEntryByName("Model Type");
						assert(modelType.GetType() == BUTTON_INT32);
						modelType.SetDef(MODEL_TYPE_NONE);
						QObject& modObj = (QObject&)obj;
						modObj.SetModelIndex(-1);				// nuke model index as well
					}
			 	}
		 	}
			// kts added 9/27/96 10:05 to create a merged object for this room
			if(allMergedObjects[rIter - rooms.begin()].size())
			{
				char buffer[12];
				string objectName("MergedObjectForRoom");
				objectName += itoa(rIter - rooms.begin(),buffer,10);
				DBSTREAM3( cdebug << "    Creating object <" << objectName << ">" << endl; )
				Euler newRotation;
				newRotation.a = 0;
				newRotation.b = 0;
				newRotation.c = 0;
				QColBox collision(rIter->GetColBox());

				QObjectAttributeData newobjOAD(objectOADs[mergedClassIndex]);		// make an all default merged object oad

				int32 oadFlags = objectOADs[mergedClassIndex].GetOADFlags();
				string mergedModelName = levelName;
				mergedModelName += "\\mrgrm";
				mergedModelName += itoa((rIter - rooms.begin()),buffer,10);
				mergedModelName += ".3ds";
				DBSTREAM3( cdebug << "    mergedModelName = <" << mergedModelName << ">" << endl; )

				int modelIndex = FindModel(mergedModelName.c_str());				// FindModel will create a new model name if one not found

				const QObjectAttributeDataEntry* constModelType = newobjOAD.GetEntryByName("Model Type");
				assert(constModelType);
				QObjectAttributeDataEntry* modelType = (QObjectAttributeDataEntry*)constModelType;
				modelType->SetDef(MODEL_TYPE_MESH);				// set model type to mesh

				const QObjectAttributeDataEntry* constMeshName = newobjOAD.GetEntryByName("Mesh Name");
				assert(constMeshName);
				QObjectAttributeDataEntry* meshName = (QObjectAttributeDataEntry*) constMeshName;

				meshName->SetString(mergedModelName);			// set mesh name

				objects.push_back
				(
					new QObject
					(
						objectName.c_str(),
						mergedClassIndex,
						Point3(0,0,0),
						Point3(0,0,0),
						newRotation,
						collision,
						oadFlags,
						-1,
						newobjOAD
					)
				);
				DBSTREAM3( cdebug << "adding object to room #" << rIter - rooms.begin() << endl; )
				rIter->Add(objects.back());
			}
			++rIter;
	 	}
		DBSTREAM3( cdebug << "  QLevel::CreateMergedObjects finished" << endl; )
	}
}
#endif

//============================================================================
