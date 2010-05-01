// attrib.cc

#include <global.hp>
#include "attrib.h"
#include "oad.hpp"

#include <io.h>
#include <windowsx.h>

#include "../lib/registry.h"

#include "buttons/int.h"
#include "buttons/fixed.h"
#include "buttons/objref.h"
#include "buttons/colorpck.h"
#include "buttons/dropmenu.h"
#include "buttons/propshet.h"
#include "buttons/group.h"
#include "buttons/cycle.h"
#include "buttons/filename.h"
#include "buttons/meshname.h"
#include "buttons/checkbox.h"
#include "buttons/xdata.h"
#include "buttons/classref.h"
#include "buttons/mailbox.h"
#include "buttons/combobox.h"
#include "buttons/string.h"

#include "util.h"

#include <stdarg.h>

//#include <strstream>
//#include <pigsys/pigsys.h>
#include <iffwrite.hp>


void
Error( const char* buf, ... )
{
	static char buffer[ 1024 ];
	va_list arglist;

	va_start( arglist, buf );
	vsprintf( buffer, buf, arglist );
	va_end( arglist );

	MessageBox(GetCOREInterface()->GetMAXHWnd(), buffer, "Attrib Error", MB_OK);
}


Attributes theAttributes;

class AttributesClassDesc : public ClassDesc
{
public:
	int 			IsPublic() {return true;}
	void*			Create(BOOL loading = FALSE) {return &theAttributes;}
	const TCHAR*	ClassName() {return "Attributes";}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Attrib_ClassID; }
	const TCHAR* 	Category() {return _T("World Foundry");}
};

static AttributesClassDesc appDataTestDesc;
ClassDesc* GetPropertiesDesc() {return &appDataTestDesc;}


#if 0
LRESULT CALLBACK
RollupWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	//assert( theSelect._headerWndProc );
	if ( msg == WM_RBUTTONDOWN )
	{
		int x = LOWORD( lParam );
		int y = HIWORD( lParam );

		RECT rectParentParent;
		GetWindowRect( GetParent( GetParent( hwnd ) ), &rectParentParent );
		RECT rect;
		GetWindowRect( hwnd, &rect );

		x += rect.left - rectParentParent.left;
		lParam = MAKELONG( x, y );
		//SendMessage( GetParent( GetParent( hwnd ) ), WM_COLUMN_RBUTTONDOWN, col, lParam );

		return 0;
	}
	else
		return theSelect._headerWndProc( hwnd, msg, wParam, lParam );
}
#endif


uiDialog* theActiveGadget;

BOOL CALLBACK
AttributesDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	try
	{
		switch ( msg )
		{
#if 0
			case WM_CTLCOLOREDIT:
			{
				HDC hdcEdit = (HDC)wParam;
				HWND hwndEdit = (HWND)lParam;

#if 0
				uiDialog* pGadget = (uiDialog*)GetWindowLong( hwndEdit, GWL_USERDATA );
				DWORD colorText;
				if ( pGadget )
					colorText = GetSysColor( COLOR_BTNHIGHLIGHT );
				else
					colorText = GetSysColor( COLOR_WINDOWTEXT );
				SetTextColor( hdcEdit, colorText );
#endif

				assert( theAttributes._hBrushStatic );
				return (LRESULT)theAttributes._hBrushStatic;
				break;
			}
#endif

			case WM_INITDIALOG:
				theAttributes.Init();
				break;

			case WM_DESTROY:
				theAttributes.Destroy();
				break;

			case WM_COMMAND:
			{
				switch ( LOWORD( wParam ) )
				{
#if 0
					case IDM_ENABLED:
					{
						assert( theActiveGadget );
						//theActiveGadget->_bUserOverrideEnable = FALSE;
						//theActiveGadget = NULL;	?
						theAttributes.refreshGadgetEnabledStates();
						break;
					}

					case IDM_COMBOBOX:
					case IDM_CYCLEBUTTON:
					case IDM_SLIDER:
					{
						break;
					}

//TEST CODE
					case IDC_OPEN:
					{
						HWND hwnd = GetDlgItem( hWnd, IDC_ROLLUP );
						assert( hwnd );
						char szRollup[ 10 ];
						Edit_GetText( hwnd, szRollup, sizeof( szRollup ) );
						int nRollup = atoi( szRollup );

						assert( theMainPanel );

						IRollupWindow* pRollup = GetIRollup( theMainPanel );
						assert( pRollup );

						pRollup->SetPanelOpen( nRollup, TRUE );

						ReleaseIRollup( pRollup );

						break;
					}

					case IDC_CLOSE:
					{
						HWND hwnd = GetDlgItem( hWnd, IDC_ROLLUP );
						assert( hwnd );
						char szRollup[ 10 ];
						Edit_GetText( hwnd, szRollup, sizeof( szRollup ) );
						int nRollup = atoi( szRollup );


						break;
					}
#endif

					case IDC_COPY:
						theAttributes.CopyAttributes();
						break;

					case IDC_PASTE:
						theAttributes.PasteAttributes();
						break;

					case IDC_PREVIOUS:
						theAttributes.OnPrevious();
						break;

					case IDC_NEXT:
						theAttributes.OnNext();
						break;

					case IDC_OBJECT_NAME:
					{
						char szObjectName[ 256 ];
						// Read field name
						assert( theAttributes._hwndObjectName );
						int err = ComboBox_GetText( theAttributes._hwndObjectName, szObjectName, sizeof( szObjectName ) );
						assert( err != CB_ERR );

						static int idxSelected = CB_ERR;
						UINT notification = HIWORD( wParam );
						if ( notification == CBN_SETFOCUS )
						{
							idxSelected = ComboBox_GetCurSel( theAttributes._hwndObjectName );
							DisableAccelerators();
						}
						else if ( ( notification == CBN_KILLFOCUS ) || ( notification == CBN_DROPDOWN ) )
						{
							theAttributes.FillInObjectList();

							err = ComboBox_SetCurSel( theAttributes._hwndObjectName, idxSelected );
							assert( err != CB_ERR );

							if ( notification == CBN_KILLFOCUS )
								EnableAccelerators();
						}
						else if ( notification == CBN_EDITUPDATE )
						{
							theAttributes._theSelection->SetName( szObjectName );
							theAttributes.UpdateCopyPasteButtons( szObjectName );
						}
						else if ( notification == CBN_SELCHANGE )
						{
							theAttributes.OnObjectName();
						}
#if 0
						else
						{
							char buffer[ 256 ];
							sprintf( buffer, "notification = %d\n", notification );
							OutputDebugString( buffer );
						}
#endif
						break;
					}

					default:
					{
						HWND hwndCtl = (HWND)lParam;
						if ( hwndCtl == theAttributes._hwndClassName )
						{
							UINT notification = HIWORD( wParam );
							if ( notification == CBN_SELCHANGE )
								theAttributes.OnClassName();
						}
						else
						{
							if ( theActiveGadget = (uiDialog*)GetWindowLong( hwndCtl, GWL_USERDATA ) )
								theActiveGadget->activate( (HWND)lParam );
							theAttributes.refreshGadgetEnabledStates();
						}
					}
				}
				break;
			}

#if MAX_RELEASE < 2000
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MOUSEMOVE:
				if ( theAttributes.ip )
					theAttributes.ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
				break;
#endif

			default:
				return FALSE;
		}
	}
	catch ( Exception /*theException*/ )
	{
	}

	return TRUE;
}


// Resets the Object list to the names of the objects
void
Attributes::FillInObjectList()
{
	assert( _hwndObjectName );

	ComboBox_ResetContent( _hwndObjectName );
	// List object names
	for ( int i=0; i<ip->GetSelNodeCount(); ++i )
	{
		ComboBox_AddString( _hwndObjectName, ip->GetSelNode( i )->GetName() );
	}
}


void
Attributes::OnClassName()
{
	assert( _hwndClassName );
	int idx = ComboBox_GetCurSel( _hwndClassName );
	if ( idx != CB_ERR )
	{
		char szClassName[ 512 ];
		ComboBox_GetLBText( _hwndClassName, idx, szClassName );

		assert( strchr( szClassName, '[' ) );
		strcpy( szClassName, strchr( szClassName, '[' ) + 1 );
		assert( strchr( szClassName, '.' ) );
		*( strchr( szClassName, '.' ) ) = '\0';

		if ( strcmp( _szClassName, szClassName ) != 0 )
		{
			INode* savedSelection = _theSelection;
			strcpy( _szClassName, szClassName );

			DestroyAttributes();
			_theSelection = savedSelection;
			CreateAttributes( szClassName );
		}
	}
}


// Move to the specified (index) entry in the Object list
// Does nothing if new index is current index [mostly to avoid visible flash]
void
Attributes::_Navigate( int index )
{
	assert( ip );
	assert( 0 <= index && index < ip->GetSelNodeCount() );
	INode* newSel = ip->GetSelNode( index );
	assert( newSel );

	assert( _theSelection );
	if ( newSel != _theSelection )
	{
		DestroyAttributes();
		_theSelection = newSel;
		assert( _theSelection );
		CreateAttributes();
		SetObjectSelected( index );
	}
}


// BUTTON: Go to the selected object
void
Attributes::OnObjectName()
{
	assert( _hwndObjectName );
	int index = ComboBox_GetCurSel( _hwndObjectName );
	_Navigate( index );
}


// BUTTON: Go to previous object
void
Attributes::OnPrevious()
{
	assert( _hwndObjectName );
	int index = ComboBox_GetCurSel( _hwndObjectName );
	_Navigate( index - 1 );
}


// BUTTON: Go to next object
void
Attributes::OnNext()
{
	assert( _hwndObjectName );
	int index = ComboBox_GetCurSel( _hwndObjectName );
	_Navigate( index + 1 );
}


// Updates the "Copy" button to say "Copy _objectName_"
void
Attributes::UpdateCopyPasteButtons( const char* szObjectName )
{
	FILE* fpAttrib = fopen( Attrib_OAD_Clipboard, "rb" );

	int cbClass;
	char* pClass;
	pClass = (char*)LoadBinaryFile( Attrib_ClassName_Clipboard, cbClass );

	assert( _hwndPaste );
	assert( ip );
	if ( pClass && fpAttrib && ( ip->GetSelNodeCount() != 0 ) )
	{
		char szBuffer[ 256 ];
		sprintf( szBuffer, "Paste class\n%s", pClass );
		if ( ip->GetSelNodeCount() > 1 )
			strcat( szBuffer, "..." );
		Button_SetText(	_hwndPaste, szBuffer );
		Button_Enable( _hwndPaste, true );
	}
	else
	{
		Button_SetText( _hwndPaste, "Paste" );
		Button_Enable( _hwndPaste, false );
	}

	if ( pClass )
		free( pClass );

	if ( fpAttrib )
		fclose( fpAttrib );

	assert( _hwndCopy );
	if ( ip->GetSelNodeCount() == 0 )
	{
		Button_SetText( _hwndCopy, "Copy" );
		Button_Enable( _hwndCopy, false );
	}
	else
	{
		assert( szObjectName );

		char szBuffer[ 128 ];
		sprintf( szBuffer, "Copy object\n%s", szObjectName );

		Button_SetText( _hwndCopy, szBuffer );
		Button_Enable( _hwndCopy, true );
	}
}


void
Attributes::SetObjectSelected( int index )
{
	assert( index != CB_ERR );
	ComboBox_SetCurSel( _hwndObjectName, index );
	ComboBox_Enable( _hwndObjectName, true );

	assert( _hwndPrevious );
	Button_Enable( _hwndPrevious, index != 0 );

	assert( _hwndNext );
	assert( ip );
	Button_Enable( _hwndNext, index != ip->GetSelNodeCount() - 1 );

	UpdateCopyPasteButtons( _theSelection->GetName() );
}


void
Attributes::SetClassButton()
{
	assert( _szClassName && *_szClassName );
	assert( _hwndClassName );

	for ( int idxClass=0; idxClass<ComboBox_GetCount( _hwndClassName ); ++idxClass )
	{
		char* szClassName = (char*)alloca(
			ComboBox_GetLBTextLen( _hwndClassName, idxClass ) + sizeof( '\0' ) );
		assert( szClassName );
		ComboBox_GetLBText( _hwndClassName, idxClass, szClassName );
		char* pLeftParen = strchr( szClassName, '[' );
		if ( pLeftParen )
		{
			if ( strncmp( pLeftParen+1, _szClassName, strlen( _szClassName ) ) == 0 )
				break;
		}
	}
	assert( idxClass != ComboBox_GetCount( _hwndClassName ) );
	ComboBox_SetCurSel( _hwndClassName, idxClass );
	ComboBox_Enable( _hwndClassName, true );
}


void
Attributes::CopyAttributes()
{
	AppDataChunk* adClass = _theSelection->GetAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_CLASS );
	if ( adClass )
	{
		FILE* fp = fopen( Attrib_ClassName_Clipboard, "wb" );
		assert( fp );
		fwrite( adClass->data, sizeof( char ), adClass->length, fp );
		fclose( fp );
	}
	else
	{
		_unlink( Attrib_ClassName_Clipboard );
		_unlink( Attrib_OAD_Clipboard );
		UpdateCopyPasteButtons( _theSelection->GetName() );
		return;
	}

	AppDataChunk* adOad = _theSelection->GetAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_OAD );
	if ( adOad )
	{
		FILE* fp = fopen( Attrib_OAD_Clipboard, "wb" );
		assert( fp );
		fwrite( adOad->data, sizeof( char ), adOad->length, fp );
		fclose( fp );
	}
	else
	{
		_unlink( Attrib_ClassName_Clipboard );
		_unlink( Attrib_OAD_Clipboard );
		UpdateCopyPasteButtons( _theSelection->GetName() );
		return;
	}

	UpdateCopyPasteButtons( _theSelection->GetName() );
}


UINT buttonPressed;

BOOL CALLBACK
MultiPasteDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( msg == WM_INITDIALOG )
	{
		HICON _hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_WF ) );
		assert( _hIcon );
		SetClassLong( hDlg, GCL_HICON, (LONG)_hIcon );

		HWND _hwndObjectName = theAttributes._hwndObjectName;
		assert( _hwndObjectName );
		int idx = ComboBox_GetCurSel( _hwndObjectName );
		assert( idx != CB_ERR );
		char szObjectName[ 512 ];
		ComboBox_GetLBText( _hwndObjectName, idx, szObjectName );

		char szButtonText[ 256 ];

		HWND hwnd = GetDlgItem( hDlg, IDC_OBJECT );
		assert( hwnd );
		sprintf( szButtonText, "&%s", szObjectName );
		Button_SetText( hwnd, szButtonText );

		hwnd = GetDlgItem( hDlg, IDC_ALLSELECTED );
		int nObjects = ComboBox_GetCount( _hwndObjectName );
		sprintf( szButtonText, "&All %d objects selected", nObjects );
		Button_SetText( hwnd, szButtonText );
	}
	else if ( msg == WM_COMMAND )
	{
		int button = LOWORD( wParam );
		switch ( button )
		{
			case IDC_OBJECT:
			case IDC_ALLSELECTED:
			{
				buttonPressed = button;
				EndDialog( hDlg, TRUE );
				return TRUE;
			}

			case IDCANCEL:
			{
				EndDialog( hDlg, FALSE );
				return FALSE;
			}
		}
	}
	return FALSE;
}


void
Attributes::PasteAttributes()
{
	assert( _hwndObjectName );
	int nObjects = ComboBox_GetCount( _hwndObjectName );

	assert( nObjects > 0 );

	int idxStart, idxEnd;

	if ( nObjects == 1 )
	{
		idxStart = ComboBox_GetCurSel( _hwndObjectName );
		assert( idxStart != CB_ERR );
		idxEnd = idxStart + 1;
	}
	else
	{
		assert( hInstance );
		_hwndMax = GetCOREInterface()->GetMAXHWnd();
		assert( _hwndMax );
		if ( !DialogBox( hInstance, MAKEINTRESOURCE( IDD_MULTI_PASTE ), _hwndMax, MultiPasteDlgProc ) )
			return;

		if ( buttonPressed == IDC_OBJECT )
		{
			idxStart = ComboBox_GetCurSel( _hwndObjectName );
			assert( idxStart != CB_ERR );
			idxEnd = idxStart + 1;
		}
		else if ( buttonPressed == IDC_ALLSELECTED )
		{
			idxStart = 0;
			idxEnd = nObjects;
		}
	}

	int idxSelected = ComboBox_GetCurSel( _hwndObjectName );
	assert( idxSelected != CB_ERR );

	int cbClass;
	char* pClass;
	pClass = (char*)LoadBinaryFile( Attrib_ClassName_Clipboard, cbClass );
	if ( !pClass )
		return;

	int cbAttrib;
	char* pAttrib;
	pAttrib = (char*)LoadBinaryFile( Attrib_OAD_Clipboard, cbAttrib );
	if ( !pAttrib )
	{
		free( pClass );
		return;
	}

	assert( pClass );
	assert( pAttrib );
	for ( int i=idxStart; i<idxEnd; ++i )
	{
		assert( ip );
		int nSelectedObjects = ip->GetSelNodeCount();
		assert( 0 <= i && i < nSelectedObjects );

		char szObjectName[ 512 ];
		ComboBox_GetLBText( _hwndObjectName, i, szObjectName );
		assert( *szObjectName );

		//Animatable* newSel = _theSelection;
		INode* newSel = ip->GetSelNode( i );
		assert( newSel );

		newSel->RemoveAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_CLASS );
		newSel->AddAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_CLASS, cbClass, pClass );

		newSel->RemoveAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_OAD );
		newSel->AddAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_OAD, cbAttrib, pAttrib );
	}

	assert( _theSelection );
	INode* savedSelection = theAttributes._theSelection;
	theAttributes.DestroyGadgets();
	theAttributes._theSelection = savedSelection;
	theAttributes.CreateAttributes();
	UpdateCopyPasteButtons( _theSelection->GetName() );
}


Attributes::~Attributes()
{
	assert( _hBrushStatic );
	DeleteObject( _hBrushStatic );

	assert( _fontWingdings );
	DeleteObject( _fontWingdings );
}


Attributes::Attributes()
{
	_hBrushStatic = CreateSolidBrush( GetSysColor( COLOR_WINDOWTEXT ) );
	assert( _hBrushStatic );

	iu = NULL;
	ip = NULL;
	hPanel = NULL;
	_numGadgets = 0;
	for ( int i=0; i<_MAX_GADGETS; ++i )
		_gadgets[ i ] = NULL;
	_oad = NULL;

	_theSelection = NULL;

	{
	LOGFONT logFontWingdings =
	{
		-14,						// LONG lfHeight;
		0,							// LONG lfWidth;
		0,							// LONG lfEscapement;
		0,							// LONG lfOrientation;
		FW_NORMAL,					// LONG lfWeight;
		FALSE,						// BYTE lfItalic;
		FALSE,						// BYTE lfUnderline;
		FALSE,						// BYTE lfStrikeOut;
		SYMBOL_CHARSET,				// BYTE lfCharSet;
		OUT_TT_ONLY_PRECIS,			// BYTE lfOutPrecision;
		CLIP_DEFAULT_PRECIS,		// BYTE lfClipPrecision;
		DEFAULT_QUALITY,			// BYTE lfQuality;
		DEFAULT_PITCH | FF_DECORATIVE,// BYTE lfPitchAndFamily;
		"Wingdings"					// TCHAR lfFaceName[LF_FACESIZE];
	};
	_fontWingdings = CreateFontIndirect( &logFontWingdings );
	assert( _fontWingdings );
	}

	x = y = 0;
	szGroupName = NULL;

	// Create list of class names
	_hPanelClass = NULL;
	_numClasses = 0;
	for ( i=0; i<50; ++i )
		_szClasses[ i ] = NULL;

	const char szRegWorldFoundry[] = "Software\\World Foundry\\GDK";
	int success;
	success = GetLocalMachineStringRegistryEntry( szRegWorldFoundry, "WORLD_FOUNDRY_DIR",
		szWorldFoundryDir, sizeof( szWorldFoundryDir ) );
	if ( !success )
		Error( "WORLD_FOUNDRY_DIR not set in registry" );

	success = GetLocalMachineStringRegistryEntry( szRegWorldFoundry, "OAD_DIR",
		szOadDir, sizeof( szOadDir ) );
	if ( !success )
		Error( "OAD_DIR not set in registry" );

	{ // Get classes available in the OAD directory (.oad files)
	char szOadFileSpec[ _MAX_PATH ];
	sprintf( szOadFileSpec, "%s/*.oad", szOadDir );

	_finddata_t findFile;
	long dirHandle = _findfirst( szOadFileSpec, &findFile );
	if ( dirHandle != -1 )
	{
		char szFilename[ _MAX_PATH ];
		sprintf( szFilename, "%s/%s", szOadDir, findFile.name );
		FILE* fp = fopen( szFilename, "rb" );
		assert( fp );
		oadHeader header;
		fread( &header, 1, sizeof( header ), fp );
		fclose( fp );
		_szClasses[ _numClasses ] = (char*)malloc(
			strlen( header.name ) + sizeof( " [" ) + strlen( findFile.name ) + sizeof( "]" ) + sizeof( '\0' ) );
		assert( _szClasses[ _numClasses ] );
		strcpy( _szClasses[ _numClasses ], header.name );
		strcat( _szClasses[ _numClasses ], " [" );
		strcat( _szClasses[ _numClasses ], _strlwr( findFile.name ) );
		strcat( _szClasses[ _numClasses ], "]" );
		++_numClasses;

		while ( _findnext( dirHandle, &findFile ) == 0 )
		{
			char szFilename[ _MAX_PATH ];
			sprintf( szFilename, "%s/%s", szOadDir, findFile.name );
			FILE* fp = fopen( szFilename, "rb" );
			assert( fp );
			oadHeader header;
			fread( &header, 1, sizeof( header ), fp );
			fclose( fp );
			_szClasses[ _numClasses ] = (char*)malloc(
				strlen( header.name ) + 2 + strlen( findFile.name ) + 1 + 1 );
			assert( _szClasses[ _numClasses ] );
			strcpy( _szClasses[ _numClasses ], header.name );
			strcat( _szClasses[ _numClasses ], " [" );
			strcat( _szClasses[ _numClasses ], _strlwr( findFile.name ) );
			strcat( _szClasses[ _numClasses ], "]" );
			++_numClasses;
		}
	}
	}
}


HWND
Attributes::CreateWindowEx( DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName,
	DWORD dwStyle, int x, int y, int nWidth, int nHeight,
	HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam )
{
	return ::CreateWindowEx( dwExStyle,
		lpClassName,
		lpWindowName,
		WS_CHILD | WS_VISIBLE | dwStyle,
		theAttributes.x + x,
		y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam );
}


uiDialog*
Attributes::findGadget( const char* szName, buttonType ) const
{
	for ( int i=0; i<_numGadgets; ++i )
	{
		assert( _gadgets[ i ] );
		if ( ( strcmp( _gadgets[ i ]->fieldName(), szName ) == 0 ) )
			return _gadgets[ i ];
	}
	return NULL;
}


uiDialog*
Attributes::findGadget( const char* szName ) const
{
	for ( int i=0; i<_numGadgets; ++i )
	{
		assert( _gadgets[ i ] );
		if ( ( strcmp( _gadgets[ i ]->variableName(), szName ) == 0 ) )
			return _gadgets[ i ];
	}
	return NULL;
}


typeDescriptor* pEndOfTypeDescriptors;


void
Attributes::FillInClassReference( HWND hwndCombo, const char* szClassName )
{
	assert( hwndCombo );
	assert( _font );
	SetWindowFont( hwndCombo, _font, TRUE );
	assert( _numClasses > 0 );
	assert( ComboBox_GetCount( hwndCombo ) == 0 );
	for ( int i=0; i<_numClasses; ++i )
	{
		int err = ComboBox_AddString( hwndCombo, _szClasses[i] );
		assert( err != CB_ERR );
	}

	if ( szClassName )
	{
	}
}


void
Attributes::CreateAttributes( const char* IszClassName )
{
	assert( !hPanel );
	assert( ip );
	assert( hInstance );

	assert( _theSelection );

	hPanel = NULL;

	if ( !IszClassName )
	{	// Read AppData for class name (default to DEFAULT_CLASS if no AppData)
		AppDataChunk* adClass = _theSelection->GetAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_CLASS );
		strcpy( _szClassName, adClass ? (char*)adClass->data : DEFAULT_CLASS );
	}
	else
		strcpy( _szClassName, IszClassName );

	{ // Create the class rollup
	// !!! I want this to go away, but I don't know how to do it yet...
	if ( !_hPanelClass )
	{
		_hPanelClass = theAttributes.ip->AddRollupPage(
			hInstance,
			MAKEINTRESOURCE(IDD_CLASSNAME),
			AttributesDlgProc,
			"Class",
			0 );
		assert( _hPanelClass );

		_hwndObjectName = GetDlgItem( _hPanelClass, IDC_OBJECT_NAME );
		assert( _hwndObjectName );

		_hwndPrevious = GetDlgItem( _hPanelClass, IDC_PREVIOUS );
		assert( _hwndPrevious );

		_hwndNext = GetDlgItem( _hPanelClass, IDC_NEXT );
		assert( _hwndNext );

		assert( _font );

		_hwndCopy = GetDlgItem( _hPanelClass, IDC_COPY );
		assert( _hwndCopy );
		SetWindowFont( _hwndCopy, _font, TRUE );

		_hwndPaste = GetDlgItem( _hPanelClass, IDC_PASTE );
		assert( _hwndPaste );
		SetWindowFont( _hwndPaste, _font, TRUE );

		_hwndClassName = GetDlgItem( _hPanelClass, IDC_CLASS_NAME );
		assert( _hwndClassName );
		FillInClassReference( _hwndClassName, _szClassName );
	}	// !_hPanelClass
	}	// class rollup

	assert( _hwndCopy );
	Button_Enable( _hwndCopy, true );

	assert( _hwndPaste );
	Button_Enable( _hwndPaste, true );

	_numGadgets = 0;
	for ( int i=0; i<_MAX_GADGETS; ++i )
		_gadgets[ i ] = NULL;

	try
	{
		_oad = new Oad( _szClassName );
	}
	catch ( Exception /*theException*/ )
	{
		// TODO:
		OutputDebugString( "exception" );
		return;
	}

	assert( _oad );

	typeDescriptor* td = _oad->startOfTypeDescriptors();

	int len = _oad->len();

	pEndOfTypeDescriptors = (typeDescriptor*)( ((char*)td) + len );
	y = 0;
	int size = 0;
	for ( ; size<_oad->len(); size += sizeof( typeDescriptor ), ++td )
	{
		if ( td->showAs == SHOW_AS_HIDDEN )
			continue;

		//assert( *td->szEnableExpression == '\0' || *td->szEnableExpression == '1' );

#define BUTTON( __type__, __class__ ) \
	case BUTTON_##__type__: \
		{ \
		uiDialog* field = new __class__( td ); \
		assert( field ); \
		field->make_dialog_gadgets( hPanel ); \
		y += 18; \
		_gadgets[ _numGadgets ] = field; \
		++_numGadgets; \
		break; \
		}

		switch ( td->type )
		{
			case BUTTON_INT32:
			{
				uiDialog* field;
				if ( td->showAs == SHOW_AS_COLOR )
					field = new ColorPicker( td );
				else if ( td->showAs == SHOW_AS_DROPMENU )
					field = new DropMenu( td );
				else if ( td->showAs == SHOW_AS_RADIOBUTTONS )
					field = new CycleButton( td );
				else if ( td->showAs == SHOW_AS_CHECKBOX )
					field = new CheckBox( td );
				else if ( td->showAs == SHOW_AS_MAILBOX )
					field = new Mailbox( td );
				else if ( td->showAs == SHOW_AS_COMBOBOX )
					field = new ComboBox( td );
				/*
				else if ( td->showAs == SHOW_AS_FOURCC )
					field = new FourCC( td );
				*/
				else
					field = new Int32( td );
				assert( field );
				field->make_dialog_gadgets( hPanel );
				y += 18;
				_gadgets[ _numGadgets++ ] = field;
//				++_numGadgets;
				break;
			}

			BUTTON( FIXED32, Fixed32 )
			BUTTON( OBJECT_REFERENCE, ObjectReference )
			BUTTON( STRING, String )

			case BUTTON_FILENAME:
			{
				uiDialog* field;
				if ( strcmp( "Mesh Name", td->name ) == 0 )
					field = new MeshName( td );
				else
					field = new Filename( td );
				assert( field );
				field->make_dialog_gadgets( hPanel );
				y += 18;
				_gadgets[ _numGadgets++ ] = field;
//				++_numGadgets;
				break;
			}

			BUTTON( GROUP_START, Group )
			BUTTON( XDATA, XData )
			BUTTON( CLASS_REFERENCE, ClassReference )

			case BUTTON_GROUP_STOP:
			{
				assert( szGroupName );
				Group* field = (Group*)findGadget( szGroupName, BUTTON_GROUP_START );
				field->endGroup();
				assert( field );
				y += 8;
				break;
			}

			case BUTTON_PROPERTY_SHEET:
			{
				extern HWND retPanel;

				uiDialog* field = new RollUp( td );
				assert( field );
				field->make_dialog_gadgets( NULL );
				hPanel = retPanel;
				_gadgets[ _numGadgets ] = field;
				++_numGadgets;
				break;
			}

			default:
				OutputDebugString( td->name );
				OutputDebugString( "\n" );
				break;
		}

	}
	assert( size == _oad->len() );

#if defined( DO_OAD_IFF )
#else
	AppDataChunk* adOad = _theSelection->GetAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_OAD );
	if ( adOad )
	{	// Apply fields stored in AppData overriding the defaults contained in the .oad file
		byte* xdataStart = (byte*)adOad->data;
		byte* xdata = xdataStart;

		while ( xdata < (byte*)adOad->data + adOad->length )
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

			uiDialog* gadget = findGadget( szFieldName, bt );
			if ( gadget )
				gadget->reset( xdata );
			else
				xdata = pFieldStart + size;
		}
	}
#endif

	refreshGadgetEnabledStates();
	SetClassButton();
}


void
Attributes::refreshGadgetEnabledStates()
{
	for ( int i=0; i<_numGadgets; ++i )
	{
		assert( _gadgets[ i ] );
		_gadgets[ i ]->checkEnable();
	}
}


void
Attributes::DestroyClass()
{
	if ( _hPanelClass )
	{
		ip->DeleteRollupPage( _hPanelClass );
		_hPanelClass = 0;
	}
}


void
Attributes::DestroyGadgets()
{
	if ( hPanel )
	{
		assert( ip );
		for ( int i=0; i<_numGadgets; ++i )
		{
			assert( _gadgets[ i ] );
			delete _gadgets[ i ];
		}
		_numGadgets = 0;
		hPanel = NULL;
	}

	if ( _oad )
		delete _oad, _oad = NULL;
}


void
Attributes::DestroyAttributes( bool bApply )
{
	if ( hPanel )
		Apply();

	DestroyGadgets();
}


void
Attributes::BeginEditParams( Interface* ip, IUtil* iu )
{
	try
	{
		this->iu = iu;
		this->ip = ip;

		assert( ip );
		_font = ip->GetIObjParam()->GetAppHFont();
		assert( _font );

		if ( ip->GetSelNodeCount() != 0 )
		{
			_theSelection = ip->GetSelNode( 0 );
			assert( _theSelection );
			CreateAttributes();

			FillInObjectList();

			SetObjectSelected( 0 );
		}
	}
	catch ( Exception /*theException*/ )
	{
	}
}


void
Attributes::EndEditParams( Interface* ip, IUtil* iu )
{
	try
	{
		DestroyAttributes();
		DestroyClass();
	}
	catch ( Exception /*theException*/ )
	{
	}
	this->iu = NULL;
	this->ip = NULL;
}


void
Attributes::SelectionSetChanged( Interface* ip, IUtil* iu )
{
	try
	{
		if ( ip->GetSelNodeCount() > 0 )
		{
			if ( _theSelection != ip->GetSelNode( 0 ) )
				DestroyAttributes();
		}

		if ( _hwndObjectName )
		{
			while ( ComboBox_GetCount( _hwndObjectName ) )
				ComboBox_DeleteString( _hwndObjectName, 0 );
		}

		if ( ip->GetSelNodeCount() == 0 )
		{
			if ( _hwndClassName )
				ComboBox_Enable( _hwndClassName, false );
			if ( _hwndPrevious )
				Button_Enable( _hwndPrevious, false );
			if ( _hwndObjectName )
				ComboBox_Enable( _hwndObjectName, false );
			if ( _hwndNext )
				Button_Enable( _hwndNext, false );
			if ( _hwndCopy )
				Button_Enable( _hwndCopy, false );
			if ( _hwndPaste )
				Button_Enable( _hwndPaste, false );

			DestroyAttributes();

			_theSelection = NULL;
		}
		else
		{
			if ( !_theSelection || ( _theSelection != ip->GetSelNode( 0 ) ) )
			{
				_theSelection = ip->GetSelNode( 0 );
				assert( _theSelection );

				CreateAttributes();
			}
			FillInObjectList();
			SetObjectSelected( 0 );
		}
	}
	catch ( Exception /*theException*/ )
	{
	}
}


void
Attributes::Init()
{	// Called on creation of each rollup page

}


void
Attributes::Destroy()
{
}


extern int hdump( HINSTANCE, HWND, void* data, int size );

void
Attributes::Apply()
{
	assert( _theSelection );

#if 0
	{
	ostrstream str;
	IffWriterBinary _iff( str );
	_iff.enterChunk( ID( "OAD" ) );

		for ( int i=0; i<_numGadgets; ++i )
		{
			assert( _gadgets[ i ] );
			if ( _gadgets[ i ]->canSave() )
			{
//				_iff << _gadgets[ i ]->fieldName();
				_iff << *( _gadgets[ i ] );
			}
		}
	_iff.exitChunk();

	ofstream out( "d:\\test.bin", ios::binary );
	assert( out.good() );
	out << str.rdbuf();
	}
#endif

	int size = 0;
	for ( int i=0; i<_numGadgets; ++i )
	{
		assert( _gadgets[ i ] );
		if ( _gadgets[ i ]->canSave() )
			size += _gadgets[ i ]->storedDataSize();
	}

	byte* xdata = (byte*)malloc( size );
	assert( xdata );
	byte* xdataStart = xdata;

	for ( i=0; i<_numGadgets; ++i )
	{
		//MessageBox( hPanel, _gadgets[i]->fieldName(), _gadgets[i]->fieldName(), MB_OK );
		assert( _gadgets[ i ] );
		if ( _gadgets[ i ]->canSave() )
		{
			byte* pXdataBefore = xdata;
			_gadgets[ i ]->copy_to_xdata( xdata );
			//assert( pXdataBefore != xdata );
			assert( xdata == pXdataBefore + _gadgets[i]->storedDataSize() );
		}
	}

	assert( xdata == xdataStart + size );





	assert( _hwndClassName );
	int idx = ComboBox_GetCurSel( _hwndClassName );
	assert( idx != CB_ERR );
	int len = ComboBox_GetLBTextLen( _hwndClassName, idx );
	assert( len != CB_ERR );
	char* szClassName = (char*)malloc( len + 1 );
	assert( szClassName );
	ComboBox_GetLBText( _hwndClassName, idx, szClassName );
	const char* pLeftParen = strchr( szClassName, '[' );
	if ( pLeftParen )
	{
		strcpy( szClassName, pLeftParen + 1 );
		assert( strchr( szClassName, '.' ) );
		*( strchr( szClassName, '.' ) ) = '\0';
	}

	_theSelection->RemoveAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_CLASS );
	_theSelection->AddAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_CLASS, strlen(szClassName)+1, szClassName );

	if ( strcmp( szClassName, "disabled" ) != 0 )
	{
		_theSelection->RemoveAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_OAD );
		_theSelection->AddAppDataChunk( appDataTestDesc.ClassID(), UTILITY_CLASS_ID, SLOT_OAD, size, xdataStart );

#if 0
		{
			//hdump( hInstance, NULL, xdataStart, size );
			FILE* fp = fopen( "c:\\xdata.bin", "wb" );
			if ( fp )
			{
				fwrite( xdataStart, sizeof( byte ), size, fp );
				fclose( fp );
			}
		}
#endif
	}
}
