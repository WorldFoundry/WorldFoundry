/**********************************************************************
 *<
	FILE: txt2oad.cpp

	DESCRIPTION:  Import World Foundry Level information (.txt)

	CREATED BY: William B. Norris IV

  8 Feb 97	WBNIV	Created
  9 Feb 97	WBNIV	Finished

  Copyright 1997 Recombinant Limited.  All Rights Reserved.

 *>
 **********************************************************************/

#include "Max.h"
#include <stdio.h>
#include <direct.h>
#include <commdlg.h>
#include "3dsires.h"
#include "txt2oad.h"
#include "../lib/wf_id.hp"
 
#include <source/oas/pigtool.h>
#include <source/oas/oad.h>

HINSTANCE hInstance;

TCHAR *GetString(int id)
{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


BOOL WINAPI 
DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	if ( !hInstance ) 
	{
		hInstance = hinstDLL;
		InitCustomControls( hInstance );
		InitCommonControls();
	}

	switch ( fdwReason ) 
	{
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,_T("3DSIMP.DLL: DllMain"),_T("3DSIMP"),MB_OK);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

//------------------------------------------------------

class StudioClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new WorldFoundryImport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_3DSTUDIO); }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID() { return Txt2Oad_ClassID; }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEIMPORT);  }
	};

static StudioClassDesc StudioDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return Txt2Oad_ClassName; }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &StudioDesc; break;
		default: return 0; break;
		}

	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }




WorldFoundryImport::WorldFoundryImport() {
	}

WorldFoundryImport::~WorldFoundryImport() {
	}

int
WorldFoundryImport::ExtCount() {
	return 1;
	}

const TCHAR *
WorldFoundryImport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("txt");
		}
	return _T("");
	}

const TCHAR *
WorldFoundryImport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_3DSSCENEFILE);
	}
	
const TCHAR *
WorldFoundryImport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_3DSMESH);
	}

const TCHAR *
WorldFoundryImport::AuthorName() {			// ASCII Author name
	return _T( "William B. Norris IV" );
	}

const TCHAR *
WorldFoundryImport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
WorldFoundryImport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
WorldFoundryImport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
WorldFoundryImport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
WorldFoundryImport::ShowAbout(HWND hWnd) 
{
}



static BOOL CALLBACK
ImportDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) 
	{
		case WM_INITDIALOG:
			//CheckRadioButton( hDlg, IDC_3DS_MERGE, IDC_3DS_REPLACE, replaceScene?IDC_3DS_REPLACE:IDC_3DS_MERGE );
			CenterWindow(hDlg,GetParent(hDlg));
			SetFocus(hDlg); // For some reason this was necessary.  DS-3/4/96
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDOK: 
				{
	            	//replaceScene = IsDlgButtonChecked(hDlg,IDC_3DS_REPLACE);
					EndDialog(hDlg, 1);
					return TRUE;
				}
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
		}
	}

	return FALSE;
}


void
skip_ws( char* & str )
{
    while ( *str && isspace( *str ) )
        ++str;
}


void
skip_to_ws( char* & str )
{
    while ( *str && !isspace( *str ) )
        ++str;
}


void
get_field( char* & str, char* szTag )
{
    skip_ws( str );
	if ( *str != '{' )
		DebugBreak();
    assert( *str == '{' );
    ++str;
    char* pQuote = strchr( str, '}' );
	assert( pQuote );
    strncpy( szTag, str, pQuote - str );
    *( szTag + ( pQuote - str ) ) = '\0';
    str = pQuote + 1;
	skip_ws( str );
}


char*
LoadTextFile( const char* _szFilename, int& nSize )
{
	FILE* fp;
	char* ptr;
	const char* szFilename = _szFilename;

	fp = fopen( (char*)szFilename, "rt" );
	if ( !fp )
		return NULL;

	fseek( fp, 0L, SEEK_END );
	assert( !ferror( fp ) );
	nSize = ftell( fp );
	fseek( fp, 0L, SEEK_SET );
	assert( !ferror( fp ) );

	ptr = (char*)malloc( nSize+1 );
	assert( ptr );
	if ( ptr )
	{
		int cbRead = fread( ptr, 1, nSize, fp );
		assert( !ferror( fp ) );
		assert( cbRead <= nSize );
		*( ptr + cbRead ) = '\0';
		nSize = cbRead;
	}

	fclose( fp );
	assert( !ferror( fp ) );

	return ptr;
}


char oad[ 32768 ];
char* pOad;

#if 0
enum
{
	SLOT_CLASS,
	SLOT_OAD
};
#endif

#define CHECK_BUTTON( __buttonName__, __buttonId__ ) \
	else if ( strcmp( szButtonType, __buttonName__ ) == 0 ) \
		return __buttonId__


int 
buttonNameToType( const char* szButtonType )
{
	if ( 0 )
		;
	CHECK_BUTTON( "PROPERTY_SHEET", BUTTON_PROPERTY_SHEET );
	CHECK_BUTTON( "INT32", BUTTON_INT32 );
	CHECK_BUTTON( "FIXED32", BUTTON_FIXED32 );
	CHECK_BUTTON( "STRING", BUTTON_STRING );
	CHECK_BUTTON( "OBJECT_REFERENCE", BUTTON_OBJECT_REFERENCE );
	CHECK_BUTTON( "FILENAME", BUTTON_FILENAME );
	CHECK_BUTTON( "CAMERA_REFERENCE", BUTTON_CAMERA_REFERENCE );
	CHECK_BUTTON( "LIGHT_REFERENCE", BUTTON_LIGHT_REFERENCE );
	CHECK_BUTTON( "MESHNAME", BUTTON_MESHNAME );
	CHECK_BUTTON( "CLASS_REFERENCE", BUTTON_CLASS_REFERENCE );
	CHECK_BUTTON( "XDATA", BUTTON_XDATA );
	else
	{
		assert( 0 );
		return -1;
	}
}


int
#if MAX_RELEASE < 2000
WorldFoundryImport::DoImport( const TCHAR* filename, ImpInterface* i, Interface* gi ) 
#else
WorldFoundryImport::DoImport( const TCHAR* filename, ImpInterface* i, Interface* gi, int ) 
#endif
{
	assert( gi );
	SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	int len;
	char* _level = LoadTextFile( filename, len );
	assert( _level );

	char szTag[ 100 ];			// OBJECT, FIELD, or END

	INode* iNode = NULL;		// current object being parsed
	char* p = _level;
	char* pEnd = _level + len;
	pOad = oad;
	char* pOadFieldStart = NULL;

	while ( p < pEnd )
	{
		assert( pOad - oad < sizeof( oad ) );			// Don't overrun buffer

        get_field( p, szTag );

        if ( (strcmp( szTag, "OBJECT" ) == 0) || (strcmp( szTag, "END" ) == 0) )
		{
            static char szObjectName[ 200 ];				// Save until done parsing fields
            static char szClassName[ 100 ];				// Save until done parsing fields

			if ( iNode )
			{	// Starting a new object, but already have an existing one.  Update its AppData
				// Remove existing chunk
				iNode->RemoveAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_CLASS );
				iNode->RemoveAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_OAD );

				// Add a chunk
				assert( *szClassName );
				char* appClassName = _strdup( szClassName );
				assert( appClassName );
				char* appOad = (char*)malloc( pOad - oad );
				assert( appOad );
				memcpy( appOad, oad, pOad - oad );
				iNode->AddAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_CLASS, strlen( appClassName ) + 1, appClassName );
				iNode->AddAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_OAD, pOad - oad, appOad );
			}

			if ( strcmp( szTag, "OBJECT" ) == 0 )
			{ // New object -- continue getting more object data and fields
				get_field( p, szObjectName );
				get_field( p, szClassName );
				iNode = gi->GetINodeByName( szObjectName );
				pOad = oad;
			}
			// "END" falls through to end
		}
        else if ( strcmp( szTag, "FIELD" ) == 0 )
		{
			// Write out data to AppData chunk
			pOadFieldStart = pOad;
			*((long*)pOad) = -1;			pOad += sizeof( long );

			// Field Name
            char szFieldName[ 100 ];
            get_field( p, szFieldName );

			// Button type
			char szButtonType[ 32 ];
			get_field( p, szButtonType );
//			skip_to_ws( p );

            int buttonType = buttonNameToType( szButtonType );
			*((long*)pOad) = buttonType;	pOad += sizeof( long );

			*((long*)pOad) = -1;			pOad += sizeof( long );

			strcpy( pOad, szFieldName );	pOad += strlen( szFieldName ) + 1;

            switch ( buttonType )
			{
                case BUTTON_FIXED16:
                case BUTTON_INT8:
                case BUTTON_INT16:
                case BUTTON_WAVEFORM:
                    assert( 0 );
                    break;

                case BUTTON_INT32:
                {
					char szNumber[ 256 ];
					get_field( p, szNumber );
					strcpy( pOad, szNumber );
					pOad += strlen( szNumber ) + 1;
                    break;
                }

                case BUTTON_FIXED32:
                {
					char szNumber[ 256 ];
					get_field( p, szNumber );
					strcpy( pOad, szNumber );
					pOad += strlen( szNumber ) + 1;
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
                    static char szString[ 32768 ];
                    get_field( p, szString );
					strcpy( pOad, szString );
					pOad += strlen( szString ) + 1;
                    break;
                }

                case BUTTON_PROPERTY_SHEET:
				{
					char szNumber[ 40 ];
					get_field( p, szNumber );
					*((long*)pOad) = atol( szNumber );	pOad += 4;
					break;
				}

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
				case LEVELCONFLAG_SHORTCUT:
                    assert( 0 );                    // Should not have been stored
                    break;

                default:
                        assert( 0 );
            }

			assert( pOadFieldStart );
			assert( *((long*)pOadFieldStart) == -1 );				// size contained -1 as placeholder
			*((long*)pOadFieldStart) = pOad - pOadFieldStart;		// write out size of data 
        }
		else
		{
			DebugPrint( "" );
			assert( 0 );
		}
    }

	assert( p == pEnd );
	free( _level );

	SetCursor( LoadCursor( NULL, IDC_ARROW ) );

	return IMPEXP_SUCCESS;
}
