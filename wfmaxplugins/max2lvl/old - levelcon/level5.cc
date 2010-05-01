//============================================================================
// level5.cc
//============================================================================
// This file is a continuation of level.cc from levelcon; I'm breaking the source file into
// multiple chunks in hopes that Watcom will be able to compile it in debug mode.

#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include "levelcon.hp"
#include "level.hp"
#include "asset.hp"
#include <stl/algo.h>
#include <pclib/hdump.hp>

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//============================================================================


//============================================================================

bool
QLevel::ValidateVelocityLevel(void) const		// Do Velocity-specific consistancy checks on the level
{
	bool levelOK = true;

	for (int index=0; index < objects.size(); index++)
	{
		// Check that pathed objects don't have Mobility=0
		QObjectAttributeData thisOAD = objects[index]->GetOAD();
		const QObjectAttributeDataEntry* mobilityEntryPtr = thisOAD.GetEntryByName("Mobility");
		if (mobilityEntryPtr)
		{
			if ( (mobilityEntryPtr->GetDef() == 0) && (objects[index]->GetPathIndex() != -1) )
			{
				cerror << "Levelcon Error:  Object <" << objects[index]->GetName() << "> is both pathed and anchored." << endl;
				levelOK = false;
			}
		}

		// Insert additional checks here....

	}

//	assert(levelOK);
	return levelOK;
}

//============================================================================

void
QLevel::PrintNameList( ostream& out) const
{
	out << ";//============================================================================" << endl;
	out << ";// Object Index to Name list, created by LevelCon V" LEVELCON_VER ", DO NOT MODIFY" << endl;
	out << ";// Object Index    Name" << endl;
	out << ";//============================================================================" << endl;
	for(int index=0;index<objects.size();index++)
	 {
//		out << setw(4) << index << "            " << objects[index]->GetName() << endl;
//		out << setw(11) << objects[index]->GetName() << " = " << index << endl;
		out << "( define " << setw(11) << objects[index]->GetName() << " " << index << " )" << endl;
	 }
	out << ";//============================================================================" << endl;
}

//============================================================================

void
QLevel::Save(const char* name)
{
	ValidateVelocityLevel();
	SaveINIFile(name);
	SaveAssetLSTFile(name);
	SaveLVLFile(name);
}

//============================================================================
// break up string into sub-strings based on seperator

int
parseStringIntoParts(  const char* _string, char* _xDataParameters[], int nMaxParameters )
	{
	enum { SEPERATOR = '|' };
	int _nItems = 0;

	char* strBegin = (char*)_string;
	char* strSeperator;

	// null string array so we know which ones to free later
	for ( int paramIndex=0; paramIndex < nMaxParameters; ++paramIndex )
	 {
		_xDataParameters[ paramIndex ] = NULL;
	 }

	do
		{
		int len;

		strSeperator = strchr( strBegin, SEPERATOR );

		if ( strSeperator )
			len = strSeperator - strBegin;
		else
			{ // Take to end of string
			len = strlen( strBegin );
			}

		_xDataParameters[ _nItems ] = (char*)malloc( len + 1 );
		assert( _xDataParameters[ _nItems ] );
		strncpy( _xDataParameters[ _nItems ], strBegin, len );
		*( _xDataParameters[ _nItems ] + len ) = '\0';
		//debug( "_xDataParameters[%d]: [%s]", _nItems, _xDataParameters[_nItems] );

		++_nItems;
		assert(_nItems < nMaxParameters);

		strBegin = strSeperator + 1;
		}
	while ( strSeperator );

	assert( _nItems > 0 );

	return _nItems;
	}

//============================================================================
// kts I can't figure out how to do this with find

boolean
ContainsString(vector<asset> data, const char* name)
{
	for(int assetIndex=0;assetIndex < data.size() ;assetIndex++)
	 {
//		if(!strcmp(data[assetIndex]._name.c_str(),name))
		if(data[assetIndex].Name() == name)
		 {
		 	cdebug << "  QLevel::SaveAssetLSTFile: found " << endl;
			return(boolean::BOOLTRUE);
		 }
	 }
	return(boolean::BOOLFALSE);
}

//============================================================================

void StripLeadingPath(char* oldFilename)
{
	if (strstr(oldFilename, "levels\\3ds\\"))
		strcpy(oldFilename, oldFilename + strlen("levels\\3ds\\"));

	else if (strstr(oldFilename, "levels/3ds/"))
		strcpy(oldFilename, oldFilename + strlen("levels/3ds/"));

	else if (strstr(oldFilename, "levels\\alias\\"))
		strcpy(oldFilename, oldFilename + strlen("levels\\alias\\"));

	else if (strstr(oldFilename, "levels/alias/"))
		strcpy(oldFilename, oldFilename + strlen("levels/alias/"));
}

//============================================================================

string StripLeadingPath(const string& str)
{
	char oldFilename[_MAX_PATH];

	assert(str.length() < _MAX_PATH);
	strcpy(oldFilename,str.c_str());

	if (strstr(oldFilename, "levels\\3ds\\"))
		strcpy(oldFilename, oldFilename + strlen("levels\\3ds\\"));

	else if (strstr(oldFilename, "levels/3ds/"))
		strcpy(oldFilename, oldFilename + strlen("levels/3ds/"));

	else if (strstr(oldFilename, "levels\\alias\\"))
		strcpy(oldFilename, oldFilename + strlen("levels\\alias\\"));

	else if (strstr(oldFilename, "levels/alias/"))
		strcpy(oldFilename, oldFilename + strlen("levels/alias/"));

	return(string(oldFilename));
}

//============================================================================

void
QLevel::ResolvePathOffsets(void) const
{
	DBSTREAM1( cprogress << "QLevel::ResolvePathOffsets" << endl; )
	// tweak hierarchical/relative paths
	for(int indexObject=0;indexObject<GetObjectCount();indexObject++)
	{
		// get this object's "Object To Follow" field
		const QObjectAttributeData&  objOad =
									GetObject(indexObject).GetOAD();
		const QObjectAttributeDataEntry* followObjectEntry =
									objOad.GetEntryByName("Object To Follow");
		if (followObjectEntry != NULL) // if field exists
		{
			// check for recursive hierarchy
			{
			vector<string> objList;

			// start list with name of current object
			objList.push_back(GetObject(indexObject).GetName());
			assert(objList.size() == 1);

			// check for object reference fo follow
			string str = followObjectEntry->GetString();
			while (str.length())	// string not empty
			{
				// get position of string in list
				const vector<string>::const_iterator where
					= find(objList.begin(), objList.end(), str);
				if( where == objList.end() ) // not already in list
				{
					objList.push_back(str);	// add to list for next search
				}
				else						// already in list, TROUBLE!
				{
					cerror
						<< "Levelcon ERROR: Path hierarchy which loops!"
						<< endl
						<< "\t\t";
					vector<string>::const_iterator list = objList.begin();

					// output the hierarchy (with stars around the problem)
					while (list != objList.end())
					{
						if (list == where)
							cerror << "*" << *list << "* -> ";
						else
							cerror << "[" << *list << "] -> ";
						list++;
					}
					cerror << "*" << str << "*" << endl;
					exit(1);
				}

				// get next object in chain
				int indexTarget = FindObjectIndex(str);
				assert(indexTarget < GetObjectCount());
				AssertMsg(indexTarget >= 0, "indexTarget = " << indexTarget << ", string = " << str << ", followObjectEntry = " << *followObjectEntry);
				QObject objTarget = GetObject(indexTarget);

				// look for another object reference
				// get new object's "Object To Follow" field
				const QObjectAttributeData&  objOad =
									objTarget.GetOAD();
				const QObjectAttributeDataEntry* followObjectEntry =
									objOad.GetEntryByName("Object To Follow");

				if (followObjectEntry != NULL)	// if field exists
					str = followObjectEntry->GetString(); // get contents
				else
					str = "";	// else set termination condition
			}
			}

			// Now, get delta from "offset" object (target)
			if (followObjectEntry->GetString().length())	// if string not empty
			{
				const QObjectAttributeDataEntry* offsetObjectEntry =
									objOad.GetEntryByName("Follow Offset");
				if (offsetObjectEntry != NULL ) // if field exists
				{
					const string nameStr(offsetObjectEntry->GetString());
 					if (nameStr.length() > 0)	// if field has a reference
					{
						// get target object
						int indexTarget = FindObjectIndex(nameStr);
						assert(indexTarget < GetObjectCount());
						assert(indexTarget >= 0);
						QObject objTarget = GetObject(indexTarget);

						// get position of target
						const QPoint& vecTarget(objTarget.Position());

						// get this object
						QObject objSource = GetObject(indexObject);

						// get this path, !cast away const!
						QPath& oPath = (QPath&)paths[objSource.GetPathIndex()];

						// get first point in path
						const QPoint& vecSource(oPath.GetPoint(0));

						// calculate difference in position (offset)
						const QPoint vecDelta(vecTarget - vecSource);

						// add delta to all points in path
						for (int i = 0; i < oPath.Size(); i++)
						{
							oPath.SetPoint(vecDelta + oPath.GetPoint(i), i);
						}
					}
				}
			}
		}
	}
	DBSTREAM2( cdebug << "QLevel::ResolvePathOffsets: done" << endl; )
}

//============================================================================