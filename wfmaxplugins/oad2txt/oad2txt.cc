///////////////////////////////////////////////////////////////////////////////
//
// oad2txt.cpp	World Foundry
//
// Copyright 1998 Recombinant Limited.  All Rights Reserved.
//
//

#include "global.hpp"
#include "oad2txt.hpp"	// C++ header file
#include "oad2txt.h"		// Resource header file
#include "../lib/registry.h"		// registry reader
#include "oad.h"
#include <iffwrite/iffwrite.h>
#include <string>

static LVLExportClassDesc LVLExportCD;		// static instance of the export class descriptor
HINSTANCE hInstance;                        // this DLL's instance handle (some Windows thing)
Interface* gMaxInterface;					// Global pointer to MAX interface class

///////////////////////////////////////////////////////////////////////////////
// Functions called by MAX when our DLL is loaded

BOOL WINAPI
DllMain( HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved )
{
	if ( !hInstance )
	{
		hInstance = hinstDLL;
		InitCustomControls( hInstance );
		InitCommonControls();
	}

	return TRUE;
}


__declspec( dllexport ) int
LibNumberClasses()
{
	return 1;
}


__declspec( dllexport ) ClassDesc*
LibClassDesc( int i )
{
	if ( i == 0 )
		return &LVLExportCD;
	else
		return 0;
}


__declspec( dllexport ) const TCHAR*
LibDescription()
{
	return _T( "World Foundry Level Attributes to Text v" LEVELCON_VER );
}


__declspec( dllexport ) ULONG
LibVersion()
{
	return VERSION_3DSMAX;
}

///////////////////////////////////////////////////////////////////////////////
// Miscelaneous support functions (GUI crap, etc.)

TCHAR*
GetString( int id )
{
	static TCHAR buf[256];
	if ( hInstance )
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


static void
MessageBox(int s1, int s2)
{
	TSTR str1( GetString( s1 ) );
	TSTR str2( GetString( s2 ) );
	MessageBox( GetActiveWindow(), str1.data(), str2.data(), MB_OK );
}


static int
MessageBox(int s1, int s2, int option = MB_OK)
{
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
}


static int
Alert(int s1, int s2 = IDS_LVLEXP, int option = MB_OK)
{
	return (int)MessageBox(s1, s2, option);
}

///////////////////////////////////////////////////////////////////////////////
// Actual methods of the LVLExport class

LVLExport::LVLExport()
{
}


LVLExport::~LVLExport()
{
}


int
LVLExport::ExtCount()
{
	return 1;
}


const TCHAR*
LVLExport::Ext(int n)		// Extensions supported for import/export modules
{
	switch(n)
	{
		case 0:
			return _T("txt");
	}
	return _T("");
}


const TCHAR*
LVLExport::LongDesc()		// Long ASCII description (i.e. "Targa 2.0 Image File")
{
	return ShortDesc();
}


const TCHAR*
LVLExport::ShortDesc()		// Short ASCII description (i.e. "Targa")
{
	return _T("World Foundry Level Attributes Text File");
}


const TCHAR*
LVLExport::AuthorName()		// ASCII Author name
{
	return _T("William B. Norris IV");
}


const TCHAR*
LVLExport::CopyrightMessage() 	// ASCII Copyright message
{
	return _T("Copyright 1997 Recombinant Limited.  All Rights Reserved.");
}


const TCHAR*
LVLExport::OtherMessage1()		// Other message #1
{
	return _T("OtherMessage1");
}


const TCHAR*
LVLExport::OtherMessage2()		// Other message #2
{
	return _T("OtherMessage2");
}


unsigned int
LVLExport::Version()			// Version number * 100 (i.e. v3.01 = 301)
{
	return 0;
}

void AboutBox( HWND hDlg );

void
LVLExport::ShowAbout( HWND hWnd )
{
	assert( hWnd );
	AboutBox( hWnd );
}


void
wf2txt( ostream& s, SceneEnumProc* theSceneEnum )
{
	IffWriterText _iff( s );
	//_iff.enterChunk( ID( "OAD" ) );


	for ( SceneEntry* thisEntry = theSceneEnum->head; thisEntry; thisEntry = thisEntry->next )
	{
		//{OBJECT} {tarbox01} {target}
		//	{FIELD} 1 {Mass} {75.000000}

		char* oadData = NULL;
		size_t oadDataSize = 0;
		std::string className;

		INode* thisNode = thisEntry->node;
		assert( thisNode );

		AppDataChunk* appDataChunk = thisNode->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_CLASS );
		int xDataSize = -1;

		className = std::string( appDataChunk ? (char*)(appDataChunk->data) : DEFAULT_CLASS );


		_iff.enterChunk( ID( "OBJ" ) );
			_iff.enterChunk( ID( "NAME" ) );
				_iff << thisEntry->node->GetName();
			_iff.exitChunk();

			_iff.enterChunk( ID( "STR" ) );
				_iff.enterChunk( ID( "NAME" ) );
					_iff << "Class Name";
				_iff.exitChunk();
				_iff.enterChunk( ID( "DATA" ) );
					_iff << className;
				_iff.exitChunk();
			_iff.exitChunk();

			AppDataChunk* adOad = thisNode->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_OAD );
			if ( adOad )
			{	// Apply fields stored in AppData overriding the defaults contained in the .oad file
				char* xdataStart = (char*)adOad->data;
				char* xdata = xdataStart;

				while ( xdata < (char*)adOad->data + adOad->length )
				{
					char* pFieldStart = xdata;			// Remember starting place in case we need to skip over all the data

					int size = *((int*)xdata);
					xdata += sizeof( int );

					buttonType bt = *((int*)xdata);
					xdata += sizeof( int );

					visualRepresentation showAs = *((int*)xdata);
					xdata += sizeof( int );

					const char* szFieldName = (const char*)xdata;
					xdata += strlen( szFieldName ) + 1;

					//s << "\t{FIELD} {" << szFieldName << "} ";

					switch ( bt )
					{
						case BUTTON_FIXED16:
						case BUTTON_INT8:
						case BUTTON_INT16:
						case BUTTON_WAVEFORM:
							assert( 0 );
							break;

						case BUTTON_GROUP_START:
						case BUTTON_GROUP_STOP:
						case BUTTON_EXTRACT_CAMERA:
						case LEVELCONFLAG_EXTRACTCAMERANEW:
						case LEVELCONFLAG_NOINSTANCES:
						case LEVELCONFLAG_NOMESH:
						case LEVELCONFLAG_SINGLEINSTANCE:
						case LEVELCONFLAG_TEMPLATE:
						case LEVELCONFLAG_EXTRACTCAMERA:
						case LEVELCONFLAG_ROOM:
						case LEVELCONFLAG_COMMONBLOCK:
						case LEVELCONFLAG_ENDCOMMON:
							assert( 0 );			// Should not have been stored
							break;

#define simpleChunk( __id__, __data__ )				do { _iff.enterChunk( __id__ ); _iff << __data__; _iff.exitChunk(); } while ( 0 )
#define typeChunk( __type__, __name__, __data__ )	do { _iff.enterChunk( __type__ ); simpleChunk( ID( "NAME" ), __name__ ); simpleChunk( ID( "DATA" ), __data__ ); _iff.exitChunk(); } while ( 0 )
#define TYPECHUNK( __str__ )							typeChunk( ID( __str__ ), szFieldName, xdata )

						case BUTTON_PROPERTY_SHEET:
						{
							ulong i = *((ulong*)xdata);  xdata += sizeof( long );
							typeChunk( ID( "I32" ), szFieldName, i );
							//s << "{PROPERTY_SHEET} {" << i << "}";
							break;
						}

						case BUTTON_INT32:
						{
							typeChunk( ID( "I32" ), szFieldName, xdata );
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_FIXED32:
						{
							typeChunk( ID( "FX32" ), szFieldName, xdata );
							//s << "{FIXED32} {" << xdata << "}";
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_STRING:
						{
							typeChunk( ID( "STR" ), szFieldName, xdata );
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_OBJECT_REFERENCE:
						{
							_iff.enterChunk( ID( "STR" ) );
								simpleChunk( ID( "NAME" ), szFieldName );
								simpleChunk( ID( "DATA" ), xdata );
								simpleChunk( ID( "HINT" ), "Object Reference" );
							_iff.exitChunk();
							//s << "{OBJECT_REFERENCE} {" << xdata << "}";
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_FILENAME:
						{
							_iff.enterChunk( ID( "STR" ) );
								simpleChunk( ID( "NAME" ), szFieldName );
								simpleChunk( ID( "DATA" ), xdata );
								simpleChunk( ID( "HINT" ), "Filename" );
							_iff.exitChunk();
							//s << "{FILENAME} {" << xdata << "}";
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_CAMERA_REFERENCE:
						{
							_iff.enterChunk( ID( "STR" ) );
								simpleChunk( ID( "NAME" ), szFieldName );
								simpleChunk( ID( "DATA" ), xdata );
								simpleChunk( ID( "HINT" ), "Camera Reference" );
							_iff.exitChunk();
							//s << "{CAMERA_REFERENCE} {" << xdata << "}";
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_LIGHT_REFERENCE:
						{
							_iff.enterChunk( ID( "STR" ) );
								simpleChunk( ID( "NAME" ), szFieldName );
								simpleChunk( ID( "DATA" ), xdata );
								simpleChunk( ID( "HINT" ), "Light Reference" );
							_iff.exitChunk();
							//s << "{LIGHT_REFERENCE} {" << xdata << "}";
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_MESHNAME:
						{
							_iff.enterChunk( ID( "STR" ) );
								simpleChunk( ID( "NAME" ), szFieldName );
								simpleChunk( ID( "DATA" ), xdata );
								simpleChunk( ID( "HINT" ), "Meshname" );
							_iff.exitChunk();
							//s << "{MESHNAME} {" << xdata << "}";
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_CLASS_REFERENCE:
						{
							_iff.enterChunk( ID( "STR" ) );
								simpleChunk( ID( "NAME" ), szFieldName );
								simpleChunk( ID( "DATA" ), xdata );
								simpleChunk( ID( "HINT" ), "Class Reference" );
							_iff.exitChunk();
							//s << "{CLASS_REFERENCE} {" << xdata << "}";
							xdata += strlen( xdata ) + 1;
							break;
						}

						case BUTTON_XDATA:
						{
							_iff.enterChunk( ID( "STR" ) );
								simpleChunk( ID( "NAME" ), szFieldName );
								simpleChunk( ID( "DATA" ), xdata );
								simpleChunk( ID( "HINT" ), "xdata" );
							_iff.exitChunk();
							//s << "{XDATA} {" << xdata << "}";
							xdata += strlen( xdata ) + 1;
							break;
						}

						default:
						{
							char szMessage[ 512 ];
							sprintf( szMessage, "Unknown button type of %d, field name = \"%s\"", bt, szFieldName );
							OutputDebugString( szMessage );
							break;
						}
					}

					xdata = pFieldStart + size;
					s << endl;
				}
//	 			assert( pOad == pOadEnd );
//				free( pOadStart );

			_iff.exitChunk();
		}
	}
	//s << "{END}" << endl;
}


int
#if MAX_RELEASE < 2000
LVLExport::DoExport(const TCHAR *name,ExpInterface *ei,Interface *gi)	// Export file
#else
LVLExport::DoExport(const TCHAR *name,ExpInterface *ei,Interface *gi,int)	// Export file
#endif
{
	assert( gi );
	gMaxInterface = gi;

	// Ask the scene to enumerate all of its nodes so we can determine if there are any we can use
	SceneEnumProc myScene( ei->theScene, gi->GetTime(), gi );

	try
	{
		ofstream fp( name, ios::out );
		wf2txt( fp, &myScene );
	}
	catch ( LVLExporterException theException )
	{
		return 0;	// Return to MAX with failure
	}

	return 1;		// Return to MAX with success

}

//==============================================================================
