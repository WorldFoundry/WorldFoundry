#include <max.h>
#include "toolbar.h"
#include <direct.h>
#include <windowsx.h>
#include "../lib/levelnum.h"
#include "../lib/objname.h"
#include "../lib/loaddll.h"
#include "../lib/loaddll.h"
#include "../lib/registry.h"

#define NUM_ITEMS( _array_ )	( sizeof( (_array_) ) / sizeof( *(_array_) ) )


const char szPerspectiveCorrect[] = "PerspectiveCorrect";
const char szParanoid[] = "Paranoid";
const char szSingleSided[] = "Single Sided Polygons";
const char szShowBoxes[] = "Show Boxes";
const char szShowPositions[] = "Show Positions";
const char szNoInitialRotation[] = "No Initial Rotation";
//const char szSubdivide[] = "Polygon Subdivision";
const char szPlaystation[] = "Sony Playstation";
const char szDebug[] = "Debug";
const char szWindow[] = "Run In Window";
const char szFrameRate[] = "Simulated Frame Rate";
const char szHeight[] = "Display Height";
const char szCommandOtherParams[] = "Other Command Line Parameters";
const char szLevelNumber[] = "Level #";
const char szRunAfterMake[] = "Run game after make";

StreamDestination streamDestination[ 14 ] = {
	{ "null", "n" },
	{ "stdout", "s" },
	{ "stderr", "e" },
	{ "screen", "c" },
	{ "mono #0", "m0" },
	{ "mono #1", "m1" },
	{ "mono #2", "m2" },
	{ "mono #3", "m3" },
	{ "mono #4", "m4" },
	{ "mono #5", "m5" },
	{ "mono #6", "m6" },
	{ "mono #7", "m7" },
	{ "mono #8", "m8" },
	{ "mono #9", "m9" }
//	{ "filename" "" }
};

DebuggingStream idComboStreams[ 23 ] = {
	{ IDC_WARNINGS, "Streams.Warnings", "-pw" },
	{ IDC_ERRORS, "Streams.Errors", "-pe" },
	{ IDC_FATAL, "Streams.Fatal", "-pf" },
	{ IDC_STATISTICS, "Streams.Statistics", "-ps" },
	{ IDC_PROGRESS, "Streams.Progress", "-pp" },
	{ IDC_DEBUGGING, "Streams.Debugging", "-pd" },
	//
	{ IDC_ACTOR, "Streams.Actor", "-sa" },
	{ IDC_MOVEMENT, "Streams.Movement", "-sm" },
	{ IDC_COLLISION, "Streams.Collision", "-sc" },
	{ IDC_ROOM, "Streams.Room", "-sr" },
	{ IDC_FLOW, "Streams.Flow", "-sf" },
	{ IDC_AI, "Streams.AI", "-si" },
	{ IDC_LEVEL, "Streams.Level", "-sl" },
	{ IDC_LOGO, "Streams.Logo", "-so" },
	{ IDC_TOOL, "Streams.Tool", "-st" },
	{ IDC_CAMERA, "Streams.Camera", "-se" },
	{ IDC_TEXTURE, "Streams.Texture", "-su" },
	{ IDC_MEMORY, "Streams.Memory", "-sM" },
	{ IDC_SCRIPT, "Streams.Script", "-sS" },
	{ IDC_ASSETID, "Streams.AssetID", "-sA" },
	{ IDC_STREAMING, "Streams.Streaming", "-ss" },
	{ IDC_MAILBOX, "Streams.Mailbox", "-sx" },
	{ IDC_FRAMEINFO, "Streams.FrameInfo", "-sn" }
};


Toolbar theToolbar;

class ToolbarClassDesc : public ClassDesc
{
public:
	int 			IsPublic() { return 1; }
	void*			Create( BOOL loading = FALSE ) { return &theToolbar; }
	const TCHAR*	ClassName() { return Toolbar_ClassName; }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return Toolbar_ClassID; }
	const TCHAR* 	Category() { return _T( "World Foundry" ); }

#if 0
	BOOL			HasClassParams()	{ return TRUE; }
	void			ResetClassParams()	{ theToolbar._setButtons_Level(); }
#endif
};

static ToolbarClassDesc appDataTestDesc;
ClassDesc* GetPropertiesDesc() {return &appDataTestDesc;}


void
Error( const char* szMsg )
{
	assert( szMsg );
	HWND hwnd = GetCOREInterface() ? GetCOREInterface()->GetMAXHWnd() : 0;
	MessageBox( hwnd, szMsg, GetPropertiesDesc()->ClassName(), MB_OK );
}


BOOL CALLBACK
ToolbarDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_INITDIALOG:
			theToolbar.Init( hWnd );
			break;

		case WM_DESTROY:
			theToolbar.Destroy( hWnd );
			break;

		case WM_COMMAND:
			theToolbar.Command( hWnd, LOWORD( wParam ) );
			break;

#if 0
		case WM_MOUSEWHEEL:
		{
			WORD fwKeys = LOWORD(wParam);	// key flags
			int zDelta = HIWORD(wParam);	// wheel rotation
			int xPos = LOWORD(lParam);		// horizontal position of pointer
			int yPos = HIWORD(lParam);		// vertical position of pointer

			DebugPrint( "WM_MOUSEWHEEL\n" );

			break;
		}
#endif

#if 0
		case WM_NOTIFY:
			if ( ( (LPNMHDR)lParam)->code == TTN_NEEDTEXT )
			{
				LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lParam;
				switch ( lpttt->hdr.idFrom )
				{
					case IDC_WIN:
						lpttt->lpszText = _T("Windows 95");
						break;
					case IDC_PSX:
						lpttt->lpszText = _T("Sony Playstation");
						break;
				}
			}
			break;
#endif

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			if ( theToolbar.ip )
				theToolbar.ip->RollupMouseMessage( hWnd, msg, wParam, lParam );
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


#include <io.h>

void
UpdateButtonsCallback::proc( Interface* )
{
	theToolbar._setButtons_Level();
}


Toolbar::Toolbar()
{
	iu = NULL;
	ip = NULL;
	_hPanel = 0;
	_hPanelButtons = 0;
	_hPanelParameters = 0;
	_hPanelDebugStreams = 0;

	int success;
	success = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK, "WORLD_FOUNDRY_DIR",
		szWorldFoundryDir, sizeof( szWorldFoundryDir ) );
	if ( !success )
		Error( "WORLD_FOUNDRY_DIR not set in registry" );

#if 0
	// No longer needed -- Toolbar now figures everything out itself
	success = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK, "LEVELS_DIR",
		szLevelsDir, sizeof( szLevelsDir ) );
	if ( !success )
		Error( "Recombinant/World Foundry/GDK/LEVELS_DIR not set in registry" );
#endif
}


Toolbar::~Toolbar()
{
}


int
Toolbar::ReadCheckBox( HWND hwnd, const char* szKey, UINT button )
{
	assert( hwnd );

	char szBuffer[ 256 ];
	// Read in values
	int bSuccess;
	bSuccess = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\ToolBar", szKey, szBuffer, sizeof( szBuffer ) );
	int i = atoi( szBuffer );
	if ( bSuccess )
		Button_SetCheck( GetDlgItem( hwnd, button ), i );

	return i;
}


int
Toolbar::ReadRadioButton( HWND hwnd, const char* szKey, UINT button, UINT buttonDefault )
{
	assert( hwnd );

	char szBuffer[ 256 ];
	// Read in values
	int bSuccess;
	bSuccess = GetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\ToolBar", szKey, szBuffer, sizeof( szBuffer ) );
	if ( bSuccess && atoi( szBuffer ) )
	{
		Button_SetCheck( GetDlgItem( hwnd, button ), TRUE );
		SendMessage( _hPanelParameters, WM_COMMAND, button, 0 );
		return button;
	}
	else
	{
		Button_SetCheck( GetDlgItem( hwnd, buttonDefault ), TRUE );
		SendMessage( _hPanelParameters, WM_COMMAND, buttonDefault, 0 );
		return buttonDefault;
	}
}


int
Toolbar::ReadInteger( HWND hwnd, const char* szKey, UINT button )
{
	assert( hwnd );

	char szBuffer[ 256 ];
	// Read in values
	int bSuccess;
	bSuccess = GetLocalMachineStringRegistryEntry(
		szRegWorldFoundryGDK "\\ToolBar", szKey,
		szBuffer, sizeof( szBuffer ) );
	if ( bSuccess )
		ComboBox_SetText( GetDlgItem( hwnd, button ), szBuffer );

	return atoi( szBuffer );
}


void
Toolbar::ReadString( HWND hwnd, const char* szKey, UINT button )
{
	assert( hwnd );

	ReadInteger( hwnd, szKey, button );
}


void
Toolbar::SaveCheckBox( HWND hwnd, const char* szKey, UINT button )
{
	assert( hwnd );

	HWND hwndButton = GetDlgItem( hwnd, button );
	assert( hwndButton );

	char szBuffer[ 256 ];
	sprintf( szBuffer, "%d", Button_GetCheck( hwndButton ) );

	bool bSuccess;
	bSuccess = SetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\ToolBar", szKey, szBuffer );
	assert( bSuccess );
}


void
Toolbar::SaveRadioButton( HWND hwnd, const char* szKey, UINT button )
{
	assert( hwnd );

	HWND hwndButton = GetDlgItem( hwnd, button );
//	if ( !hwndButton )
//		return;

	assert( hwndButton );
	char szBuffer[ 256 ];
	sprintf( szBuffer, "%d", Button_GetCheck( hwndButton ) );

	bool bSuccess;
	bSuccess = SetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\ToolBar", szKey, szBuffer );
	assert( bSuccess );
}


void
Toolbar::SaveInteger( HWND hwnd, const char* szKey, UINT button )
{
	assert( hwnd );

	HWND hwndButton = GetDlgItem( hwnd, button );
	assert( hwndButton );

	char szBuffer[ 256 ];
	Edit_GetText( hwndButton, szBuffer, sizeof( szBuffer ) );

	bool bSuccess;
	bSuccess = SetLocalMachineStringRegistryEntry( szRegWorldFoundryGDK "\\ToolBar", szKey, szBuffer );
	assert( bSuccess );
}


void
Toolbar::SaveString( HWND hwnd, const char* szKey, UINT button )
{
	assert( hwnd );

	SaveInteger( hwnd, szKey, button );
}



void
Toolbar::BeginEditParams( Interface* ip, IUtil* iu )
{
	this->iu = iu;
	this->ip = ip;

	assert( ip );
	_font = ip->GetIObjParam()->GetAppHFont();
	assert( _font );

	assert( !_hPanel );
	assert( ip );
	assert( hInstance );

	ip->RegisterRedrawViewsCallback( &buttonsProc );

	// BUTTONS PANEL
	assert( !_hPanelButtons );
	_hPanelButtons = theToolbar.ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE( IDD_TOOLBAR_BUTTONS ),
		(DLGPROC)ToolbarDlgProc,
		"Commands" );
	assert( _hPanelButtons );

	// TARGET PLATFORM PANEL
	assert( !_hPanel );
	_hPanel = theToolbar.ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE( IDD_TARGETPLATFORM ),
		(DLGPROC)ToolbarDlgProc,
		"Target Platform",
		NULL,
		APPENDROLL_CLOSED );
	assert( _hPanel );


	// PARAMETERS PANEL
	_hPanelParameters = theToolbar.ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE( IDD_COMMAND_LINE_PARAMETERS ),
		(DLGPROC)ToolbarDlgProc,
		"Game Execution Parameters" );
	assert( _hPanelParameters );

	_hPanelDebugStreams = theToolbar.ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE( IDD_DEBUG_STREAMS ),
		(DLGPROC)ToolbarDlgProc,
		"Debugging Streams",
		NULL,
		APPENDROLL_CLOSED );
	assert( _hPanelDebugStreams );


	// TODO: Enumerate display mode DirectDraw supports for video card
	HWND hwndHeight = GetDlgItem( _hPanelParameters, IDC_HEIGHT );
	assert( hwndHeight );
	ComboBox_AddString( hwndHeight, "200" );
	ComboBox_AddString( hwndHeight, "240" );
	ComboBox_AddString( hwndHeight, "384" );
	ComboBox_AddString( hwndHeight, "400" );
	ComboBox_AddString( hwndHeight, "480" );
	ComboBox_AddString( hwndHeight, "512" );
	ComboBox_AddString( hwndHeight, "640" );
	ComboBox_AddString( hwndHeight, "800" );
	ComboBox_AddString( hwndHeight, "1024" );
	ComboBox_AddString( hwndHeight, "1152" );
	ComboBox_AddString( hwndHeight, "1200" );
	ComboBox_AddString( hwndHeight, "1600" );

	{ // Debugging streams
	for ( int i=0; i<NUM_ITEMS( idComboStreams ); ++i )
	{
		HWND hwndStreamType = GetDlgItem( _hPanelDebugStreams, idComboStreams[i].id );
		assert( hwndStreamType );

		for ( int j=0; j<NUM_ITEMS( streamDestination ); ++j )
			ComboBox_AddString( hwndStreamType, streamDestination[j].name );

		ReadString( _hPanelDebugStreams, idComboStreams[i].name, idComboStreams[i].id );
	}
	}

	// Framerate
	HWND hwndFrameRate = GetDlgItem( _hPanelParameters, IDC_FRAMERATE );
	assert( hwndFrameRate );
	ComboBox_AddString( hwndFrameRate, "10" );
	ComboBox_AddString( hwndFrameRate, "15" );
	ComboBox_AddString( hwndFrameRate, "20" );
	ComboBox_AddString( hwndFrameRate, "30" );
	ComboBox_AddString( hwndFrameRate, "60" );

//	ReadString( _hPanelParameters, szCommandOtherParams, IDC_OTHERPARAMS );
	ReadInteger( _hPanelParameters, szFrameRate, IDC_FRAMERATE );
	ReadInteger( _hPanelParameters, szHeight, IDC_HEIGHT );
	ReadCheckBox( _hPanelParameters, szPerspectiveCorrect, IDC_PERSPECTIVECORRECT );
	ReadCheckBox( _hPanelParameters, szParanoid, IDC_PARANOID );
//	ReadCheckBox( _hPanelParameters, szShowBoxes, IDC_SHOWBOXES );
	ReadCheckBox( _hPanelParameters, szShowPositions, IDC_SHOWPOSITIONS );
	ReadCheckBox( _hPanelParameters, szNoInitialRotation, IDC_NOINITIALROTATION );
//	ReadCheckBox( _hPanelParameters, szSound, IDC_SOUND );
//	ReadCheckBox( _hPanelParameters, szSubdivide, IDC_SUBDIVISION );
	ReadRadioButton( _hPanel, szPlaystation, IDC_PSX, IDC_WIN );
	ReadRadioButton( _hPanelParameters, szDebug, IDC_DEBUG, IDC_RELEASE );
//	ReadRadioButton( _hPanelParameters, szWindow, IDC_WINDOW, IDC_FULLSCREEN );

	ReadCheckBox( _hPanel, szRunAfterMake, IDC_RUN_AFTER_MAKE );
	ReadInteger( _hPanelParameters, szLevelNumber, IDC_LEVEL_NUMBER );

	for ( int i=0; i<NUM_ITEMS( idComboStreams ); ++i )
		Button_Enable( GetDlgItem( _hPanelDebugStreams, idComboStreams[i].id ), TRUE );

	Button_Enable( GetDlgItem( _hPanelParameters, IDC_FRAMERATE ), TRUE );
	Command( _hWnd, IDC_WIN );		// or IDC_PSX

	_setButtons_Level();
}


void
Toolbar::EndEditParams( Interface* ip, IUtil* iu )
{
	assert( _hPanel );
	theToolbar.ip->DeleteRollupPage( _hPanel );
	_hPanel = 0;

	assert( _hPanelButtons );
	theToolbar.ip->DeleteRollupPage( _hPanelButtons );
	_hPanelButtons = 0;

	assert( _hPanelParameters );
	theToolbar.ip->DeleteRollupPage( _hPanelParameters );
	_hPanelParameters = 0;

	assert( _hPanelDebugStreams );
	theToolbar.ip->DeleteRollupPage( _hPanelDebugStreams );
	_hPanelDebugStreams = 0;

	ip->UnRegisterRedrawViewsCallback( &buttonsProc );

	this->iu = NULL;
	this->ip = NULL;
}


void
Toolbar::_setButtons_Level()
{
	Interface* ip = GetCOREInterface();
	assert( ip );

	const char* szLevelName = GetLevelDir();
	bool bValidLevel = bool( szLevelName );

	char szMakeLevel[ 100 ];
	char szRunLevel[ 100 ];
	char szCleanLevel[ 100 ];
	char szLintLevel[ 100 ];
	if ( bValidLevel )
	{	// Valid level number
		if ( ReadCheckBox( _hPanel, szRunAfterMake, IDC_RUN_AFTER_MAKE ) )
			sprintf( szMakeLevel, "Make && Run \"%s\"", szLevelName );
		else
			sprintf( szMakeLevel, "Make \"%s\"", szLevelName );
		sprintf( szRunLevel, "Run \"%s\"", szLevelName );
		sprintf( szCleanLevel, "Clean \"%s\"", szLevelName );
		sprintf( szLintLevel, "Lint \"%s\"", szLevelName );
	}
	else
	{	// Error
		strcpy( szMakeLevel, "Make: [ No Level Loaded ]" );
		strcpy( szRunLevel, "Run: [ No Level Loaded ]" );
		strcpy( szCleanLevel, "Clean: [ No Level Loaded ]" );
		strcpy( szLintLevel, "Lint: [ No Level Loaded ]" );
	}
	HWND hwndMakeLevel = GetDlgItem( _hPanelButtons, IDC_MAKE_LEVEL );
	assert( hwndMakeLevel );
	Button_SetText( hwndMakeLevel, szMakeLevel );
	Button_Enable( hwndMakeLevel, bValidLevel );

	HWND hwndRunLevel = GetDlgItem( _hPanelButtons, IDC_RUN_LEVEL );
	assert( hwndRunLevel );
	Button_SetText( hwndRunLevel, szRunLevel );
	Button_Enable( hwndRunLevel, bValidLevel );

	HWND hwndCleanLevel = GetDlgItem( _hPanelButtons, IDC_CLEAN_LEVEL );
	assert( hwndCleanLevel );
	Button_SetText( hwndCleanLevel, szCleanLevel );
	Button_Enable( hwndCleanLevel, bValidLevel );

	HWND hwndExportObject = GetDlgItem( _hPanelButtons, IDC_EXPORT_OBJECT );
	assert( hwndExportObject );

	HWND hwndLintLevel = GetDlgItem( _hPanelButtons, IDC_LINT_LEVEL );
	assert( hwndLintLevel );
	Button_SetText( hwndLintLevel, szLintLevel );
	Button_Enable( hwndLintLevel, false );

	char szExportObject[ 100 ];
	int nObjectsSelected = ip->GetSelNodeCount();
	if ( nObjectsSelected == 0 )
	{
		strcpy( szExportObject, "[ No Objects Selected ]" );
	}
	else if ( nObjectsSelected == 1 )
	{
		INode* pNode = ip->GetSelNode( 0 );
		assert( pNode );
		sprintf( szExportObject, "Export Object \"%s\"", pNode->GetName() );
	}
	else
	{
		sprintf( szExportObject, "Export %d Selected Objects", nObjectsSelected );
	}
	Button_SetText( hwndExportObject, szExportObject );

	TSTR szLevelFilename = ip->GetCurFileName();
	assert( szLevelFilename );
	Button_Enable( hwndExportObject, *szLevelFilename && ( nObjectsSelected != 0 ) );
}


void
Toolbar::SelectionSetChanged( Interface* ip, IUtil* iu )
{
	_setButtons_Level();
}


void
Toolbar::Init( HWND hWnd )
{	// Called on creation of each rollup page (the whole thing)
	assert( hWnd );
	_hWnd = hWnd;
}


void
Toolbar::Destroy( HWND hWnd )
{
}


#if 0
void
rm( const char* szDir )
{
	char szCommand[ _MAX_PATH * 2 ];

	const char szStartCommand[] = "cmd.exe /C del /S /F /Q ";

	strcpy( szCommand, szStartCommand );
	strcat( szCommand, szDir );

	for ( char* pszCommand = szCommand + strlen( szStartCommand );
	pszCommand != szCommand + strlen( szCommand ); ++pszCommand )
	{
		if ( *pszCommand == '/' )
			*pszCommand = '\\';
	}
#if 0
	if ( szCommand[ strlen( szCommand ) - 1 ] != '\\' )
		strcat( szCommand, "\\" );
	strcat( szCommand, "*.*" );
#endif

	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo = { 0 };
	StartupInfo.cb = sizeof( StartupInfo );

	if ( CreateProcess(
			NULL,								// LPCTSTR lpApplicationName
			szCommand,							// LPTSTR lpCommandLine
			NULL,				// LPSECURITY_ATTRIBUTES lpProcessToolbar
			NULL,				// LPSECURITY_ATTRIBUTES lpThreadToolbar
			FALSE,				// BOOL bInheritHandles
			0,					// DWORD dwCreationFlags
			NULL,				// LPVOID lpEnvironment
			szDir,				// LPCTSTR lpCurrentDirectory
			&StartupInfo,		// LPSTARTUPINFO lpStartupInfo
			&ProcessInfo ) )	// LPPROCESS_INFORMATION lpProcessInformation
		WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
}
#endif


#if 0

void
Toolbar::CleanLevel()
{
	// Make szLevelDir[]
	char szLevelDir[ _MAX_PATH ];
	strcpy( szLevelDir, CreateLevelDirName() );
	assert( *szLevelDir );

	// run Opus MAKE
	char szCommand[ _MAX_PATH * 2 ];

	strcpy( szCommand, "make.exe -nologo clean" );

	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo = { 0 };
	StartupInfo.cb = sizeof( StartupInfo );

	if ( CreateProcess(
			NULL,								// LPCTSTR lpApplicationName
			szCommand,							// LPTSTR lpCommandLine
			NULL,				// LPSECURITY_ATTRIBUTES lpProcessToolbar
			NULL,				// LPSECURITY_ATTRIBUTES lpThreadToolbar
			FALSE,				// BOOL bInheritHandles
			0,					// DWORD dwCreationFlags
			NULL,				// LPVOID lpEnvironment
			szLevelDir,			// LPCTSTR lpCurrentDirectory
			&StartupInfo,		// LPSTARTUPINFO lpStartupInfo
			&ProcessInfo ) )	// LPPROCESS_INFORMATION lpProcessInformation
		WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
	else
		Error( "Unable to start Opus MAKE for \"clean\"" );
}


int
mkdir_recursive( const char* _szPath )
{
	assert( _szPath );

	// Convert entire path to use '/' instead of '\'
	char szPath[ MAX_PATH ];
	strcpy( szPath, _szPath );
	for ( int i=0; i<strlen( szPath ); ++i )
		if ( szPath[ i ] == '\\' )
			szPath[ i ] = '/';

	char* szEndOfPath = szPath;
	do
	{
		szEndOfPath = strchr( szEndOfPath, '/' );
		if ( szEndOfPath )
		{
			*szEndOfPath = '\0';
			_mkdir( szPath );
			*szEndOfPath = '/';
			++szEndOfPath;
		}
	}
	while ( szEndOfPath );
	_mkdir( szPath );

	return 0;
}


void
Toolbar::MakeLevel()
{
	const char* szLevelDir = GetLevelDir();
	if ( !szLevelDir )
		return;

#if 0
	// TODO: add to "globals"
	char worldFoundryDir[_MAX_PATH];
	if(!GetLocalMachineStringRegistryEntry("Software\\Recombinant\\World Foundry\\GDK","WORLD_FOUNDRY_DIR",worldFoundryDir,sizeof( worldFoundryDir ) ))
		assert( "WORLD_FOUNDRY_DIR Registry Entry is missing" );
#endif

	char szPlaystation[ _MAX_PATH ];
	GetLocalMachineStringRegistryEntry(
		"Software\\Recombinant\\World Foundry\\GDK" "\\ToolBar",
		"Sony Playstation",
		szPlaystation,
		sizeof( szPlaystation ) );

	char* szTarget = atoi( szPlaystation ) ? "psx" : "win";
	char szOutputDirectory[ _MAX_PATH ];
	char szOutputFilename[ _MAX_PATH ];
	sprintf( szOutputDirectory, "%s/assets/%s/%s", szWorldFoundryDir, szTarget, szLevelDir );
	sprintf( szOutputFilename, "%s/%s.lvl", szOutputDirectory, szLevelDir );

	mkdir_recursive( szOutputDirectory );

	// LevelCon
	HINSTANCE hLevelConInst = LoadMaxLibrary( MAX2LVL_PlugIn );
	assert( hLevelConInst );

	EXPORT_LEVEL_PROC fnLevelCon = (EXPORT_LEVEL_PROC)GetProcAddress( hLevelConInst, "levelcon" );
	assert( fnLevelCon );

	int success = fnLevelCon( szOutputFilename );

	FreeLibrary( hLevelConInst );

	if ( success == 0 )
		return;

	// Make szLevelDir[]
	char szSourceLevelDir[ _MAX_PATH ];
	strcpy( szSourceLevelDir, CreateLevelDirName() );
	assert( *szSourceLevelDir );

	{ // build Makefile for assets (buildlvl.bat)
	char szCommand[ _MAX_PATH * 2 ];

	strcpy( szCommand, szSourceLevelDir );
	strcat( szCommand, "../buildlvl.bat " );
	bool bPlaystation = Button_GetCheck( GetDlgItem( _hPanel, IDC_PSX ) );
	strcat( szCommand, bPlaystation ? "psx" : "win" );

	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo = { 0 };
	StartupInfo.cb = sizeof( StartupInfo );
	char szTitle[ 1024 ];
	sprintf( szTitle, "Analyzing files and building Makefile for \"%s\"", szLevelDir );
	StartupInfo.lpTitle = szTitle;

	//buildlvl.bat
	if ( CreateProcess(
			NULL,								// LPCTSTR lpApplicationName
			szCommand,							// LPTSTR lpCommandLine
			NULL,				// LPSECURITY_ATTRIBUTES lpProcessToolbar
			NULL,				// LPSECURITY_ATTRIBUTES lpThreadToolbar
			FALSE,				// BOOL bInheritHandles
			0,					// DWORD dwCreationFlags
			NULL,				// LPVOID lpEnvironment
			szSourceLevelDir,			// LPCTSTR lpCurrentDirectory
			&StartupInfo,		// LPSTARTUPINFO lpStartupInfo
			&ProcessInfo ) )	// LPPROCESS_INFORMATION lpProcessInformation
		WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
	else
	{
		Error( "Unable to start \"buildlvl.bat\"" );
		return;
	}
	}


	{ // run Opus MAKE
	char szCommand[ _MAX_PATH * 2 ];

	strcpy( szCommand, "make.exe -nologo" );

	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo = { 0 };
	StartupInfo.cb = sizeof( StartupInfo );
	char szTitle[ 512 ];
	sprintf( szTitle, "Making \"%s\" [OpusMake]", szLevelDir );
	StartupInfo.lpTitle = szTitle;

	if ( CreateProcess(
			NULL,								// LPCTSTR lpApplicationName
			szCommand,							// LPTSTR lpCommandLine
			NULL,				// LPSECURITY_ATTRIBUTES lpProcessToolbar
			NULL,				// LPSECURITY_ATTRIBUTES lpThreadToolbar
			FALSE,				// BOOL bInheritHandles
			0,					// DWORD dwCreationFlags
			NULL,				// LPVOID lpEnvironment
			szSourceLevelDir,			// LPCTSTR lpCurrentDirectory
			&StartupInfo,		// LPSTARTUPINFO lpStartupInfo
			&ProcessInfo ) )	// LPPROCESS_INFORMATION lpProcessInformation
		WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
	else
	{
		Error( "Unable to start Opus MAKE" );
		return;
	}
	}

	HWND hwndRunAfterMake = GetDlgItem( _hPanel, IDC_RUN_AFTER_MAKE );
	assert( hwndRunAfterMake );
	if ( Button_GetCheck( hwndRunAfterMake ) )
		RunLevel();
}


void
Toolbar::RunLevel()
{
//	const char* szLevelDir = GetLevelDir();
//	if ( !szLevelDir )
//		return;

	STARTUPINFO StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo;

	StartupInfo.cb = sizeof( StartupInfo );

	char szDir[ _MAX_PATH ];
	char szCommand[ _MAX_PATH * 2 ];

	bool bPlaystation = Button_GetCheck( GetDlgItem( _hPanel, IDC_PSX ) );

	assert( *szWorldFoundryDir );
	strcpy( szDir, szWorldFoundryDir );
	strcat( szDir, "\\assets\\" );
	strcat( szDir, bPlaystation ? "psx" : "win" );
	strcat( szDir, "\\" );

	strcpy( szCommand, szDir );

	if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_DEBUG ) ) )
	{
		strcat( szCommand, "wfdh.exe " );

		if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_PARANOID ) ) )
			strcat( szCommand, "-paranoid " );
	}
	else
		strcat( szCommand, "wfrh.exe " );

	if ( bPlaystation )
	{
//		if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_SUBDIVISION ) ) )
//			strcat( szCommand, "-subdivide " );
	}
	else
	{	// Windows 95
		if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_WINDOW ) ) )
			strcat( szCommand, "-window " );

		if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_PERSPECTIVECORRECT ) ) )
			strcat( szCommand, "-perspective " );
	}

	if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_SINGLESIDED ) ) )
		strcat( szCommand, "-singlesided " );

	if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_SHOWBOXES ) ) )
		strcat( szCommand, "-showboxes " );

	if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_SHOWPOSITIONS ) ) )
		strcat( szCommand, "-showpositions " );

	if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_NOINITIALROTATION ) ) )
		strcat( szCommand, "-norotation " );

	// Framerate
	HWND hwndFrameRate = GetDlgItem( _hPanelParameters, IDC_FRAMERATE );
	assert( hwndFrameRate );

	char szFrameRate[ 100 ];
	ComboBox_GetText( hwndFrameRate, szFrameRate, sizeof( szFrameRate ) );
	if ( *szFrameRate )
	{
		strcat( szCommand, "-rate" );
		strcat( szCommand, szFrameRate );
		strcat( szCommand, " " );
	}

	assert( _hPanelParameters );
	HWND hwndOtherCommandLine = GetDlgItem( _hPanelParameters, IDC_OTHERPARAMS );
	assert( hwndOtherCommandLine );
	ICustEdit* edit = GetICustEdit( hwndOtherCommandLine );
	assert( edit );
	char szOtherCommandLine[ _MAX_PATH ];
	edit->GetText( szOtherCommandLine, sizeof( szOtherCommandLine ) );
	ReleaseICustEdit( edit );
//	Edit_GetText( hwndOtherCommandLine, szOtherCommandLine, sizeof( szOtherCommandLine ) );
	if ( *szOtherCommandLine )
	{
		strcat( szCommand, szOtherCommandLine );
		strcat( szCommand, " " );
	}

//	if ( Button_GetCheck( GetDlgItem( _hPanelParameters, IDC_SOUND ) )
//		strcat( szCommand, "-sound " );

	{ // Debugging streams
	for ( int i=0; i<NUM_ITEMS( idComboStreams ); ++i )
	{
		HWND hwndStreamType = GetDlgItem( _hPanelDebugStreams, idComboStreams[i].id );
		assert( hwndStreamType );

		char szStreamOutputName[ 100 ];
		ComboBox_GetText( hwndStreamType, szStreamOutputName, sizeof( szStreamOutputName ) );
		if ( *szStreamOutputName )
		{
			strcat( szCommand, idComboStreams[i].szSwitch );
			for ( int j=0; j<NUM_ITEMS( streamDestination ); ++j )
			{
				if ( _stricmp( szStreamOutputName, streamDestination[j].name ) == 0 )
					break;
			}
			assert( j <= NUM_ITEMS( streamDestination ) );
			if ( j == NUM_ITEMS( streamDestination ) )
			{	// Couldn't find a match -- must be a filename
				strcat( szCommand, "f" );
				strcat( szCommand, szStreamOutputName );
			}
			else
			{
				strcat( szCommand, streamDestination[j].szSwitch );
			}
			strcat( szCommand, " " );
		}
	}
	}


	{ // Which level to run?
#if 0
	assert( _hPanelParameters );
	HWND hwndLevelNumber = GetDlgItem( _hPanelParameters, IDC_LEVEL_NUMBER );
	assert( hwndLevelNumber );
	ICustEdit* edit = GetICustEdit( hwndLevelNumber );
	assert( edit );
	char szLevelNumber[ _MAX_PATH ];
	edit->GetText( szLevelNumber, sizeof( szLevelNumber ) );
	ReleaseICustEdit( edit );
	if ( *szLevelNumber )
	{
		strcat( szCommand, szLevelNumber );
		strcat( szCommand, " " );
	}
	else
#endif
		strcat( szCommand, "1 " );
	}


	if ( !bPlaystation )
	{
		HWND hwndHeight = GetDlgItem( _hPanelParameters, IDC_HEIGHT );
		assert( hwndHeight );

		char szHeight[ 100 ];
		ComboBox_GetText( hwndHeight, szHeight, sizeof( szHeight ) );
		if ( *szHeight )
		{
			strcat( szCommand, "-height=" );
			strcat( szCommand, szHeight );
			strcat( szCommand, " " );
		}
	}

	//sprintf( szProgramAndFile, "notepad.exe %s", "c:\\autoexec.bat" );
	if ( CreateProcess(
			NULL,				// LPCTSTR lpApplicationName
			szCommand,			// LPTSTR lpCommandLine
			NULL,				// LPSECURITY_ATTRIBUTES lpProcessToolbar
			NULL,				// LPSECURITY_ATTRIBUTES lpThreadToolbar
			FALSE,				// BOOL bInheritHandles
			0,					// DWORD dwCreationFlags
			NULL,				// LPVOID lpEnvironment
			szDir,				// LPCTSTR lpCurrentDirectory
			&StartupInfo,		// LPSTARTUPINFO lpStartupInfo
			&ProcessInfo ) )	// LPPROCESS_INFORMATION lpProcessInformation
		;	//WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
	else
	{
		char szErrorMessage[ 512 ];
		sprintf( szErrorMessage, "Unable to start application \"%s\"", szCommand );
		Error( szErrorMessage );
	}
}


void
Toolbar::ExportSelectedObject()
{
	HINSTANCE hMax2IffInst = LoadMaxLibrary( MAX2IFF_PlugIn );
	assert( hMax2IffInst );

	EXPORT_MESH_QUERY_PROC fnMax2Iff = (EXPORT_MESH_QUERY_PROC)GetProcAddress( hMax2IffInst, "max2iff_Query" );
	assert( fnMax2Iff );

//	Interface* ip = GetCOREInterface();
	assert( ip );

	for ( int idxObject=0; idxObject<ip->GetSelNodeCount(); ++idxObject )
	{
		INode* pNode = ip->GetSelNode( idxObject );
		assert( pNode );
		char szFilename[ _MAX_PATH ];
		fnMax2Iff( pNode, CreateObjectFilename( pNode ) );
	}

	FreeLibrary( hMax2IffInst );
}


void
Toolbar::LintLevel()
{
	const char* szLevelDir = GetLevelDir();
	if ( !szLevelDir )
		return;

#if 0
	// LevelCon
	HINSTANCE hLevelConInst = LoadMaxLibrary( MAX2LVL_PlugIn );
	assert( hLevelConInst );

	EXPORT_LEVEL_PROC fnLevelCon = (EXPORT_LEVEL_PROC)GetProcAddress( hLevelConInst, "levelcon" );
	assert( fnLevelCon );

	int success = fnLevelCon();

	FreeLibrary( hLevelConInst );
#endif
}


#endif

void
Toolbar::Command( HWND hWnd, int button )
{
	HWND hwndButton = GetDlgItem( hWnd, button );
	// CAUTION: hwndButton may be NULL -- assert before using

	switch ( button )
	{
		case IDC_MAKE_LEVEL:
			MakeLevel();
			break;

		case IDC_RUN_LEVEL:
			RunLevel();
			break;

		case IDC_CLEAN_LEVEL:
			CleanLevel();
			break;

		case IDC_EXPORT_OBJECT:
			ExportSelectedObject();
			break;

		case IDC_LINT_LEVEL:
			LintLevel();
			break;

		case IDC_WIN:
		case IDC_PSX:
		{
			assert( _hPanelParameters );
			SaveRadioButton( _hPanel, szPlaystation, IDC_PSX );
			bool bWin = Button_GetCheck( GetDlgItem( _hPanel, IDC_WIN ) );
			Button_Enable( GetDlgItem( _hPanelParameters, IDC_SUBDIVISION ), !bWin );
			Button_Enable( GetDlgItem( _hPanelParameters, IDC_PERSPECTIVECORRECT ), bWin );
			Button_Enable( GetDlgItem( _hPanelParameters, IDC_WINDOW ), bWin );
			Button_Enable( GetDlgItem( _hPanelParameters, IDC_FULLSCREEN ), bWin );
			Button_Enable( GetDlgItem( _hPanelParameters, IDC_HEIGHT ), bWin );
			break;
		}

		case IDC_RELEASE:
		case IDC_DEBUG:
			assert( hwndButton );
			SaveRadioButton( _hPanelParameters, szDebug, IDC_DEBUG );
			Button_Enable( GetDlgItem( hWnd, IDC_PARANOID ), button == IDC_DEBUG );
			break;

		case IDC_WINDOW:
		case IDC_FULLSCREEN:
			assert( hwndButton );
			SaveRadioButton( _hPanelParameters, szWindow, IDC_WINDOW );
			break;

		case IDC_PARANOID:
			assert( hwndButton );
			SaveCheckBox( _hPanelParameters, szParanoid, IDC_PARANOID );
			break;

		case IDC_SHOWBOXES:
			assert( hwndButton );
			SaveCheckBox( _hPanelParameters, szShowBoxes, IDC_SHOWBOXES );
			break;

		case IDC_SHOWPOSITIONS:
			assert( hwndButton );
			SaveCheckBox( _hPanelParameters, szShowPositions, IDC_SHOWPOSITIONS );
			break;

		case IDC_NOINITIALROTATION:
			assert( hwndButton );
			SaveCheckBox( _hPanelParameters, szNoInitialRotation, IDC_NOINITIALROTATION );
			break;

		case IDC_SINGLESIDED:
			assert( hwndButton );
			SaveCheckBox( _hPanelParameters, szSingleSided, IDC_SINGLESIDED );
			break;

//		case IDC_SUBDIVISION:
//			assert( hwndButton );
//			SaveCheckBox( _hPanelParameters, szSubdivide, IDC_SUBDIVISION );
//			break;

		case IDC_PERSPECTIVECORRECT:
			assert( hwndButton );
			SaveCheckBox( _hPanelParameters, szPerspectiveCorrect, IDC_PERSPECTIVECORRECT );
			break;

		case IDC_FRAMERATE:
			assert( hwndButton );
			SaveInteger( _hPanelParameters, szFrameRate, IDC_FRAMERATE );
			break;

		case IDC_HEIGHT:
			assert( hwndButton );
			SaveInteger( _hPanelParameters, szHeight, IDC_HEIGHT );
			break;

//		case IDC_OTHERPARAMS:
//			assert( hwndButton );
//			SaveString( _hPanelParameters, szCommandOtherParams, button );
//			break;

		case IDC_RUN_AFTER_MAKE:
			SaveCheckBox( _hPanel, szRunAfterMake, button );
			_setButtons_Level();
			break;

		case IDC_LEVEL_NUMBER:
			SaveInteger( _hPanelParameters, szLevelNumber, button );
			break;

		case IDC_WARNINGS:
		case IDC_ERRORS:
		case IDC_FATAL:
		case IDC_STATISTICS:
		case IDC_PROGRESS:
		case IDC_DEBUGGING:
		case IDC_ACTOR:
		case IDC_MOVEMENT:
		case IDC_COLLISION:
		case IDC_ROOM:
		case IDC_FLOW:
		case IDC_AI:
		case IDC_LEVEL:
		case IDC_LOGO:
		case IDC_TOOL:
		case IDC_CAMERA:
		case IDC_TEXTURE:
		case IDC_MEMORY:
		case IDC_SCRIPT:
		case IDC_ASSETID:
		case IDC_STREAMING:
		case IDC_MAILBOX:
		case IDC_FRAMEINFO:
		{
			assert( hwndButton );
			for ( int i=0; i<NUM_ITEMS( idComboStreams ); ++i )
			{
				if ( idComboStreams[i].id == button )
					break;
			}
			assert( i < NUM_ITEMS( idComboStreams ) );
			SaveString( _hPanelDebugStreams, idComboStreams[i].name, button );
			break;
		}
	}
}
