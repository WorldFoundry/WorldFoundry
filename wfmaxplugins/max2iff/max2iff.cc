///////////////////////////////////////////////////////////////////////////////
//																			 //
// wrlexp.cpp	IFF Exporter plugin for 3DS Max								 //
//																			 //
//	(This is the new World Foundry replacement for 3DS2VRML.EXE)			 //
//	Copyright 1996,1997 by Recombinant Ltd.  All rights reserved.			 //
//																			 //
//	Written 1/6/97 by Phil Torre											 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#define EXPORTER_VERSION "V2.0.46"

#include "global.hpp"
#include <fstream>
#include "max2iff.hpp"
#include "max2iff.h"
#include "iffconv.hpp"
#include "../lib/registry.h"
#include "../lib/wf_id.hp"
#include "../lib/levelnum.h"
#include <commdlg.h>

#define szAppName	"max2iff"
#define szAppError	szAppName " Error"

static WRLExportClassDesc WRLExportCD;		// static instance of the export class descriptor
HINSTANCE hInstance;                        // this DLL's instance handle (some Windows thing)

char szOutputObjectName[ _MAX_PATH ];	// output filename
//INode* pOutputNode;
char szOutputName[ _MAX_PATH ];			// output object name

struct HwndFixedPoint
{
	HWND hwndSign;
	HWND hwndWhole;
	HWND hwndFraction;
};
static HwndFixedPoint hwndReal, hwndQuaternion;
max2iffOptions theOptions;

//============================================================================

void
WRLExpAssert( void* string, void* file , unsigned line )
{
	AssertMessageBox(0, "MAX2IFF Internal Error!" << endl << (char*)string << endl << "File: " << (char*)file << ", line: " << line);
}

//============================================================================

///////////////////////////////////////////////////////////////////////////////
// Functions called by MAX when our DLL is loaded

extern "C" BOOL WINAPI
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
	assert( i == 0 );
	return &WRLExportCD;
}


__declspec( dllexport ) const TCHAR*
LibDescription()
{
	return _T( "World Foundry Mesh Exporter " EXPORTER_VERSION);
}


__declspec( dllexport ) ULONG
LibVersion()
{
	return VERSION_3DSMAX;
}


///////////////////////////////////////////////////////////////////////////////
// Miscelaneous support functions

TCHAR*
GetString( int id )
{
	static TCHAR buf[256];
	if ( hInstance )
		return LoadString( hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


static void
MessageBox( int s1, int s2 )
{
	TSTR str1( GetString( s1 ) );
	TSTR str2( GetString( s2 ) );
	MessageBox(GetActiveWindow(), str1.data(), str2.data(), MB_OK);
}


static int
MessageBox(int s1, int s2, int option = MB_OK)
{
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
}


static int
Alert( int s1, int s2 = IDS_WRLEXP, int option = MB_OK )
{
	return (int)MessageBox(s1, s2, option);
}

///////////////////////////////////////////////////////////////////////////////
// Actual methods of the WRLExport class

WRLExport::WRLExport()
{
}


WRLExport::~WRLExport()
{
}


int
WRLExport::ExtCount()
{
	return 1;
}


const TCHAR*
WRLExport::Ext( int n )		// Extensions supported for import/export modules
{
	assert( n == 0 );
	return _T("iff");
}


const TCHAR *
WRLExport::LongDesc()		// Long ASCII description (i.e. "Targa 2.0 Image File")
{
	return _T("World Foundry Intermediate Mesh Format (IFF)");
}


const TCHAR *
WRLExport::ShortDesc()		// Short ASCII description (i.e. "Targa")
{
	return _T("World Foundry Intermediate Mesh");
}


const TCHAR *
WRLExport::AuthorName()		// ASCII Author name
{
	return _T("William B. Norris IV");
}


const TCHAR *
WRLExport::CopyrightMessage() 	// ASCII Copyright message
{
	return _T("Copyright 1997-1998 World Foundry Group.  All Rights Reserved.");
}


const TCHAR *
WRLExport::OtherMessage1()		// Other message #1
{
	return _T("");
}


const TCHAR *
WRLExport::OtherMessage2()		// Other message #2
{
	return _T("");
}


unsigned int
WRLExport::Version()			// Version number * 100 (i.e. v3.01 = 301)
{
	return 200;
}


extern void AboutBox( HWND );

void
WRLExport::ShowAbout( HWND hWnd )
{
	//AboutBox( hWnd );
}

////////////////////////////////////////////////////////////////////////////////

static char*
search_and_replace( char* pszString, char search, char replace )
{
	for ( ; *pszString; ++pszString )
	{
		if ( *pszString == search )
			*pszString = replace;
	}

	return pszString;
}

////////////////////////////////////////////////////////////////////////////////

HWND hwndAscii, hwndBinary;
HWND hwndGeometry, hwndAnimation, hwndMaterials, hwndHandles, hwndEvents;
HWND hwndGPosition, hwndGRotation, hwndGScale;
HWND hwndScale;
HWND hwndDontPrompt;

#define szRegMax2IFF szRegWorldFoundryGDK "\\max2iff"


#define GET_BUTTON( __hwnd__, __id__ ) \
	do { __hwnd__ = GetDlgItem( hDlg, __id__ ); assert( __hwnd__ ); } while ( 0 )


void
SET_BUTTON( HWND hwnd, const char* szRegistryEntry, bool bDefault )
{
	assert( hwnd );

	bool bValue = bDefault;

	char szValue[ 64 ];
	if ( GetLocalMachineStringRegistryEntry( szRegMax2IFF, szRegistryEntry, szValue, sizeof( szValue ) ) )
		bValue = atoi( szValue );

	Button_SetCheck( hwnd, bValue );
}

//==============================================================================

void
SAVE_BUTTON( HWND hwnd, const char* szRegistryEntry )
{
	assert( hwnd );
	SetLocalMachineStringRegistryEntry( szRegMax2IFF, szRegistryEntry, Button_GetState( hwnd ) ? "1" : "0" );
}


void
Edit_SetNumber( HWND hwnd, /*string szRegistryEntry,*/ int val )
{
	char szBuffer[ 256 ];
	sprintf( szBuffer, "%d", val );

//	GetLocalMachineStringRegistryEntry( szRegMax2IFFLVL, szRegistryEntry.c_str(), szBuffer, sizeof( szBuffer ) );

	Edit_SetText( hwnd, szBuffer );
}


void
SetHWNDFixedPointPrecision( HwndFixedPoint& hwnd, const char* szRegistryEntry, size_specifier& ss )
{
	char szBuffer[ 128 ];
	if ( GetLocalMachineStringRegistryEntry(
	szRegMax2IFF,
	szRegistryEntry,
	szBuffer,
	sizeof( szBuffer ) ) )
		sscanf( szBuffer, "%d.%d.%d", &ss.sign, &ss.whole, &ss.fraction );

	Edit_SetNumber( hwnd.hwndSign, ss.sign );
	Edit_SetNumber( hwnd.hwndWhole, ss.whole );
	Edit_SetNumber( hwnd.hwndFraction, ss.fraction );
}


void
GetHWNDFixedPointPrecision( HwndFixedPoint& hwnd, const char* szRegistryEntry, size_specifier& ss )
{
	char szBuffer[ 128 ];
	Edit_GetText( hwnd.hwndSign, szBuffer, sizeof( szBuffer ) );
	ss.sign = atoi( szBuffer );
	Edit_GetText( hwnd.hwndWhole, szBuffer, sizeof( szBuffer ) );
	ss.whole = atoi( szBuffer );
	Edit_GetText( hwnd.hwndFraction, szBuffer, sizeof( szBuffer ) );
	ss.fraction = atoi( szBuffer );

	sprintf( szBuffer, "%d.%d.%d", ss.sign, ss.whole, ss.fraction );
	SetLocalMachineStringRegistryEntry( szRegMax2IFF, szRegistryEntry, szBuffer );
}

//==============================================================================
// common code to export a single object into an IFF stream

int
WRLExport::ExportObject( INode* pNode, _IffWriter* iff, const max2iffOptions* options )
{
	assert( pNode );

	HCURSOR hcursorCurrent = GetCursor();
	assert( hcursorCurrent );
	HCURSOR hcursorWait = LoadCursor( NULL, IDC_WAIT );
	assert( hcursorWait );
	SetCursor( hcursorWait );

	try
	{
		IffConversion theConversion( pNode, iff, options );
	}
	catch ( WRLExporterException /*theException*/ )
	{
		SetCursor( hcursorCurrent );
		return 0;	// Return to MAX with failure
	}
}

//==============================================================================

BOOL CALLBACK
OutputOptionsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( msg == WM_INITDIALOG )
	{
		char str[ 512 ];
		bool bDontPrompt = false;
		bool bOutputBinary = true;
		char szScale[ 100 ] = "1.0";

		HICON _hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_WF ) );
		assert( _hIcon );
		SetClassLong( hDlg, GCL_HICON, (LONG)_hIcon );

		HWND hwndObjectName = GetDlgItem( hDlg, IDC_OBJECT_NAME );
		assert( hwndObjectName );
		Button_SetText( hwndObjectName, szOutputName );

		HWND hwndOutputFilename = GetDlgItem( hDlg, IDC_OUTPUT_FILENAME );
		assert( hwndOutputFilename );
		Edit_SetText( hwndOutputFilename, szOutputObjectName );

		if ( GetLocalMachineStringRegistryEntry( szRegMax2IFF, "Output Mode", str, sizeof( str ) ) )
			bOutputBinary = tolower( *str ) == 'b';

		GetLocalMachineStringRegistryEntry( szRegMax2IFF, "Scale", szScale, sizeof( szScale ) );
		GET_BUTTON( hwndScale, IDC_SCALE );
		Edit_SetText( hwndScale, szScale );

		GET_BUTTON( hwndAscii, IDC_ASCII );
		GET_BUTTON( hwndBinary, IDC_BINARY );
		Button_SetCheck( bOutputBinary ? hwndBinary : hwndAscii, true );

		GET_BUTTON( hwndGeometry, IDC_GEOMETRY );
		GET_BUTTON( hwndAnimation, IDC_ANIMATION );
		GET_BUTTON( hwndMaterials, IDC_MATERIALS );
		GET_BUTTON( hwndHandles, IDC_HANDLES );
		GET_BUTTON( hwndEvents, IDC_EVENTS );
		GET_BUTTON( hwndGPosition, IDC_GPOSITION );
		GET_BUTTON( hwndGRotation, IDC_GROTATION );
		GET_BUTTON( hwndGScale, IDC_GSCALE );

		GET_BUTTON( hwndReal.hwndSign, IDC_REAL_SIGN );
		GET_BUTTON( hwndReal.hwndWhole, IDC_REAL_WHOLE );
		GET_BUTTON( hwndReal.hwndFraction, IDC_REAL_FRACTION );
		GET_BUTTON( hwndQuaternion.hwndSign, IDC_QUAT_SIGN );
		GET_BUTTON( hwndQuaternion.hwndWhole, IDC_QUAT_WHOLE );
		GET_BUTTON( hwndQuaternion.hwndFraction, IDC_QUAT_FRACTION );

		SET_BUTTON( hwndGeometry, "Output Geometry", true );
		SET_BUTTON( hwndAnimation, "Output Animation", false );
		SET_BUTTON( hwndMaterials, "Output Materials", true );
		SET_BUTTON( hwndHandles, "Output Handles", false );
		SET_BUTTON( hwndEvents, "Output Events", false );
		SET_BUTTON( hwndGPosition, "Output Global Position", false );
		SET_BUTTON( hwndGRotation, "Output Global Rotation", false );
		SET_BUTTON( hwndGScale, "Output Global Scale", true );

		SetHWNDFixedPointPrecision( hwndReal, "real", theOptions.sizeReal );
		SetHWNDFixedPointPrecision( hwndQuaternion, "quaternion", theOptions.sizeQuaternion );

		hwndDontPrompt = GetDlgItem( hDlg, IDC_DONT_PROMPT );
		assert( hwndDontPrompt );

		if ( GetLocalMachineStringRegistryEntry( szRegMax2IFF, "Don't Prompt", str, sizeof( str ) ) )
			bDontPrompt = atof( str );

		bool bShiftPressed = GetAsyncKeyState( VK_SHIFT );
		if ( bDontPrompt && !bShiftPressed )
		{
			EndDialog( hDlg, TRUE );
		}
		else
		{
			Button_SetCheck( hwndDontPrompt, bDontPrompt );
		}
	}
	else if ( msg == WM_COMMAND )
	{
		int button = LOWORD( wParam );
		switch ( button )
		{
			case IDC_BROWSE:
			{
				assert( hInstance );

				search_and_replace( szOutputObjectName, '/', '\\' );

				OPENFILENAME ofn = {
					sizeof( OPENFILENAME ),
					GetParent( (HWND)lParam ),			// hwndOwner
					hInstance,			// hInstance
					"World Foundry IFF (*.iff)\0*.iff\0\0",	// lpstrFilter
					NULL,				// lpstrCustomFilter,
					0,					// nMaxCustFilter
					0,					// nFilterIndex
					szOutputObjectName,
					sizeof( szOutputObjectName ),
					NULL,				// lpstrFileTitle
					0,					// nMaxFileTitle
					NULL,	//?		// lpstrInitialDir
					NULL,			// lpstrTitle
					OFN_HIDEREADONLY | OFN_EXPLORER,	// Flags
//					OFN_HIDEREADONLY | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,	// Flags
					0,					// nFileOffset
					0,					// nFileExtension
					NULL,				// lpStrDefExt,
					NULL,				// lCustData
					NULL,				// lpfnHook
					NULL				// lpTemplateName
				};

				if ( GetSaveFileName( &ofn ) )
				{
					HWND hwndOutputFilename = GetDlgItem( hDlg, IDC_OUTPUT_FILENAME );
					assert( hwndOutputFilename );
					search_and_replace( szOutputObjectName, '\\', '/' );
					Edit_SetText( hwndOutputFilename, szOutputObjectName );
				}
				break;
			}

			case IDOK:
			{
				HWND hwndOutputFilename = GetDlgItem( hDlg, IDC_OUTPUT_FILENAME );
				assert( hwndOutputFilename );
				Edit_GetText( hwndOutputFilename, szOutputObjectName, sizeof( szOutputObjectName ) );

				assert( hwndBinary );
				theOptions.bOutputBinary = Button_GetState( hwndBinary );
				SetLocalMachineStringRegistryEntry( szRegMax2IFF, "Output Mode",
					theOptions.bOutputBinary ? "binary" : "ascii" );

				assert( hwndScale );
				char szScale[ 100 ];
				Edit_GetText( hwndScale, szScale, sizeof( szScale ) );
				SetLocalMachineStringRegistryEntry( szRegMax2IFF, "Scale", szScale );
				theOptions.geometryScale = atof( szScale );

				SAVE_BUTTON( hwndDontPrompt, "Don't Prompt" );
				//max2iffOptions.bDontPrompt

				SAVE_BUTTON( hwndGeometry, "Output Geometry" );
				theOptions.bOutputGeometry = bool( Button_GetState( hwndGeometry ) );
				SAVE_BUTTON( hwndAnimation, "Output Animation" );
				theOptions.bOutputAnimation = bool( Button_GetState( hwndAnimation ) );
				SAVE_BUTTON( hwndMaterials, "Output Materials" );
				theOptions.bOutputMaterials = bool( Button_GetState( hwndMaterials ) );
				SAVE_BUTTON( hwndHandles, "Output Handles" );
				theOptions.bOutputHandles = bool( Button_GetState( hwndHandles ) );
				SAVE_BUTTON( hwndEvents, "Output Events" );
				theOptions.bOutputEvents = bool( Button_GetState( hwndEvents ) );
				SAVE_BUTTON( hwndGPosition, "Output Global Position" );
				theOptions.bOutputGlobalPosition = bool( Button_GetState( hwndGPosition ) );
				SAVE_BUTTON( hwndGRotation, "Output Global Rotation" );
				theOptions.bOutputGlobalRotation = bool( Button_GetState( hwndGRotation ) );
				SAVE_BUTTON( hwndGScale, "Output Global Scale" );
				theOptions.bOutputGlobalScale = bool( Button_GetState( hwndGScale ) );

				GetHWNDFixedPointPrecision( hwndReal, "real", theOptions.sizeReal );
				GetHWNDFixedPointPrecision( hwndQuaternion, "quaternion", theOptions.sizeQuaternion );

				EndDialog( hDlg, TRUE );
				return TRUE;
			}

			case IDCANCEL:
				EndDialog( hDlg, FALSE );
				return FALSE;
		}
	}
	return FALSE;
}

//==============================================================================

bool
max2iffQuery( INode* pNode, const char* szOutputObjectName_Input)
{
	assert( pNode );

//	char str[ 512 ] = { "0" };

//	GetLocalMachineStringRegistryEntry( szRegMax2IFF, "Don't Prompt", str, sizeof( str ) );
//	bool bDontPrompt = atof( str );

//	if ( !bDontPrompt )
	{
		Interface* ip = GetCOREInterface();
		assert( ip );

		// Output variables for dialog box
		strcpy(szOutputName,pNode->GetName());
		strcpy( szOutputObjectName, (char*)szOutputObjectName_Input );
		search_and_replace( szOutputObjectName, '\\', '/' );

		HWND _hwndMax = ip->GetMAXHWnd();
		assert( _hwndMax );

		assert( hInstance );
		if ( !DialogBox( hInstance, MAKEINTRESOURCE( IDD_OUTPUT_OPTIONS ), _hwndMax, OutputOptionsDlgProc ) )
			return false;
	}
	return true;
}

//==============================================================================
// main entry point from max
//==============================================================================

int
#if MAX_RELEASE < 2000
WRLExport::DoExport( const TCHAR* szOutputObjectName, ExpInterface* ,Interface* gi )
#else
WRLExport::DoExport( const TCHAR* szOutputObjectName, ExpInterface* ,Interface* gi, int )
#endif
{
	assert( gi );
	assert( szOutputObjectName );

	HWND _hwndMax = gi->GetMAXHWnd();
	assert( _hwndMax );

	int numObjects = gi->GetSelNodeCount();

	// If no nodes are selected and usable, bail out
	if ( numObjects == 0 )
	{
		MessageBox( _hwndMax, "No objects are selected!", szAppError, MB_OK );
		return 0;
	}

//	if ( numObjects > 1 )
//	{
//		MessageBox( _hwndMax, "Select one object to convert and try again", szAppError, MB_OK );
//		return 0;
//	}

	INode* pNode = gi->GetSelNode( 0 );
	if(!max2iffQuery(pNode ,szOutputObjectName))
		return false;
	// TO DO: check to see if directory exists [if not, prompt if user wants to create directory?]
	// ...

	ofstream out( szOutputObjectName, ios::out | ios::binary );
	if ( out.fail() )
	{
		MessageBox( GetActiveWindow(), szOutputObjectName, "Unable to create output file:", MB_OK );
		return false;
	}

	WRLExport iffExport;

	_IffWriter* pIff;
	if ( theOptions.bOutputBinary )
		pIff = new IffWriterBinary( out );
	else
		pIff = new IffWriterText( out );
	assert( pIff );

	for(int objectIndex=0;objectIndex<numObjects;++objectIndex)
		if ( !iffExport.ExportObject( gi->GetSelNode(objectIndex), pIff, &theOptions ) )
			return false;


//	// first output parent
//	if ( !iffExport.ExportObject( pNode, pIff, &theOptions ) )
//		return false;
//	// until no more children are found.
//	for (int c = 0; c < pNode->NumberOfChildren(); c++)
//	{
//		if (!iffExport.ExportObject(pNode->GetChildNode(c), pIff, &theOptions))
//			return false;
//	}

	return true;
}

//==============================================================================
// interface to attrib and toolbar

extern "C" bool
max2iff_Query( INode* pNode, const char* szOutputFilename_Input )
{
	assert( pNode );
	assert( szOutputFilename_Input );


	if(!max2iffQuery(pNode,szOutputFilename_Input))
		return false;

	// TO DO: check to see if directory exists [if not, prompt if user wants to create directory?]
	// ...
	ofstream out( szOutputObjectName, ios::out | ios::binary );
	if ( out.fail() )
	{
		MessageBox( GetActiveWindow(), szOutputObjectName, "Unable to create output file:", MB_OK );
		return false;
	}

	WRLExport iffExport;

	_IffWriter* pIff;
	if ( theOptions.bOutputBinary )
		pIff = new IffWriterBinary( out );
	else
		pIff = new IffWriterText( out );
	assert( pIff );

	bool bExported = bool( iffExport.ExportObject( pNode, pIff, &theOptions ) );

	return bExported;
}

//==============================================================================
// main entry point when called from another WF plugin (max2lev for example)
//==============================================================================

extern "C" bool
max2iff( INode* pNode, _IffWriter* iff, const max2iffOptions* options )
{
	assert( pNode );

	WRLExport* iffExport = new WRLExport;
	assert( iffExport );

	bool bExported = bool( iffExport->ExportObject( pNode, iff, options ) );
	delete iffExport;

	return bExported;
}
//==============================================================================
