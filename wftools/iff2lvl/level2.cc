//============================================================================
// level2.cc
//============================================================================
//
// This file is a continuation of level.cc from levelcon; I'm breaking the source file into
// multiple chunks in hopes that Watcom will be able to compile it in debug mode.

#include "levelcon.hp"
//#include <fstream.h>
#include "level.hp"
#include "asset.hp"
//#include <stl/iterator.h>

//============================================================================

//extern Interface* gMaxInterface;

//============================================================================

extern bool ContainsString(vector<asset> data, const char* name);

void StripLeadingPath(char* oldFilename);
string StripLeadingPath(const string& str);

extern packedAssetID
FindAssetID(int roomNum, const string& name);

//============================================================================

void
QLevel::CreateAssetLST(const char* templatename)
{
	DBSTREAM1( cprogress << "  QLevel::CreateAssetLST:" << endl; )

	// Create an empty vector in the allAssets vector for each room
	assert(allAssets.size() == 0);
   unsigned int numRooms;
	for (numRooms=0; numRooms < rooms.size()+1; numRooms++)
		allAssets.push_back(vector<asset>());

	// create an empty vector in the assetIndices vector for each room
	assert(assetIndices.size() == 0);
	for (numRooms=0; numRooms < rooms.size()+1; numRooms++)
		assetIndices.push_back(vector<int>());

	char drive[_MAX_PATH];           // yea, wastefull but portable
 	char path[_MAX_PATH];
 	char name[_MAX_PATH];
 	char result[_MAX_PATH];

 	_splitpath(templatename,drive,path,name,NULL);
 	_makepath(result,drive,path,"asset",".lst");

	assert(assetExts.Entries() > 0);
   unsigned int roomIndex;
	for(roomIndex = 0; roomIndex < rooms.size()+1;roomIndex++)
	{
		for(unsigned int assetType=0;assetType < assetExts.Entries();assetType++)
		{
			assetIndices[roomIndex].push_back(1);
		}
		assert(assetExts.Entries() == assetIndices[roomIndex].size());
	}


	// Add permanent .tga and .ruv files to list
//	AssignUniqueAssetID( "palperm.tga", 0xfff );
//	AssignUniqueAssetID( "perm.tga", 0xfff );
//	AssignUniqueAssetID( "perm.ruv", 0xfff );
//	AssignUniqueAssetID( "perm.cyc", 0xFFF );

	// Walk all objects, and assign permanent IDs to those with MovesBetweenRooms==true.
   unsigned int objectIndex;
	for(objectIndex=0;objectIndex<objects.size();objectIndex++)
	{

		// output all assets for this object
		const QObject* obj = objects[objectIndex];
		DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: checking object[" << objectIndex << "] :" << *obj << endl; )
		const char* thisObjectName = obj->GetName();
		const QObjectAttributeData&  objOad = obj->GetOAD();
		const QObjectAttributeDataEntry* permFlagEntry = objOad.GetEntryByName("Moves Between Rooms");

		// If this object has no MovesBetweenRooms entry, pretend it's permanent (levelobj, etc)
		if( (permFlagEntry && permFlagEntry->GetDef()) || (permFlagEntry == NULL) )
		{
			DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: checking object named " << obj->GetName() << endl; )
			for (unsigned int oadIndex = 0;oadIndex < objOad.entries.size(); oadIndex++)
			{
				QObjectAttributeDataEntry& oadEntry = (QObjectAttributeDataEntry&)objOad.entries[oadIndex];
				DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: checking entry named " << oadEntry.GetName() << endl; )
                if (
                    (oadEntry.GetType() == BUTTON_FILENAME) ||
                    (oadEntry.GetType() == BUTTON_STRING) 
                   )
				{
                    if(oadEntry.GetType() == BUTTON_STRING)
                        assert(oadEntry.GetShowAs() == SHOW_AS_FILENAME);
                    DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: if BUTTON_FILENAME true"  << endl; )
					if (oadEntry.GetString().length() == 0)
					{
						DBSTREAM3( cdebug << "  QLevel::CreateAssetLST: string length is 0"  << endl; )
						const QObjectAttributeDataEntry* modelType = objOad.GetEntryByName("Model Type");
						if (modelType != NULL)
						{
							assert(modelType->GetDef() < MODEL_TYPE_MAX);					// kts check if new model type is handled here
							if (oadEntry.GetName() == "Mesh Name" && ((modelType->GetDef()==MODEL_TYPE_MESH)||(modelType->GetDef()==MODEL_TYPE_SCARECROW) ) )
							{
								AssertMessageBox(0, "Required asset " << oadEntry.GetName() << " missing from object " << thisObjectName << ", object type is " << objectTypes[obj->GetTypeIndex()] << "oadEntry = " << oadEntry);
							}
						}
					}
					else
					{
							AssignUniqueAssetID( oadEntry.GetString().c_str(), 0xFFF );
					}
				}
			}
		}
	}


	// Now walk the room object lists and assign non-permanent IDs
	for(roomIndex = 0; roomIndex < rooms.size();roomIndex++)
	{
		DBSTREAM3( cdebug << "Generating asset IDs for Room " << roomIndex << endl; )

		// Add .tga and .ruv entries for this room
//		sprintf( result, "pal%d.tga", roomIndex );
//		AssignUniqueAssetID(result, roomIndex);
//
//		sprintf( result, "room%d.tga", roomIndex );
//		AssignUniqueAssetID(result, roomIndex);
//
//		sprintf( result, "room%d.ruv", roomIndex );
//		AssignUniqueAssetID(result, roomIndex);
//
//		sprintf( result, "room%d.cyc", roomIndex );
//		AssignUniqueAssetID(result, roomIndex);

		for(objectIndex = 0;objectIndex < rooms[roomIndex].length(); objectIndex++)
		{
//			DBSTREAM3( cdebug << "  QLevel::SaveAssetLSTFile: checking object named " << rooms[roomIndex].GetObjectByIndex(objectIndex).GetName() << endl; )
			const char* thisObjectName = rooms[roomIndex].GetObjectByIndex(objectIndex).GetName();
			// output all assets for this object
			const QObjectAttributeData&  objOad = rooms[roomIndex].GetObjectByIndex(objectIndex).GetOAD();
			const QObjectAttributeDataEntry* permFlagEntry = objOad.GetEntryByName("Moves Between Rooms");

			if(permFlagEntry && !(permFlagEntry->GetDef()))	// if this is a room obj
			{
				for(unsigned int oadIndex = 0;oadIndex < objOad.entries.size(); oadIndex++)
			 	{
					QObjectAttributeDataEntry& oadEntry = (QObjectAttributeDataEntry&)objOad.entries[oadIndex];

                    if (
                        (oadEntry.GetType() == BUTTON_FILENAME) ||
                        (oadEntry.GetType() == BUTTON_STRING) 
                       )
                    {
                        if(oadEntry.GetType() == BUTTON_STRING)
                            assert(oadEntry.GetShowAs() == SHOW_AS_FILENAME);
						if (oadEntry.GetString().length() == 0)
						{
							const QObjectAttributeDataEntry* modelType = objOad.GetEntryByName("Model Type");
							assert(modelType);
							if (oadEntry.GetName() == "Mesh Name" && ((modelType->GetDef()==MODEL_TYPE_MESH)||(modelType->GetDef()==MODEL_TYPE_SCARECROW)) )
							{
								AssertMessageBox(0, "Required asset " << oadEntry.GetName() << " missing from object " << rooms[roomIndex].GetObjectByIndex(objectIndex).GetName());
							}
						}
						else
						{
							AssignUniqueAssetID(oadEntry.GetString().c_str(), roomIndex);
						}
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
	for(unsigned int roomIndex = 0; roomIndex < rooms.size();roomIndex++)
	{
		for(int objectIndex = 0;objectIndex < rooms[roomIndex].length(); objectIndex++)
		{
			const char* thisObjectName = rooms[roomIndex].GetObjectByIndex(objectIndex).GetName();
			const QObjectAttributeData&  objOad = rooms[roomIndex].GetObjectByIndex(objectIndex).GetOAD();
			const QObjectAttributeDataEntry* permFlagEntry = objOad.GetEntryByName("Moves Between Rooms");
			int roomLookupIndex;

			for(unsigned int oadIndex = 0;oadIndex < objOad.entries.size(); oadIndex++)
			{
				QObjectAttributeDataEntry& oadEntry = (QObjectAttributeDataEntry&)objOad.entries[oadIndex];
                if ( 
                    ((oadEntry.GetType() == BUTTON_FILENAME) && oadEntry.GetString().length()) ||
                    ((oadEntry.GetType() == BUTTON_STRING) && oadEntry.GetString().length())
                   )
				{
                    if(oadEntry.GetType() == BUTTON_STRING)
                        assert(oadEntry.GetShowAs() == SHOW_AS_FILENAME);

					// Look this asset up in the appropriate room to find the ID
					DBSTREAM3( cdebug << "  looking up assetID for " << oadEntry.GetName() << " with string of " << oadEntry.GetString() << endl; )
					//oadEntry.SetString(StripLeadingPath(oadEntry.GetString()));

					if(permFlagEntry && permFlagEntry->GetDef())	// if this is a perm obj
						roomLookupIndex = 0;
					else
						roomLookupIndex = roomIndex + 1;

					char oldName[_MAX_PATH];
					strcpy(oldName, oadEntry.GetString().c_str());

               // kts this strlwr needs to go away, but first the levels in wflevels need to be updated to 
               // have the proper case
					strlwr(oldName);

					char* ext;
					for(map<string,string,less<string> >::iterator iter(extensionsMap.begin());iter != extensionsMap.end();iter++)
						if((ext = strstr(oldName,(*iter).first.c_str())) != 0 )
						{
							strncpy(ext, (*iter).second.c_str(), 4);
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
QLevel::AssignUniqueAssetID(const char* name, unsigned int roomNum)
{
	DBSTREAM3( cdebug << "AssignUniqueAssetID(): Looking up <" << name << ">, in room " << roomNum << endl; )
	int32 type;
	int32 index;
	int originalRoomNum = roomNum;		// packed asset id DOESN'T have room # adjusted, so preseve for new id code, below		

	char writableName[_MAX_PATH];
	strcpy(writableName, name);

	if (roomNum == 0xfff)
		roomNum = 0;		// permanents
	else
		roomNum++;			// rooms

	DBSTREAM3( cdebug << "AssignUniqueAssetID(): room # converted to " << roomNum << endl; )

	assert(allAssets.size() >= roomNum);	// Make sure the asset vector has been allocated already

	char* ext;
   // kts this strlwr needs to go away, but first the levels in wflevels need to be updated to 
   // have the proper case
	strlwr(writableName);
	for(map<string,string,less<string> >::iterator iter(extensionsMap.begin());iter != extensionsMap.end();iter++)
	{
		ext = strstr(writableName,(*iter).first.c_str());
		if(ext)
		{
			strncpy(ext,(*iter).second.c_str(),4);
			StripLeadingPath(writableName);
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
//	AssertMsg(strrchr(writableName,'.'),"writableName = " << writableName);
	char extension[_MAX_PATH];
	_splitpath(writableName,NULL,NULL,NULL,extension);
	assert(extension[0] == '.');
//	packedID.Type(assetExts.Lookup((const char*)strrchr(writableName, '.')));
	type = assetExts.Lookup(string(extension));

	index = assetIndices[roomNum][type];
	assetIndices[roomNum][type] = assetIndices[roomNum][type]+1;

	packedAssetID packedID(originalRoomNum,type,index);
	DBSTREAM3( cdebug << "\tCreating new ID: " << packedID << endl; )
	allAssets[roomNum].push_back(asset(writableName, packedID));
	return asset(writableName, packedID);
}

//============================================================================

packedAssetID
QLevel::FindAssetID(int roomNum, const string& name)
{
	DBSTREAM3( cdebug << "FindAssetID():  Looking for <" << name << "> in room " << roomNum << "..." << endl; )

	for(unsigned int assetIndex=0;assetIndex < allAssets[roomNum].size() ;assetIndex++)
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
	char drive[_MAX_PATH];
	char path[_MAX_PATH];
	char name[_MAX_PATH];
	char result[_MAX_PATH];

	_splitpath(templatename,drive,path,name,NULL);
	_makepath(result,drive,path,"asset",".inc");

	DBSTREAM3( cdebug << " asset.inc filename = " << result << endl; )
	ofstream assetlst(result);
	AssertMessageBox(assetlst.good(), "Cannot open file <" << result << "> for write");

	assetlst << "@*============================================================================" << endl;
	assetlst << "@* asset.inc: asset list for use with prep, created by LevelCon V"LEVELCON_VER", DO NOT MODIFY" << endl;
	assetlst << "@*============================================================================" << endl;
	assetlst << "FILE_HEADER(" << name << "," << allAssets.size() << ","LEVELCON_VER")" << endl;

	for (unsigned int roomIndex = 0; roomIndex < allAssets.size(); roomIndex++)
	{
		vector<asset> roomAssets = allAssets[roomIndex];
		if (roomIndex == 0)
			assetlst << "ROOM_START(Permanents,PERM)" << endl;
		else
			assetlst << "ROOM_START(Room" << roomIndex-1 << ",RM" << roomIndex-1 << ")" << endl;


		vector<asset>::const_iterator iter = roomAssets.begin();
		while(iter < roomAssets.end())
			assetlst << hex << "\tROOM_ENTRY(" << *iter++ << ")\n";
//		copy(roomAssets.begin(),roomAssets.end(),ostream_iterator<asset>(assetlst,")\n\tROOM_ENTRY("));
		assetlst << "ROOM_END" << endl;
	}
		assetlst << "FILE_FOOTER" << endl;
}

#if 0
// kts old copy

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
	AssertMessageBox(assetlst.good(), "Cannot open file <" << result << "> for write");

	assetlst << "//============================================================================" << endl;
	assetlst << "// asset.lst: asset list, created by LevelCon V"LEVELCON_VER", DO NOT MODIFY" << endl;
	assetlst << "//============================================================================" << endl;
	assetlst << endl;

	for (unsigned int roomIndex = 0; roomIndex < allAssets.size(); roomIndex++)
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
#endif
