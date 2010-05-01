//============================================================================
// asset.cc:
// Copyright(c) 1996 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			class containing array of assets
	History:
			Created	6/19/96 11:51AM Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, STL, iostream
	Restrictions:

	Example:
*/
//============================================================================

#include "global.hpp"

#include "asset.hpp"

//============================================================================

extern Interface* gMaxInterface;

//============================================================================

//============================================================================

void
AssetExtensions::Load(istream& in)
{
	DBSTREAM3( cdebug << "AssetExtensions::Load: " << endl; )
	char stringBuffer[100];

	while(in.good())
	 {
		in.getline(stringBuffer,100);
		strlwr(stringBuffer);

		if(strlen(stringBuffer) != 0 && stringBuffer[strlen(stringBuffer)-1] == '\n')
			stringBuffer[strlen(stringBuffer)-1] = 0;
		if(strlen(stringBuffer) != 0 && (stringBuffer[0] != '/' && stringBuffer[1] != '/'))
		 {
			// only read until whitespace
			stringBuffer[ strcspn(stringBuffer, " \t") ] = 0;
			DBSTREAM3( cdebug << "AssetExtensions::Load: parsing line <" << stringBuffer << ">" << endl; )
//			assert(Lookup(stringBuffer) == -1);				// insure it is not already in the list
			extensions.push_back(stringBuffer);
		 }
	 }
	DBSTREAM3( cdebug << "AssetExtensions::Load: list:" << endl; )
	DBSTREAM3( Save(cdebug); )
}

//============================================================================

void
AssetExtensions::Save(ostream& out)
{
	copy(extensions.begin(),extensions.end(),ostream_iterator<string>(out,"\n"));
}

//============================================================================

int
AssetExtensions::Entries()
{
	return extensions.size();
}

//============================================================================

const string&
AssetExtensions::operator[](int index)
{
	assert(index >= 0);
	assert(index < Entries());
	return(extensions[index]);
}

//============================================================================

int
AssetExtensions::Lookup(const string& inputExtension)
{
	char temp[10];
	strcpy(temp, inputExtension.c_str());
	strlwr(temp);
	string extension(temp);

	DBSTREAM3( cdebug << "AssetExtensions::Lookup: string = <" << extension << ">" << endl; )

	for(unsigned int index=0;index<extensions.size();index++)			// kts does STL have a better way to do this?
	 {
		DBSTREAM3( cdebug << "AssetExtensions::Lookup: checking index " << index << " which contains " << extensions[index] << endl; )
		if(extensions[index] == extension)
		 {
			DBSTREAM3( cdebug << "found, index = " << index << endl; )
			return(index);
		 }
	 }
	AssertMessageBox(0,"AssetExtensions::Lookup: failed, looked for extension " << extension );
	return(-1);
}

//============================================================================
