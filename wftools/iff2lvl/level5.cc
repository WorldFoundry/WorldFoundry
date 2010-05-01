//============================================================================
// level5.cc
//============================================================================
// This file is a continuation of level.cc from levelcon; I'm breaking the source file into
// multiple chunks in hopes that Watcom will be able to compile it in debug mode.

#include "global.hp"
//#include <iostream.h>
//#include <fstream.h>
//#include <iomanip.h>
#include "levelcon.hp"
#include "level.hp"
#include "asset.hp"
//#include <stl/algo.h>
#include "hdump.hp"
#include "object.hp"
#include "path.hp"


//============================================================================

//extern Interface* gMaxInterface;

//============================================================================

bool
QLevel::ValidateVelocityLevel(void) const		// Do Velocity-specific consistancy checks on the level
{
	bool levelOK = true;

	for (unsigned int index=0; index < objects.size(); index++)
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

	return levelOK;
}

//============================================================================

void
QLevel::PrintNameList( ostream& out) const
{
	char nameBuffer[80];

	out << ";//============================================================================" << endl;
	out << ";// Object Index to Name list, created by LevelCon V"LEVELCON_VER", DO NOT MODIFY" << endl;
	out << ";// Object Index    Name" << endl;
	out << ";//============================================================================" << endl;

	for(unsigned int index=0;index<objects.size();index++)
	{
		strcpy(nameBuffer, objects[index]->GetName());
		while ( strchr(nameBuffer, ' ') )
			*strchr(nameBuffer, ' ') = '-';

		out << "( define " << setw(11) << nameBuffer << " " << index << " )" << endl;
	}

	out << ";//============================================================================" << endl;
}

//============================================================================

void
QLevel::Save(const char* name)
{
	ValidateVelocityLevel();
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

bool
ContainsString(vector<asset> data, const char* name)
{
	for(unsigned int assetIndex=0;assetIndex < data.size() ;assetIndex++)
	 {
//		if(!strcmp(data[assetIndex]._name.c_str(),name))
		if(data[assetIndex].Name() == name)
		 {
		 	DBSTREAM3( cdebug << "  QLevel::SaveAssetLSTFile: found " << endl; )
			return(true);
		 }
	 }
	return(false);
}

//============================================================================

void
QLevel::StripLeadingPath(char* oldFilename)
{

  //	strlwr(oldFilename);

	// kts special case for mh3d which allows any sub-directory under mh3d (but prevents level specific sub-directories
	if(
		strstr(oldFilename,"levels/mh3d/") ||
		strstr(oldFilename,"levels\\mh3d\\")
	)
	{
		char name[_MAX_PATH];
		char ext[_MAX_PATH];
		_splitpath(oldFilename,NULL,NULL,name,ext);
		_makepath(oldFilename,NULL,NULL,name,ext);
	}

	for(vector<string>::iterator iter(meshDirectories.begin());iter != meshDirectories.end();iter++)
		if(strstr(oldFilename,(*iter).c_str()))
		{
			strcpy(oldFilename, oldFilename + (*iter).length());
		}

}

//============================================================================

#if 0
string StripLeadingPath(const string& str)
{
	char oldFilename[_MAX_PATH];

	AssertMessageBox( str.length() < _MAX_PATH, "Path too Long: " << str );
	strcpy(oldFilename,str.c_str());

	strlwn(oldFilename);
	for(map<string,string,less<string> >::iterator iter(meshDirectories.begin());iter != meshDirectories.end();iter++)
		if(ext = strstr(oldFilename,(*iter).first.c_str()))
		{
			strcpy(oldFilename, oldFilename + (*iter).first.length());
		}
	return(string(oldFilename));
}
#endif

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
				//	cerror
				//		<< "Levelcon ERROR: Path hierarchy which loops!"
				//		<< endl
				//		<< "\t\t";
				//	vector<string>::const_iterator list = objList.begin();
				//
				//	// output the hierarchy (with stars around the problem)
				//	while (list != objList.end())
				//	{
				//		if (list == where)
				//			cerror << "*" << *list << "* -> ";
				//		else
				//			cerror << "[" << *list << "] -> ";
				//		list++;
				//	}
				//	cerror << "*" << str << "*" << endl;
				//	assert(0);

					stringstream errorStream;
					errorStream
						<< "Path hierarchy loops!"
						<< endl
						<< "\t\t";
					std::vector<std::string>::const_iterator list = objList.begin();

					// output the hierarchy (with stars around the problem)
					while (list != objList.end())
					{
						if (list == where)
							errorStream << "*" << *list << "* -> ";
						else
							errorStream << "[" << *list << "] -> ";
						list++;
					}
					errorStream << "*" << str << "*" << std::endl;
					AssertMessageBox(0, errorStream.str());
				}

				// get next object in chain
				int indexTarget = FindObjectIndex(str);
				assert(indexTarget < GetObjectCount());
				AssertMessageBox(indexTarget >= 0, "indexTarget = " << indexTarget << ", string = " << str << ", followObjectEntry = " << *followObjectEntry);
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
						const Point3& vecTarget = objTarget.Position();

						// get this object
						QObject objSource = GetObject(indexObject);

						// get this path

						int thePathIndex = objSource.GetPathIndex();

						assert(objSource.GetPathIndex() < (int)paths.size());
						AssertMessageBox(objSource.GetPathIndex() >= 0, "Object <" << objSource.GetName() << "> wants a path!");
						QPath oPath = paths[(int)objSource.GetPathIndex()];

						// get first point in path (position at time=0)
						const Point3& vecSource = oPath.GetPosition(0);

						// calculate difference in position (offset)
						const Point3 vecDelta(vecTarget - vecSource);

						// add delta to all points in path
						oPath.AddPositionOffset(vecDelta);
					}
				}
			}
		}
	}
	DBSTREAM2( cdebug << "QLevel::ResolvePathOffsets: done" << endl; )
}

//============================================================================
