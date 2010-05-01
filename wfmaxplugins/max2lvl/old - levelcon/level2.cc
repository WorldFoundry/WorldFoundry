//============================================================================
// level2.cc
//============================================================================
//
// This file is a continuation of level.cc from levelcon; I'm breaking the source file into
// multiple chunks in hopes that Watcom will be able to compile it in debug mode.

//#include "global.hp"
#include "levelcon.hp"
//#include <iostream.h>
#include <fstream.h>
#include "level.hp"
#include "asset.hp"

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//============================================================================

extern boolean ContainsString(vector<asset> data, const char* name);

void StripLeadingPath(char* oldFilename);
string StripLeadingPath(const string& str);

extern packedAssetID
FindAssetID(int roomNum, const string& name);

//============================================================================

extern int32 numPF3s;

void
QLevel::CreateAssetLST(const char* templatename)
{
	DBSTREAM1( cprogress << "  QLevel::CreateAssetLST:" << endl; )

	// Create an empty vector in the allAssets vector for each room
	assert(allAssets.size() == 0);
	for (int numRooms=0; numRooms < rooms.size()+1; numRooms++)
		allAssets.push_back(vector<asset>());

	char drive[_MAX_DRIVE];
 	char path[_MAX_PATH];
 	char name[_MAX_FNAME];
 	char result[_MAX_PATH];

 	_splitpath(templatename,drive,path,name,NULL);
 	_makepath(result,drive,path,"asset",".lst");


	pf3Index = 1;		// running index for PF3 assets
	otherIndex = 1;		// running index for all other asset types

	// Add permanent .tga and .ruv files to list
	strcpy(result, name);
	strcpy(result + strlen(name), "/palperm.tga");
	AssignUniqueAssetID(result, 0xfff);
	strcpy(result + strlen(name), "/perm.tga");
	AssignUniqueAssetID(result, 0xfff);
	strcpy(result + strlen(name), "/perm.ruv");
	AssignUniqueAssetID(result, 0xfff);

	// Wall all objects, and assign permanent IDs to those with MovesBetweenRooms==true.
	for(int objectIndex=0;objectIndex<objects.size();objectIndex++)
	{
		// output all assets for this object
		const QObject* obj = objects[objectIndex];
		const char* thisObjectName = obj->GetName();
		const QObjectAttributeData&  objOad = obj->GetOAD();
		const QObjectAttributeDataEntry* permFlagEntry = objOad.GetEntryByName("Moves Between Rooms");

		if(permFlagEntry && permFlagEntry->GetDef())
		{
			DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: checking object named " << obj->GetName() << endl; )
			for (int oadIndex = 0;oadIndex < objOad.entries.size(); oadIndex++)
			{
				QObjectAttributeDataEntry& oadEntry = (QObjectAttributeDataEntry&)objOad.entries[oadIndex];
				DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: checking entry named " << oadEntry.GetName() << endl; )
				if ( (oadEntry.GetType() == BUTTON_FILENAME) || (oadEntry.GetType() == BUTTON_MESHNAME) )
				{
					DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: if BUTTON_FILENAME or BUTTON_MESHNAME true"  << endl; )
					if (oadEntry.GetString().length() == 0)
					{
						DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: string length is 0"  << endl; )
						const QObjectAttributeDataEntry* modelType = objOad.GetEntryByName("Model Type");
						assert(modelType);
						assert(modelType->GetDef() < MODEL_TYPE_MAX);					// kts check if new model type is handled here
						if (((modelType->GetDef()==MODEL_TYPE_MESH)||(modelType->GetDef()==MODEL_TYPE_SCARECROW)) )
						{
							cerror << "Levelcon Error: required asset " << oadEntry.GetName() << " missing from object " << thisObjectName << ", object type is " << objectTypes[obj->GetTypeIndex()] << endl;
							exit(1);
						}
					}
					else
						AssignUniqueAssetID(oadEntry.GetString().c_str(), 0xfff);
				}
			}
		}
	}


	// Now walk the room object lists and assign non-permanent IDs
	for(int roomIndex = 0; roomIndex < rooms.size();roomIndex++)
	{
		DBSTREAM3( cdebug << "Generating asset IDs for Room " << roomIndex << endl; )

		// Add .tga and .ruv entries for this room
		char roomTGAname[] = "/room\0     ";
		char roomRUVname[] = "/room\0     ";
		char roomPALname[] = "/pal\0      ";
		sprintf(&roomPALname[strlen(roomPALname)], "%d", roomIndex);
		strcat(roomPALname, ".tga");
		strcpy(result + strlen(name), roomPALname);
		AssignUniqueAssetID(result, roomIndex);
		sprintf(&roomTGAname[strlen(roomTGAname)], "%d", roomIndex);
		strcat(roomTGAname, ".tga");
		strcpy(result + strlen(name), roomTGAname);
		AssignUniqueAssetID(result, roomIndex);
		sprintf(&roomRUVname[strlen(roomRUVname)], "%d", roomIndex);
		strcat(roomRUVname, ".ruv");
		strcpy(result + strlen(name), roomRUVname);
		AssignUniqueAssetID(result, roomIndex);

		for(objectIndex = 0;objectIndex < rooms[roomIndex].length(); objectIndex++)
		{
//			DBSTREAM3( cdebug << "  QLevel::SaveAssetLSTFile: checking object named " << rooms[roomIndex].GetObjectByIndex(objectIndex).GetName() << endl; )
			const char* thisObjectName = rooms[roomIndex].GetObjectByIndex(objectIndex).GetName();
			// output all assets for this object
			const QObjectAttributeData&  objOad = rooms[roomIndex].GetObjectByIndex(objectIndex).GetOAD();
			const QObjectAttributeDataEntry* permFlagEntry = objOad.GetEntryByName("Moves Between Rooms");

			if(permFlagEntry && !(permFlagEntry->GetDef()))	// if this is a room obj
			{
				for(int oadIndex = 0;oadIndex < objOad.entries.size(); oadIndex++)
			 	{
					QObjectAttributeDataEntry& oadEntry = (QObjectAttributeDataEntry&)objOad.entries[oadIndex];

					if ( (oadEntry.GetType() == BUTTON_FILENAME) || (oadEntry.GetType() == BUTTON_MESHNAME) )
				 	{
						if (oadEntry.GetString().length() == 0)
						{
							const QObjectAttributeDataEntry* modelType = objOad.GetEntryByName("Model Type");
							assert(modelType);
							if (((modelType->GetDef()==MODEL_TYPE_MESH)||(modelType->GetDef()==MODEL_TYPE_SCARECROW)) )
							{
								cerror << "Levelcon Error: required asset " << oadEntry.GetName() << " missing from object " << rooms[roomIndex].GetObjectByIndex(objectIndex).GetName() << endl;
								exit(1);
							}
						}
						else
							AssignUniqueAssetID(oadEntry.GetString().c_str(), roomIndex);
					}
			 	}
			}
		}
	}
}

//============================================================================

void
QLevel::PatchAssetIDsIntoOADs()
{
	DBSTREAM1( cprogress << "PatchAssetIDsIntoOADs(): Starting...." << endl; )
	for(int roomIndex = 0; roomIndex < rooms.size();roomIndex++)
	{
		for(int objectIndex = 0;objectIndex < rooms[roomIndex].length(); objectIndex++)
		{
			const char* thisObjectName = rooms[roomIndex].GetObjectByIndex(objectIndex).GetName();
			const QObjectAttributeData&  objOad = rooms[roomIndex].GetObjectByIndex(objectIndex).GetOAD();
			const QObjectAttributeDataEntry* permFlagEntry = objOad.GetEntryByName("Moves Between Rooms");
			int roomLookupIndex;

			for(int oadIndex = 0;oadIndex < objOad.entries.size(); oadIndex++)
			{
				QObjectAttributeDataEntry& oadEntry = (QObjectAttributeDataEntry&)objOad.entries[oadIndex];
				if ( ((oadEntry.GetType() == BUTTON_MESHNAME) || (oadEntry.GetType() == BUTTON_FILENAME)) && oadEntry.GetString().length() )
				{
					// Look this asset up in the appropriate room to find the ID
					DBSTREAM3( cdebug << "  looking up assetID for " << oadEntry.GetName() << " with string of " << oadEntry.GetString() << endl; )
					//oadEntry.SetString(StripLeadingPath(oadEntry.GetString()));

					if(permFlagEntry && permFlagEntry->GetDef())	// if this is a perm obj
						roomLookupIndex = 0;
					else
						roomLookupIndex = roomIndex + 1;

					char oldName[_MAX_PATH];
					strcpy(oldName, oadEntry.GetString().c_str());
					char *ext = strstr(oldName, ".3ds");
					if (ext)
					{
						strncpy(ext, ".pf3", 4);
						StripLeadingPath(oldName);
						oadEntry.SetString(string(oldName));
					}

					packedAssetID assetID = FindAssetID(0, oadEntry.GetString());
					if (assetID.ID() == 0)
						assetID = FindAssetID(roomLookupIndex, oadEntry.GetString());

					if (assetID.ID())
					{
						DBSTREAM3( cdebug << "  setting oad entry " << oadEntry.GetName() << " with asset ID of " << hex << assetID.ID() << " in object "  << objects[objectIndex]->GetName() << endl; )
						oadEntry.SetDef(assetID.ID());
					}
					else
						cerror << "Levelcon Error: Can't find assetID for entry <" << oadEntry.GetString() << "> in object <" << thisObjectName << ">. Probable Cause: this object is not in any room and moves between rooms is set to false" << endl;
			    }
			}
		}
	}
}

//============================================================================

asset
QLevel::AssignUniqueAssetID(const char* name, int roomNum)
{
	DBSTREAM3( cdebug << "AssignUniqueAssetID(): Looking up <" << name << ">, in room " << roomNum << endl; )
	packedAssetID packedID;
	packedID.Room(roomNum);

	char writableName[_MAX_PATH];
	strcpy(writableName, name);

	if (roomNum == 0xfff)
		roomNum = 0;		// permanents
	else
		roomNum++;			// rooms

	assert(allAssets.size() >= roomNum);	// Make sure the asset vector has been allocated already

	bool isPF3 = false;
	char *ext = strstr(writableName, ".pf3");
	if (ext)
		isPF3 = true;	// This is already a .pf3
	else
	{
		ext = strstr(writableName, ".3ds");
		if (ext)
		{
			strncpy(ext, ".pf3", 4);
			StripLeadingPath(writableName);
			isPF3 = true;
		}
	}

	// check permanents for this ID first
	packedAssetID existingID = FindAssetID(0, string(writableName));
	if (existingID.ID())
	{
		DBSTREAM3( cdebug << "\tFound it in permanents: " << existingID << endl; )
		return asset(writableName, existingID);
	}

	// Doesn't already exist in permanents, so check the room vector
	existingID = FindAssetID(roomNum, string(writableName));
	if (existingID.ID())
	{
		DBSTREAM3( cdebug << "\tFound it in room " << roomNum << ": " << existingID << endl; )
		return asset(writableName, existingID);
	}

	// No pre-existing ID for this asset, so make one up and add it to the room vector.
	if (isPF3)
		packedID.Index(pf3Index++);
	else
		packedID.Index(otherIndex++);

	packedID.Type(assetExts.Lookup((const char*)strrchr(writableName, '.')));
	DBSTREAM3( cdebug << "\tCreating new ID: " << packedID << endl; )
	allAssets[roomNum].push_back(asset(writableName, packedID));
	return asset(writableName, packedID);
}

//============================================================================

packedAssetID
QLevel::FindAssetID(int roomNum, const string& name)
{
	DBSTREAM3( cdebug << "FindAssetID():  Looking for <" << name << ">..." << endl; )

	for(int assetIndex=0;assetIndex < allAssets[roomNum].size() ;assetIndex++)
	{
		asset foo = allAssets[roomNum][assetIndex];

		DBSTREAM3( cdebug << "\tChecking <" << foo.Name() << ">..." << endl; )
		if(foo.Name() == name)
		{
			DBSTREAM3( cdebug << "\tFound it!" << endl; )
			return(foo.ID());
		}
	}
	DBSTREAM3( cdebug << "\tCouldn't find it!" << endl; )
	return((packedAssetID) 0 );
}

//============================================================================


void
QLevel::SaveAssetLSTFile(const char* templatename)
{
	DBSTREAM1( cprogress << "  QLevel::SaveAssetLSTFile started" << endl; )
	char drive[_MAX_DRIVE];
	char path[_MAX_PATH];
	char name[_MAX_FNAME];
	char result[_MAX_PATH];

	_splitpath(templatename,drive,path,name,NULL);
	_makepath(result,drive,path,"asset",".lst");

	DBSTREAM3( cdebug << " asset.LST filename = " << result << endl; )
	ofstream assetlst(result);
	if(assetlst.rdstate() != ios::goodbit)
	{
		cerror << "Levelcon Error: cannot open file <" << result << "> for writting" << endl;
		exit(5);
	}

	assetlst << "//============================================================================" << endl;
	assetlst << "// asset.lst: asset list, created by LevelCon V" LEVELCON_VER ", DO NOT MODIFY" << endl;
	assetlst << "//============================================================================" << endl;
	assetlst << endl;

	for (int roomIndex = 0; roomIndex < allAssets.size(); roomIndex++)
	{
		vector<asset> roomAssets = allAssets[roomIndex];

		if (roomIndex == 0)
			assetlst << "Permanents:" << endl;
		else
			assetlst << "Room" << roomIndex-1 << ":" << endl;

		assetlst << hex << "\t";
		copy(roomAssets.begin(),roomAssets.end(),ostream_iterator<asset>(assetlst,"\n\t"));
		assetlst << endl;
	}
}