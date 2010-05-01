#include "colour.h"
#include "oad.hpp"
#include <stdlib.h>
#include <string>

#include <windowsx.h>

#include "../lib/registry.h"
#include "../lib/wf_id.hp"

Colour theColour;

class ColourClassDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theColour;}
	const TCHAR *	ClassName() {return "Colour";}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Colour_ClassID;}
	const TCHAR* 	Category() {return _T("World Foundry");}
};

static ColourClassDesc appDataTestDesc;
ClassDesc* GetPropertiesDesc()
{
	return &appDataTestDesc;
}


void
Error( const char* szMsg )
{
	assert( szMsg );
	MessageBox( NULL, szMsg, appDataTestDesc.ClassName(), MB_OK );
}


BOOL CALLBACK
ColourDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_INITDIALOG:
			theColour.Init( hWnd );
			break;

		case WM_DESTROY:
			theColour.Destroy( hWnd );
			break;

		case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
					theColour.Apply();
					break;

				case IDC_LOAD:
					theColour.LoadScheme();
					break;

				case IDC_SAVE:
					theColour.SaveScheme();
					break;
			}
			break;
		}

#if MAX_RELEASE < 2000
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			if ( theColour.ip )
				theColour.ip->RollupMouseMessage( hWnd, msg, wParam, lParam );
			break;
#endif

		default:
			return FALSE;
	}
	return TRUE;
}


#include <io.h>

Colour::Colour()
{
	iu = NULL;
	ip = NULL;
	_hPanel = NULL;

	// Create list of class names
	_numClasses = 0;
	for ( int i=0; i<_MAX_CLASSES; ++i )
		_szClasses[ i ] = NULL;

	int success;
	success = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK, "OAD_DIR",
		szOadDir, sizeof( szOadDir ) );

	char output[1024];

	strcpy(output,"OAD_DIR not set in registry:");
	strcat(output,szRegWorldFoundryGDK);

	if ( !success )
		Error( output );

	strcpy( szColourSchemeInitialDir, szWorldFoundryDir );
	*szColourSchemeFilename = '\0';

	{ // Get classes available in the OAD directory (.oad files)
	char szOadFileSpec[ _MAX_PATH ];
	sprintf( szOadFileSpec, "%s/*.oad", szOadDir );

	_finddata_t findFile;
	long dirHandle = _findfirst( szOadFileSpec, &findFile );
	if ( dirHandle != -1 )
	{
		_szClasses[ _numClasses ] = _strdup( findFile.name );
		assert( _szClasses[ _numClasses ] );
		*( strrchr( _szClasses[ _numClasses ], '.' ) ) = '\0';
		_strlwr( _szClasses[ _numClasses ] );
		++_numClasses;

		while( _findnext( dirHandle, &findFile ) == 0 )
        {
			_szClasses[ _numClasses ] = _strdup( findFile.name );
			assert( _szClasses[ _numClasses ] );
			*( strrchr( _szClasses[ _numClasses ], '.' ) ) = '\0';
			_strlwr( _szClasses[ _numClasses ] );
			++_numClasses;
		}
	}
	}
}


Colour::~Colour()
{
	for ( int idxClass=0; idxClass<_numClasses; ++idxClass )
	{
		assert( _szClasses[ idxClass ] );
		free( _szClasses[ idxClass ] );
	}
}


class ClassColour
{
public:
	ClassColour( const char* szClassName );
	~ClassColour();
	void CreateGadget( int y );
	const char* ClassName() const;
	const char* ClassDescription() const;
	const COLORREF GetColour() const;
	bool isEnabled() const;
	void SetColour( COLORREF );

protected:
	HWND hwndLabel;
	COLORREF _colorRef;
	IColorSwatch* _color;
	HWND hwndColor;
	char* _szClassName;
	char* _szClassDescription;
};


bool
ClassColour::isEnabled() const
{
	return true;
}


const char*
ClassColour::ClassName() const
{
	assert( _szClassName );
	assert( *_szClassName );
	return _szClassName;
}


const char*
ClassColour::ClassDescription() const
{
	assert( _szClassDescription );
	return _szClassDescription;
}


const COLORREF
ClassColour::GetColour() const
{
	assert( _color );
	return _color->GetColor();
}


void
ClassColour::SetColour( COLORREF newColourRef )
{
	_colorRef = newColourRef;
	assert( _color );
	_color->SetColor( _colorRef );
}


ClassColour::ClassColour( const char* szClassName )
{
	oadHeader oadh;

	_szClassName = _strdup( szClassName );
	assert( _szClassName );

	char szOadFile[ _MAX_PATH ];
	sprintf( szOadFile, "%s/%s.oad", theColour.szOadDir, szClassName );
	FILE* fp = fopen( szOadFile, "rb" );
	assert( fp );
	int nBytes = fread( &oadh, 1, sizeof( oadh ), fp );
	assert( nBytes == sizeof( oadh ) );
	fclose( fp );

	_szClassDescription = (char*)malloc( strlen( oadh.name )
		+ strlen( " [" ) + strlen( ClassName() ) + strlen( ".oad]" )
		+ 1 );
	assert( _szClassDescription );
	strcpy( _szClassDescription, oadh.name );
	strcat( _szClassDescription, " [" );
	strcat( _szClassDescription, ClassName() );
	strcat( _szClassDescription, ".oad]" );
}


ClassColour::~ClassColour()
{
	assert( _szClassName );
	free( _szClassName );

	assert( _szClassDescription );
	free( _szClassDescription );

	assert( _color );
	ReleaseIColorSwatch( _color );

	assert( hwndColor );
	DestroyWindow( hwndColor );

	assert( hwndLabel );
	DestroyWindow( hwndLabel );
}


void
ClassColour::CreateGadget( int y )
{
	hwndColor = CreateWindowEx(
		0,
		"ColorSwatch",
		"",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		5,
		y,
		20,		// width
		16,			// height
		theColour._hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndColor );
	_colorRef = RGB(0,0,0);

	const char* szClassLabel = ClassDescription();

	_color = GetIColorSwatch( hwndColor, _colorRef, (char*)szClassLabel );
	assert( _color );

	hwndLabel = CreateWindowEx(
		0,
#if defined( TOOLTIPS )
		"CustButton",
#else
		"STATIC",
#endif
		szClassLabel,
		WS_CHILD | WS_VISIBLE,
		5 + 20 + 5,
		y,
		0,
		16,
		theColour._hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndLabel );
	SetWindowFont( hwndLabel, theColour._font, TRUE );

	SIZE size;
	// get the DC for the label
	HDC hdc = GetDC( hwndLabel );
	HFONT hOldFont = (HFONT)SelectObject( hdc, theColour.ip->GetIObjParam()->GetAppHFont() );
	GetTextExtentPoint32( hdc, szClassLabel, strlen( szClassLabel ), &size );
	SelectObject( hdc, hOldFont );
	ReleaseDC( hwndLabel, hdc );
	SetWindowPos( hwndLabel, HWND_TOP, 0, 0, size.cx, 16, SWP_NOMOVE );

#if defined( TOOLTIPS )
	_label = GetICustButton( hwndLabel );
	assert( _label );
	if ( *_td->helpMessage )
		_label->SetTooltip( TRUE, _td->helpMessage );
#endif

	char szrgbColour[ 100 ];
	bool bSuccess;
	bSuccess = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\Colour",
		_szClassName, szrgbColour, sizeof( szrgbColour ) );
	if ( bSuccess )
	{
		sscanf( szrgbColour, "%08x", &_colorRef );
		assert( _color );
		_color->SetColor( _colorRef );
	}
}


void
Colour::LoadColourSchemeFile()
{
	FILE* fp = fopen( szColourSchemeFilename, "rt" );
	assert( fp );
	while ( !feof( fp ) )
	{
		char szLine[ _MAX_PATH * 2 ];
		char szClassName[ _MAX_PATH ];
		int r, g, b;
		fgets(  szLine, sizeof( szLine ), fp );
		int nItems = sscanf( szLine, "%s %d %d %d\n", szClassName, &r, &g, &b );
		assert( nItems == 4 );
		for ( int i=0; i<_numClasses; ++i )
		{
			ClassColour* c = _class[ i ];
			assert( c );
			if ( c->isEnabled && ( _stricmp( szClassName, c->ClassName() ) == 0 ) )
			{
				COLORREF col = RGB( r, g, b );
				c->SetColour( col );
			}
		}
	}
	fclose( fp );
}



void
Colour::LoadScheme()
{
	static OPENFILENAME ofn =
		{
		sizeof( OPENFILENAME ),
		NULL,	// hwndOwner
		NULL,	// hInstance
		NULL,	// lpstrFilter
		NULL,	// lpstrCustomFilter,
		0,		// nMaxCustFilter
		0,		// nFilterIndex
		szColourSchemeFilename,
		sizeof( szColourSchemeFilename ),
		NULL,	// lpstrFileTitle
		0,		// nMaxFileTitle
		szColourSchemeInitialDir,	// lpstrInitialDir
		"Load Colour Scheme",	// lpstrTitle
		OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,	// Flags
		0,		// nFileOffset
		0,		// nFileExtension
		NULL,	// lpStrDefExt,
		NULL,	// lCustData
		NULL,	// lpfnHook
		NULL	// lpTemplateName
		};

	assert( hInstance );
	ofn.hInstance = hInstance;

	if ( GetOpenFileName( &ofn ) )
		LoadColourSchemeFile();
}


void
Colour::SaveScheme()
{
	static OPENFILENAME ofn =
		{
		sizeof( OPENFILENAME ),
		NULL,	// hwndOwner
		NULL,	// hInstance
		NULL,	// lpstrFilter
		NULL,	// lpstrCustomFilter,
		0,		// nMaxCustFilter
		0,		// nFilterIndex
		szColourSchemeFilename,
		sizeof( szColourSchemeFilename ),
		NULL,	// lpstrFileTitle
		0,		// nMaxFileTitle
		szColourSchemeInitialDir,	// lpstrInitialDir
		"Save Colour Scheme",	// lpstrTitle
		OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,	// Flags
		0,		// nFileOffset
		0,		// nFileExtension
		NULL,	// lpStrDefExt,
		NULL,	// lCustData
		NULL,	// lpfnHook
		NULL	// lpTemplateName
		};

	assert( hInstance );
	ofn.hInstance = hInstance;

	if ( GetSaveFileName( &ofn ) )
	{
		FILE* fp = fopen( szColourSchemeFilename, "wt" );
		assert( fp );
		for ( int i=0; i<_numClasses; ++i )
		{
			ClassColour* c = _class[ i ];
			assert( c );
			if ( c->isEnabled() )
				fprintf( fp, "%s %d %d %d\n", c->ClassName(), GetRValue( c->GetColour() ),
					GetGValue( c->GetColour() ), GetBValue( c->GetColour() ) );
		}
		fclose( fp );
	}
}


int
fnSortDescription( const void* p1, const void* p2 )
{
	assert( p1 );
	const char* szFilename1 = (*(ClassColour**)p1)->ClassDescription();
	assert( p2 );
	const char* szFilename2 = (*(ClassColour**)p2)->ClassDescription();

	return strcmp( szFilename1, szFilename2 );
}


void
Colour::BeginEditParams( Interface* ip, IUtil* iu )
{
	this->iu = iu;
	this->ip = ip;

	assert( ip );
	_font = ip->GetIObjParam()->GetAppHFont();
	assert( _font );

	assert( !_hPanel );
	assert( ip );
	assert( hInstance );

	assert( !_hPanel );
	_hPanel = theColour.ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_COLOUR),
		ColourDlgProc,
		"Colour by Class",
		0 );
	assert( _hPanel );

	for ( int idxClass=0; idxClass<_numClasses; ++idxClass )
	{
		_class[ idxClass ] = new ClassColour( _szClasses[idxClass] );
		assert( _class[ idxClass ] );
	}

	qsort( (void*)_class, _numClasses, sizeof( _class[0] ), fnSortDescription );

	for ( idxClass=0; idxClass<_numClasses; ++idxClass )
	{
		assert( _class[ idxClass ] );
		_class[ idxClass ]->CreateGadget( 60 + idxClass*18 );
	}

	if ( *szColourSchemeFilename )
		LoadColourSchemeFile();
}


void
Colour::EndEditParams( Interface* ip, IUtil* iu )
{
	int i;

	for ( i=0; i<_numClasses; ++i )
	{
		char szrgbColour[ 100 ];
		sprintf( szrgbColour, "%08x",  theColour._class[i]->GetColour() );
		bool bSuccess;
		bSuccess = SetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\Colour",
			theColour._szClasses[ i ], szrgbColour );
		assert( bSuccess );
	}

	for ( i=0; i<_numClasses; ++i )
	{
		assert( _class[ i ] );
		delete _class[ i ];
	}
	assert( _hPanel );
	theColour.ip->DeleteRollupPage( _hPanel );
	_hPanel = NULL;

	this->iu = NULL;
	this->ip = NULL;
}


void
Colour::SelectionSetChanged( Interface* ip, IUtil* iu )
{
}


void
Colour::Init( HWND hWnd )
{ // Called on creation of each rollup page
}


void
Colour::Destroy( HWND hWnd )
{
}


extern int hdump( HINSTANCE, HWND, void* data, int size );

void
Colour::Apply()
{
	assert( ip );

	INode* root = ip->GetRootNode();
	assert( root );

//	OutputDebugString( root->GetName() );
//	OutputDebugString( "\n" );

	int nChildren = root->NumberOfChildren();

	for ( int i=0; i<nChildren; ++i )
	{
		INode* child = root->GetChildNode( i );
		assert( child );
		//OutputDebugString( "\t" );
		//OutputDebugString( child->GetName() );
		//OutputDebugString( "\t" );
		AppDataChunk* adClass = child->GetAppDataChunk( Attrib_ClassID, UTILITY_CLASS_ID, Attributes::SLOT_CLASS );
		char* szClassName = adClass ? (char*)adClass->data : "disabled";
		assert( *szClassName );

		for ( int cl=0; cl<_numClasses; ++cl )
		{
			ClassColour* c = _class[ cl ];
			assert( c );
			if ( c->isEnabled() && ( _stricmp( szClassName, c->ClassName() ) == 0 ) )
			{
				child->SetWireColor( c->GetColour() );
				break;
			}
		}
	}

	for ( i=0; i<_numClasses; ++i )
	{
		char szrgbColour[ 100 ];
		sprintf( szrgbColour, "%08x",  theColour._class[i]->GetColour() );
		bool bSuccess;
		bSuccess = SetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\Colour",
			theColour._szClasses[ i ], szrgbColour );
		assert( bSuccess );
	}

	ip->ForceCompleteRedraw();
}
