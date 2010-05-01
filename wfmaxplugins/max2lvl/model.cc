//============================================================================
// model.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of level model data
	History:
			Created	05-04-95 06:30pm Kevin T. Seghetti

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

#include "pigtool.h"
#include "model.hpp"
#include "levelcon.hpp"
#include <game/levelcon.h>		// included from velocity\source


//============================================================================

QModel::QModel(const string& newName)
{
	good = true;
//	assert(strlen(newName) < NAME_LEN-1);

	// WBN (3 May 96) -- Converts filename to appropriate place in asset tree
	char convertedName[ 256 ];
	strcpy( convertedName, newName.c_str() );

	static char sz3dsDir[] = "levels\\3ds\\";
	static char szAliasDir[] = "levels\\alias\\";

	if ( strnicmp( newName.c_str(), sz3dsDir, strlen( sz3dsDir ) ) == 0 )
		strcpy(convertedName, newName.c_str() + strlen( sz3dsDir ) );
	else if ( strnicmp( newName.c_str(), szAliasDir, strlen( szAliasDir ) ) == 0 )
		strcpy(convertedName, newName.c_str() + strlen( szAliasDir ) );

//	cout << "newName = " << newName << " convertedName = " << convertedName << endl;
	name = convertedName;

//	strcpy(name,newName);
}

//============================================================================

QModel::~QModel()
{
}

//============================================================================

void
QModel::Save(ostream& lvl)
{
	assert(good);
	_ModelOnDisk diskModel;

	for(int i=0;i<MODELONDISK_MAX;i++)					// insure rest of struct is zeros
		diskModel.name[i] = 0;
	assert(name.length() < MODELONDISK_MAX);
	strcpy(diskModel.name,name.c_str());
//	fwrite(&diskModel,sizeof(_ModelOnDisk),1,fp);
	lvl.write( (const char*)&diskModel, sizeof(_ModelOnDisk) );
}

//============================================================================

ostream& operator<<(ostream& s, const QModel &model)
{
	assert(model.good);

	s << model.name;
	return(s);
}

//============================================================================
