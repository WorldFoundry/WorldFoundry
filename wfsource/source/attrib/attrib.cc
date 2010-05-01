// attrib.cc

#include <attrib/attrib.hp>
#include <attrib/oad.hp>

#include <iffwrite/iffwrite.hp>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <attrib/buttons/int.hp>
#include <attrib/buttons/fixed.hp>
#include <attrib/buttons/propshet.hp>
#include <attrib/buttons/string.hp>
#include <attrib/buttons/group.hp>
#include <attrib/buttons/dropmenu.hp>
#include <attrib/buttons/checkbox.hp>
#include <attrib/buttons/filename.hp>
#if 0
#include "attrib/buttons/objref.hp"
#include "attrib/buttons/colorpck.hp"
#include "attrib/buttons/cycle.hp"
#include "attrib/buttons/meshname.hp"
#include "attrib/buttons/xdata.hp"
#include "attrib/buttons/classref.hp"
#include "attrib/buttons/mailbox.hp"
#include "attrib/buttons/combobox.hp"
#endif

// For property sheet header
extern HWND hwndTabControl;
extern HIMAGELIST hSmall;
extern HIMAGELIST hLarge;
// For group start/group stop [group.cc] code
char* szGroupName;

#pragma message( "TODO: OAD allow short and long labels [as different chunk names]" )

#if 0
void
Error( const char* buf, ... )
{
	static char buffer[ 1024 ];
	va_list arglist;

	va_start( arglist, buf );
	vsprintf( buffer, buf, arglist );
	va_end( arglist );

	MessageBox( NULL, buffer, "attrib", MB_OK );
}
#endif


Attributes::Attributes( HWND hwnd, Oad* theOad, bool bExpandedDisplay )
{
	assert( hwnd );
	_hPanel = _hDlg = hwnd;

	_hBrushStatic = CreateSolidBrush( GetSysColor( COLOR_WINDOWTEXT ) );
	assert( _hBrushStatic );

	_bExpandedDisplay = bExpandedDisplay;

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

	_font = Button_GetFont( _hDlg );
	assert( _font );

	assert( theOad );
	_oad = theOad;

	theAttributes = this;
	CreateGadgets();
}


BOOL CALLBACK
AttributesDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	try
	{
		switch ( msg )
		{
			case WM_INITDIALOG:
//				theAttributes.Init();
				break;

			case WM_DESTROY:
//				theAttributes.Destroy();
				break;

			case WM_SIZE:
			{
				if ( hwndTabControl )
				{	// Need to resize the tab control
#pragma message( "TODO: Duplicated code in WM_SIZE code and RollUp constructor" )
                	// Calculate the display rectangle, assuming the tab control is the size of the client area.
					RECT rc;
                	SetRect(&rc, 0, 0,  LOWORD(lParam), HIWORD(lParam) );
                	TabCtrl_AdjustRect(hwndTabControl, FALSE, &rc);
                	// Size the tab control to fit the client area.
                	HDWP hdwp = BeginDeferWindowPos( 1 );
                	DeferWindowPos( hdwp, hwndTabControl, NULL, 0, 0,  LOWORD( lParam ), HIWORD( lParam ), SWP_NOMOVE | SWP_NOZORDER );
					EndDeferWindowPos( hdwp );
				}

				break;
			}

			case WM_NOTIFY:
			{
				UINT idCtrl = UINT( wParam );
				NMHDR* pnmh = (NMHDR*)lParam;

				if ( pnmh->code == TCN_SELCHANGE )
				{
					if ( pnmh->hwndFrom == theAttributes->_hPanel )
						theAttributes->ChangeSheet();
				}
			}

			case WM_COMMAND:
			{
				switch ( LOWORD( wParam ) )
				{
					default:
					{
						HWND hwndCtl = (HWND)lParam;
						if ( uiDialog* theActiveGadget = (uiDialog*)GetWindowLong( hwndCtl, GWL_USERDATA ) )
						{
							theActiveGadget->activate( (HWND)lParam );
							assert( theAttributes );
							theAttributes->refreshGadgetEnabledStates();
						}
					}
				}
				break;
			}

			default:
				return FALSE;
		}
	}
	catch ( Exception theException )
	{
	}

	return TRUE;
}


Attributes::~Attributes()
{
	DestroyGadgets();

	assert( _hBrushStatic );
	DeleteObject( _hBrushStatic );

	assert( _fontWingdings );
	DeleteObject( _fontWingdings );

	delete _oad, _oad = NULL;
}


void
Attributes::ChangeSheet()
{
	int iPage = hwndTabControl ? TabCtrl_GetCurSel( hwndTabControl ) : -1;

	for ( int idxGadget=0; idxGadget<_numGadgets; ++idxGadget )
	{
		uiDialog* pGadget = _gadgets[ idxGadget ];
		assert( pGadget );
		pGadget->show_hide( _bExpandedDisplay || ( pGadget->_idxSheet == iPage ) );
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
		WS_CHILD | dwStyle,
		theAttributes->x + x,
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

HWND retPanel;

void
Attributes::CreateGadgets()		// const char* IszClassName )
{
	DestroyGadgets();

	y = 5;
	x = 0;
	szGroupName = NULL;
	_idxCurrentTab = -1;


	int _idxSelectedSheet = 0;	// by default
	int _nPriority = -1;

	assert( ::hInstance );

	assert( _hDlg );
	char szTitle[ 512 ];
	sprintf( szTitle, "%s - %s - UDM", "Untitled", _oad->className().c_str() );
	Button_SetText(  _hDlg, szTitle );

 	assert( _font );

	_numGadgets = 0;
	for ( int i=0; i<_MAX_GADGETS; ++i )
		_gadgets[ i ] = NULL;

	assert( _oad );

	typeDescriptor* td = _oad->startOfTypeDescriptors();

	int len = _oad->len();

	int column = _bExpandedDisplay ? -160 : 0;

	y = 0;
	int size = 0;
	for ( ; size<_oad->len(); size += sizeof( typeDescriptor ), ++td )
	{
		if ( td->showAs == SHOW_AS_HIDDEN )
			continue;

#define BUTTON( __type__, __class__ ) \
	case BUTTON_##__type__: \
		{ \
		uiDialog* field = new __class__( td ); \
		assert( field ); \
		field->make_dialog_gadgets( _hDlg ); \
		y += _Y_INC; \
		_gadgets[ _numGadgets ] = field; \
		++_numGadgets; \
		break; \
		}

		switch ( td->type )
		{
			case BUTTON_INT32:
			{
				uiDialog* field;
				if ( 0 )
					;
				else if (
				( td->showAs == SHOW_AS_DROPMENU )
				|| ( td->showAs == SHOW_AS_RADIOBUTTONS )
				)
					field = new DropMenu( td );
				else if ( td->showAs == SHOW_AS_CHECKBOX )
					field = new CheckBox( td );
#if 0
				else if ( td->showAs == SHOW_AS_COLOR )
					field = new ColorPicker( td );
				else if ( td->showAs == SHOW_AS_RADIOBUTTONS )
					field = new CycleButton( td );
				else if ( td->showAs == SHOW_AS_MAILBOX )
					field = new Mailbox( td );
				else if ( td->showAs == SHOW_AS_COMBOBOX )
					field = new ComboBox( td );
				/*
				else if ( td->showAs == SHOW_AS_FOURCC )
					field = new FourCC( td );
				*/
#endif
				else
					field = new Int32( td );
				assert( field );
				field->make_dialog_gadgets( _hDlg );
				y += _Y_INC;
				_gadgets[ _numGadgets++ ] = field;
				break;
			}

			BUTTON( FIXED32, Fixed32 )
//			BUTTON( OBJECT_REFERENCE, ObjectReference )
			BUTTON( OBJECT_REFERENCE, String )
			BUTTON( STRING, String )
//			BUTTON( CLASS_REFERENCE, ClassReference )
			BUTTON( CLASS_REFERENCE, String )

//			BUTTON( XDATA, XData )

#if 1
			BUTTON( FILENAME, Filename )
#else
			case BUTTON_FILENAME:
			{
				uiDialog* field;
				if ( strcmp( "Mesh Name", td->name ) == 0 )
					field = new MeshName( td );
				else
					field = new Filename( td );
				assert( field );
				field->make_dialog_gadgets( _hDlg );
				y += _Y_INC;
				_gadgets[ _numGadgets++ ] = field;
//				++_numGadgets;
				break;
			}
#endif

			BUTTON( GROUP_START, Group )
			case BUTTON_GROUP_STOP:
			{
				assert( szGroupName );
				Group* field = (Group*)findGadget( szGroupName, BUTTON_GROUP_START );
				field->endGroup();
				assert( field );
				y += (_Y_INC/2);
				break;
			}

			case BUTTON_PROPERTY_SHEET:
			{
				extern HWND retPanel;

				uiDialog* field = new RollUp( td );
				assert( field );
				field->make_dialog_gadgets( _hDlg );
				++_idxCurrentTab;

#pragma message( "TODO: when iff, make default an expression to be evaulated" )
				if ( td->def >= _nPriority )
				{
					_nPriority = td->def;
					_idxSelectedSheet = _idxCurrentTab;
				}

				_hPanel = retPanel;
				_gadgets[ _numGadgets ] = field;
				++_numGadgets;
				if ( _bExpandedDisplay )
					column += 160;

				x = column;
#pragma message( "TODO: get size of tab control and set y to that value [or why aren't the buttons being created in child space?]" )
				y = 30;
				break;
			}

			default:
			{
				printf( "Skipping button \"%s\"\n", td->name );
				OutputDebugString( td->name );
				OutputDebugString( "\n" );
				break;
			}
		}

	}
	assert( size == _oad->len() );

	refreshGadgetEnabledStates();
	TabCtrl_SetCurSel( hwndTabControl, _idxSelectedSheet );
	ChangeSheet();
}


void
Attributes::setData( const void* pData )
{
#if 0
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
#endif

	refreshGadgetEnabledStates();
	ChangeSheet();
}


void
Attributes::refreshGadgetEnabledStates()
{
	for ( int idxGadget=0; idxGadget<_numGadgets; ++idxGadget )
	{
		uiDialog* pGadget = _gadgets[ idxGadget ];
		assert( pGadget );
		pGadget->checkEnable();
	}
}


void
Attributes::DestroyGadgets()
{
	for ( int idxGadget=0; idxGadget<_numGadgets; ++idxGadget )
	{
		uiDialog* pGadget = _gadgets[ idxGadget ];
		assert( pGadget );
		delete pGadget;
	}
	_numGadgets = 0;

	if ( hwndTabControl )
		DestroyWindow( hwndTabControl ), hwndTabControl = 0;

	if ( hSmall )
		ImageList_Destroy( hSmall ), hSmall = 0;

	if ( hLarge )
		ImageList_Destroy( hLarge ), hLarge = 0;
}


void
Attributes::Init()
{	// Called on creation of each rollup page
}


void
Attributes::Destroy()
{
}


void
Attributes::Apply()
{
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

	ofstream out( "c:\\test.bin", ios::binary );
	assert( out.good() );
	out << str.rdbuf();
	}
#endif
}
