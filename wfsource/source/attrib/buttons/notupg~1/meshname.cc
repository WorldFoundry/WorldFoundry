// meshname.cpp

#include "buttons/button.h"
#include "buttons/meshname.h"
#include "../lib/levelnum.h"
#include "../lib/objname.h"
#include "../lib/loaddll.h"
#include "../lib/loaddll.cc"

void Error( const char* buf, ... );

MeshName::MeshName( typeDescriptor* td ) : Filename( td )
{
	_hwndCreateMesh = NULL;
}


MeshName::~MeshName()
{
	assert( _hwndCreateMesh );
	DestroyWindow( _hwndCreateMesh );
}


void
MeshName::createMesh()
{
	INode* pNode = theAttributes._theSelection;
	assert( pNode );

	const char* szObjectFilename = CreateObjectFilename( pNode );
	assert( szObjectFilename );
	assert( *szObjectFilename );

	HINSTANCE hMax2IffInst = LoadMaxLibrary( MAX2IFF_PlugIn );
	assert( hMax2IffInst );

	EXPORT_MESH_QUERY_PROC fnMax2Iff = (EXPORT_MESH_QUERY_PROC)GetProcAddress( hMax2IffInst, "max2iff_Query" );
	assert( fnMax2Iff );

	const char* szLevelDir = CreateLevelDirName();
	assert( szLevelDir );
	assert( *szLevelDir );

	if ( fnMax2Iff( pNode, szObjectFilename ) )
	{
		assert( strncmp( szObjectFilename, szLevelDir, strlen( szLevelDir ) ) == 0 );
		reset( (char*)szObjectFilename + strlen( szLevelDir ) );
	}

	FreeLibrary( hMax2IffInst );
}


bool
MeshName::enable( bool bEnabled )
{
	if ( Filename::enable( bEnabled ) )
	{
		assert( _hwndCreateMesh );
		Button_Enable( _hwndCreateMesh, uiDialog::enable() );
		return true;
	}
	else
		return false;
}


void
MeshName::activate( HWND hwndButton )
{
	uiDialog::activate( hwndButton );
	if ( hwndButton == hwndBrowseButton )
		Filename::activate( hwndButton );
	else if ( hwndButton == _hwndCreateMesh )
		createMesh();
}


int
MeshName::make_dialog_gadgets( HWND hPanel )
{
	int x;
	int width;

	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	hwndBrowseButton = theAttributes.CreateWindowEx(
		0,
		"BUTTON",
		"1",
		0,
		wLabel + 3,
		theAttributes.y,
		20,
		16,
		hPanel );
	assert( hwndBrowseButton );
	SetWindowFont( hwndBrowseButton, theAttributes._fontWingdings, TRUE );
	SetWindowLong( hwndBrowseButton, GWL_USERDATA, (LONG)this );

	_hwndCreateMesh = theAttributes.CreateWindowEx(
		0,
		"BUTTON",
		"=",
		0,
		wLabel + 3 + 20 + 3,
		theAttributes.y,
		20,
		16,
		hPanel );
	assert( _hwndCreateMesh );
	SetWindowFont( _hwndCreateMesh, theAttributes._fontWingdings, TRUE );
	SetWindowLong( _hwndCreateMesh, GWL_USERDATA, (LONG)this );

	theAttributes.y += 18;

	hwndEdit = theAttributes.CreateWindowEx(
		0,
		"CustEdit",
		_td->name,
		WS_TABSTOP,
		x = 5,
		theAttributes.y,
		width = 155,
		16,			// height
		hPanel );
	assert( hwndEdit );
	_edit = GetICustEdit( hwndEdit );
	assert( _edit );

	reset();

	return x + width;
}
