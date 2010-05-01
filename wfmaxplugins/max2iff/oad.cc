// oad.cpp

#include "global.hpp"

#include <string>

#define SYS_INT32
typedef long int32;
typedef short int16;
#include <source/oas/oad.h>

#include "../lib/wf_id.hp"

void* LoadBinaryFile( const char* _szFilename, int& nSize );

typeDescriptor*
GetEntryByName( INode* pNode, const char* szButtonName, int buttonType, const char* szClassName )
{
	static typeDescriptor tdReturn;

	assert( pNode );

	AppDataChunk* adClass = pNode->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_CLASS );
	assert( stricmp( (char*)adClass->data, "handle" ) == 0 );
	AppDataChunk* adOad = pNode->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_OAD );

	int cbHandle;
	oadFile* classHandle;
	classHandle = (oadFile*)LoadBinaryFile( "d:/wfsrc/levels.src/oad/handle.oad", cbHandle );
	assert( classHandle );


	// First, find typedescriptor in .oad file

	typeDescriptor* td = classHandle->types;
	cbHandle -= sizeof( oadHeader );

	while ( cbHandle > 0 )
	{
		if ( ( buttonType == td->type )
		&& ( strcmp( szButtonName, td->name ) == 0 ) )
		{
			tdReturn = *td;
			if ( adOad )
			{	// Check for override within object
				char* xdataStart = (char*)adOad->data;
				char* xdata = xdataStart;

				while ( xdata < (char*)adOad->data + adOad->length )
				{
					char* pFieldStart = (char*)xdata;			// Remember starting place in case we need to skip over all the data

					int size = *((int*)xdata);
					//assert( size > 0 );
					xdata += sizeof( int32 );

					int bt = *((int*)xdata);
					xdata += sizeof( int32 );

					visualRepresentation showAs = *((int*)xdata);
					xdata += sizeof( int32 );

					const char* szFieldName = (const char*)xdata;
					xdata += strlen( szFieldName ) + 1;

					if ( strcmp( szButtonName, szFieldName ) == 0 )
					{
						tdReturn.def = atol( xdata );
						return &tdReturn;
					}

					xdata = pFieldStart + size;
				}
			}

			return &tdReturn;
		}
		cbHandle -= sizeof( typeDescriptor );
		//++td;
		td = (typeDescriptor*)( (char*)td + sizeof( typeDescriptor ) );
	}
	assert( cbHandle == 0 );

	return NULL;
}
