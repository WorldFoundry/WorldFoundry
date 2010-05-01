#include <stdarg.h>
#include <windowsx.h>
#include <xstddef>
#include <algorithm>
#include <vector>
#include <string>
#include "../lib/registry.h"
#include "select.h"
#include <io.h>
#include <oas/oad.h>

const char szUndefined[] = "";

extern void* LoadBinaryFile( const char* _szFilename, int& nSize );

////////////////////////////////////////////////////////////////////////////////

Oad::Oad( const char* szClassName )
{
	assert( szClassName );
	_szClassName = _strdup( szClassName );
	assert( _szClassName );
	_strlwr( _szClassName );

	char szFilename[ _MAX_PATH ];
	sprintf( szFilename, "%s/%s", theSelect.szOadDir, szClassName );

	int cbFile;
	_oad = LoadBinaryFile( szFilename, cbFile );
	assert( _oad );

	// Parse file, adding
	typeDescriptor* td = (typeDescriptor*)(((char*)_oad)+sizeof( oadHeader ) );
	assert( td );

	int size = sizeof( oadHeader );

	int len = cbFile;

	typeDescriptor* pEndOfTypeDescriptors = (typeDescriptor*)( ((char*)td) + len );
	for ( ; size<cbFile; size += sizeof( typeDescriptor ), ++td )
	{
		if ( !*td->xdata.displayName )
			continue;

		switch ( td->type )
		{
			case BUTTON_PROPERTY_SHEET:
			case LEVELCONFLAG_NOINSTANCES:
			case LEVELCONFLAG_NOMESH:
			case LEVELCONFLAG_SINGLEINSTANCE:
			case LEVELCONFLAG_TEMPLATE:
			case LEVELCONFLAG_EXTRACTCAMERA:
			case LEVELCONFLAG_ROOM:
			case LEVELCONFLAG_COMMONBLOCK:
			case LEVELCONFLAG_ENDCOMMON:
			case BUTTON_EXTRACT_CAMERA:
			case LEVELCONFLAG_EXTRACTCAMERANEW:
			case BUTTON_WAVEFORM:
			case BUTTON_GROUP_START:
			case BUTTON_GROUP_STOP:
			case LEVELCONFLAG_EXTRACTLIGHT:
				continue;
		}

		std::vector< string >::iterator str;
		for ( str=theSelect._szSelectDisplayName.begin();
		str != theSelect._szSelectDisplayName.end();
		++str )
		{
			if ( strcmp( *str, td->xdata.displayName ) == 0 )
				break;
		}

		if ( str == theSelect._szSelectDisplayName.end() )
		{	// Not already in list; add it
			char* szName = _strdup( td->name );
			assert( szName );
			theSelect._szSelect.push_back( szName );

			char* szDisplayName = _strdup( td->xdata.displayName );
			assert( szDisplayName );
			theSelect._szSelectDisplayName.push_back( szDisplayName );

			char* szHelp = _strdup( td->helpMessage );
			assert( szHelp );
			theSelect._szSelectHelp.push_back( szHelp );
		}
	}
	assert( size == cbFile );
}


Oad::~Oad()
{
	assert( _oad );
	free( _oad );

	assert( _szClassName );
	free( _szClassName );
}



const char*
Oad::ClassName() const
{
	return _szClassName;
}


const char*
Oad::find( INode* pNode, const char* szField ) const
{
	static char szValue[ 2048 ];

	assert( pNode );
	AppDataChunk* pOad = pNode->GetAppDataChunk(
		Attrib_ClassID, UTILITY_CLASS_ID, 1 );
	if ( pOad )
	{
		byte* xdataStart = (byte*)pOad->data;
		byte* xdata = xdataStart;

		while ( xdata < (byte*)pOad->data + pOad->length )
		{
			byte* pFieldStart = xdata;			// Remember starting place in case we need to skip over all the data

			int size = *((int*)xdata);
			//assert( size > 0 );
			xdata += sizeof( int );

			buttonType bt = *((int*)xdata);
			xdata += sizeof( int );

			visualRepresentation showAs = *((int*)xdata);
			xdata += sizeof( int );

			const char* szFieldName = (const char*)xdata;
			xdata += strlen( szFieldName ) + 1;

			if ( strcmp( szFieldName, szField ) == 0 )
			{
				switch ( bt )
				{
					case BUTTON_INT32:
					case BUTTON_FIXED32:
					{
//						sprintf( szValue, "%g", *( (int*)(xdata) ) / 65536.0 );
						assert( strlen( (char*)xdata ) < sizeof( szValue ) );
						strcpy( szValue, (char*)xdata );
						break;
					}

					case BUTTON_GROUP_START:
					case BUTTON_GROUP_STOP:
					case BUTTON_PROPERTY_SHEET:
					{
						sprintf( szValue, "%d", *( (int*)(xdata) ) );
						break;
					}

					case BUTTON_STRING:
					case BUTTON_OBJECT_REFERENCE:
					case BUTTON_FILENAME:
					case BUTTON_CAMERA_REFERENCE:
					case BUTTON_LIGHT_REFERENCE:
					case BUTTON_MESHNAME:
					case BUTTON_CLASS_REFERENCE:
					case BUTTON_XDATA:
					{
						assert( strlen( (char*)xdata ) < sizeof( szValue ) );
						strcpy( szValue, (char*)xdata );
						break;
					}

					case LEVELCONFLAG_NOINSTANCES:
					case LEVELCONFLAG_NOMESH:
					case LEVELCONFLAG_SINGLEINSTANCE:
					case LEVELCONFLAG_TEMPLATE:
					case LEVELCONFLAG_EXTRACTCAMERA:
					case LEVELCONFLAG_ROOM:
					case LEVELCONFLAG_COMMONBLOCK:
					case LEVELCONFLAG_ENDCOMMON:
					case LEVELCONFLAG_EXTRACTCAMERANEW:
					case LEVELCONFLAG_EXTRACTLIGHT:
					{
						// ignore
						break;
					}


					case BUTTON_WAVEFORM:
					case BUTTON_FIXED16:
					case BUTTON_INT8:
					case BUTTON_INT16:
					case BUTTON_EXTRACT_CAMERA:
					{
						assert( 0 );
						break;
					}

					default:
						strcpy( szValue, "Unknown button type" );
						break;

				}

				return szValue;
			}

			xdata = pFieldStart + size;
		}

		strcpy( szValue, szUndefined );
	}
	else
		*szValue = '\0';

	return szValue;
}
