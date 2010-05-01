// filename.cpp

#include <global.hp>
#include "buttons/button.h"
#include "buttons/filename.h"

void Error( const char* buf, ... );

const char*
CreateLocalDirName()
{
	static char szLocalDir[ _MAX_PATH ];
	strcpy( szLocalDir, theAttributes.ip->GetCurFilePath() );
	assert( strrchr( szLocalDir, '\\' ) );
	*( strrchr( szLocalDir, '\\' ) + 1 ) = '\0';

	return szLocalDir;
}


Filename::Filename( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
	hwndEdit = NULL;
	_edit = NULL;
	hwndBrowseButton = NULL;
}


Filename::~Filename()
{
	assert( hwndBrowseButton );
	DestroyWindow( hwndBrowseButton );

	assert( _edit );
	ReleaseICustEdit( _edit );

	assert( hwndEdit );
	DestroyWindow( hwndEdit );
}


int
Filename::storedDataSize() const
{
	assert( _edit );
	char buffer[ _MAX_PATH ];
	_edit->GetText( buffer, sizeof( buffer ) );
	return uiDialog::storedDataSize() + strlen( buffer ) + 1;
}


int
strnicmp_filename( const char * first, const char * last, size_t count )
{
	if ( count )
	{
		int f, l;
		do
		{
			f = tolower( *first++ );
			if ( f == '\\' ) f = '/';
			l = tolower( *last++ );
			if ( l == '\\' ) l = '/';
		} while ( --count && f && ( f == l ) );

		return f - l;
	}

	return 0;
}


bool
Filename::validReference( const char* szFilename ) const
{
#if 1
	return true;
#else
	const char* szLevelDirName = CreateLocalDirName();
	return bool( strnicmp_filename( szFilename, szLevelDirName, strlen( szLevelDirName ) ) == 0 );
#endif
}


dataValidation
Filename::copy_to_xdata( byte* & saveData )
{
	uiDialog::copy_to_xdata( saveData );

	assert( _td );
	assert( _edit );

	char buffer[ _MAX_PATH ];
	_edit->GetText( buffer, sizeof( buffer ) );
	strncpy( (char*)saveData, buffer, strlen( buffer ) + 1 );	// include NUL
	saveData += strlen( buffer ) + 1;

	return DATA_OK;
}


void
Filename::reset()
{
	assert( _td );
	reset( _td->string );
}


void
Filename::reset( char* str )
{
	assert( str );
	assert( _edit );
	char szFilename[ _MAX_PATH ];
	strcpy( szFilename, str );
	{
	const char* pEnd = szFilename + strlen( szFilename ) + 1;
	for ( char* p = szFilename; p != pEnd; ++p )
	{
		if ( *p == '\\' )
			*p = '/';
	}
	}

	_edit->SetText( szFilename );
}


void
Filename::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;
	reset( (char*)saveData );
	saveData += strlen( (char*)saveData ) + 1;
}


void
Filename::activate( HWND hwndButton )
{
	uiDialog::activate( hwndButton );
	if ( hwndButton == hwndBrowseButton )
	{
		char szFilename[ _MAX_PATH ];

		char szDrive[ _MAX_DRIVE ];
		char szPath[ _MAX_PATH ];
		char sztFilename[ _MAX_FNAME ];
		char szExt[ _MAX_EXT ];

		char szRelativeFilename[ _MAX_PATH ];
		assert( _edit );
		_edit->GetText( szRelativeFilename, sizeof( szFilename ) );

		sprintf( szFilename, "%s%s", CreateLocalDirName(), szRelativeFilename );
		_splitpath( szFilename, szDrive, szPath, sztFilename, szExt );
		_makepath( szFilename, NULL, NULL, sztFilename, szExt );
		char szInitialDir[ _MAX_PATH ] = { 0 };
		_makepath( szInitialDir, szDrive, szPath, NULL, NULL );
		{
		const char* pEnd = szInitialDir + strlen( szInitialDir ) + 1;
		for ( char* p = szInitialDir; p != pEnd; ++p )
		{
			if ( *p == '/' )
				*p = '\\';
		}
		}

		char szTitle[ _MAX_PATH ];
		assert( theAttributes._theSelection );
		sprintf( szTitle, "%s [%s]", _td->xdata.displayName, theAttributes._theSelection->GetName() );

		assert( _td );
		HWND _hwndMax = GetCOREInterface()->GetMAXHWnd();
		assert( _hwndMax );
		OPENFILENAME ofn = {
			sizeof( OPENFILENAME ),
			_hwndMax,			// hwndOwner
			hInstance,			// hInstance
			_td->lpstrFilter,	// lpstrFilter
			NULL,				// lpstrCustomFilter,
			0,					// nMaxCustFilter
			0,					// nFilterIndex
			szFilename,
			sizeof( szFilename ),
			NULL,				// lpstrFileTitle
			0,					// nMaxFileTitle
			szInitialDir,		// lpstrInitialDir
			szTitle,			// lpstrTitle
			OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,	// Flags	OFN_HIDEREADONLY
			0,					// nFileOffset
			0,					// nFileExtension
			NULL,				// lpStrDefExt,
			NULL,				// lCustData
			NULL,				// lpfnHook
			NULL				// lpTemplateName
		};

		if ( !*szInitialDir )
			strcpy( szInitialDir, theAttributes.szWorldFoundryDir );

		ofn.hInstance = ::hInstance;

		if ( GetOpenFileName( &ofn ) )
		{
#if 1
			char szCurFilePath[ _MAX_PATH ];
			strcpy( szCurFilePath, theAttributes.ip->GetCurFilePath() );

			assert( strrchr( szLocalDir, '\\' ) );

			_strlwr( szCurFilePath );
			_strlwr( ofn.lpstrFile );

			if ( *szCurFilePath != *(ofn.lpstrFile) )
			{	// Not on same drive, copy the whole thing
				reset( ofn.lpstrFile );
			}
			else
			{	// Calculate relative path from the current file

				char* pszCurFilePath = szCurFilePath;
				char* pszFilename = ofn.lpstrFile;

				char* pszOldCurFilePath;
				char* pszOldFilename;

				for ( ;; )
				{
					pszOldCurFilePath = pszCurFilePath;
					pszOldFilename = pszFilename;

					pszCurFilePath = strchr( pszCurFilePath+1, '\\' );
					pszFilename = strchr( pszFilename+1, '\\' );

					if ( !pszCurFilePath )
						break;

					if ( !pszFilename )
						break;

					int lenCurFilePath = pszCurFilePath - szCurFilePath;
					int lenFilename = pszFilename - ofn.lpstrFile;
					if ( lenCurFilePath != lenFilename )
						break;

					if ( strnicmp_filename( szCurFilePath, ofn.lpstrFile, lenFilename ) != 0 )
						break;
				}
				pszCurFilePath = pszOldCurFilePath;
				pszFilename = pszOldFilename;

				char szRelativePath[ _MAX_PATH ];
				char* pszRelativePath = szRelativePath;
				*pszRelativePath = '\0';

				// for each directory left in CurFilePath, copy "..\"
				char* pSeparator = pszCurFilePath;
				while ( pSeparator = strchr( pSeparator+1, '\\' ) )
					strcat( pszRelativePath, "..\\" );

				// copy remainder of ofn.lpstrFile [pszFilename]
				assert( *pszFilename == '\\' );
				++pszFilename;
				strcat( pszRelativePath, pszFilename );

				reset( szRelativePath );
			}
#else
			const char* szLevelDirName = CreateLocalDirName();
			if ( validReference( ofn.lpstrFile ) )
			{
				char* szObjectFilename = ofn.lpstrFile + strlen( szLevelDirName );
//				char* szObjectFilename = ofn.lpstrFile;
				reset( szObjectFilename );
			}
			else
				Error( "File \"%s\" not in game level directory (%s)", ofn.lpstrFile, szLevelDirName );
#endif
		}
	}
}


double
Filename::eval() const
{
	assert( _td );
	assert( _edit );

	char buffer[ _MAX_PATH ];
	_edit->GetText( buffer, sizeof( buffer ) );
	return *buffer ? 1.0 : 0.0;
}


bool
ProjectHasFilename()
{
	TSTR& path = theAttributes.ip->GetCurFilePath();

	return bool( *path );
}


bool
Filename::enable( bool bEnabled )
{
	if ( !ProjectHasFilename() )
		bEnabled = false;

	if ( uiDialog::enable( bEnabled ) )
	{
		assert( hwndEdit );
		Edit_Enable( hwndEdit, uiDialog::enable() );
		assert( hwndBrowseButton );
		Button_Enable( hwndBrowseButton, uiDialog::enable() );
		return true;
	}
	else
		return false;
}


int
Filename::make_dialog_gadgets( HWND hPanel )
{
	assert( _td );
	assert( hPanel );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	int x = 5 + wLabel;
	int width = LAST_BUTTON_WIDTH( x + 20 + 1 );
	hwndEdit = theAttributes.CreateWindowEx(
		0,
		"CustEdit",
		_td->name,
		WS_TABSTOP,
		x, theAttributes.y,
		width, 16,
		hPanel );
	assert( hwndEdit );
	_edit = GetICustEdit( hwndEdit );
	assert( _edit );

	hwndBrowseButton = theAttributes.CreateWindowEx(
		0,
		"BUTTON",
		"1",
		WS_TABSTOP,
		x + width + 1, theAttributes.y,
		20, 16,
		hPanel );
	assert( hwndBrowseButton );
	SetWindowLong( hwndBrowseButton, GWL_USERDATA, (LONG)this );
	assert( theAttributes._fontWingdings );
	SetWindowFont( hwndBrowseButton, theAttributes._fontWingdings, true );

	reset();

	return x + width + 1 + 16;
}
