// xdata.cpp

#include <global.hp>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <ctype.h>

#include "../util.h"
#include "source/oas/oad.h"

#include "../oaddlg.h"
#include "xdata.h"
#include "../../lib/registry.h"

///////////////////////////////////////////////////////////////////////////////

int
XData::storedDataSize() const
{
	return _xdata ? uiDialog::storedDataSize() + strlen( _xdata ) + 1 : 0;
}


int
parseStringIntoParts( const char* _string, char* _xDataParameters[], int nMaxParameters )
{
	enum { SEPERATOR = '|' };
	int _nItems = 0;

	//debug( (char*)_string );

	char* strBegin = (char*)_string;
	char* strSeperator;

	do
	{
		int len;

		strSeperator = strchr( strBegin, SEPERATOR );

		if ( strSeperator )
			len = strSeperator - strBegin;
		else
		{	// Take to end of string
			len = strlen( strBegin );
		}

		_xDataParameters[ _nItems ] = (char*)malloc( len + 1 );
		assert( _xDataParameters[ _nItems ] );
		strncpy( _xDataParameters[ _nItems ], strBegin, len );
		*( _xDataParameters[ _nItems ] + len ) = '\0';
		//debug( "_xDataParameters[%d]: [%s]", _nItems, _xDataParameters[_nItems] );

		++_nItems;

		strBegin = strSeperator + 1;
	}
	while ( strSeperator );

	assert( _nItems > 0 );

	return _nItems;
}

///////////////////////////////////////////////////////////////////////////////

XData::XData( typeDescriptor* td ) : uiDialog( td )
{
	_bSave = true;
	_xdata = NULL;
}


XData::~XData()
{
	assert( hwndButton );
	DestroyWindow( hwndButton );

	if ( _xdata )
		free( _xdata ), _xdata = NULL;
}

///////////////////////////////////////////////////////////////////////////////

dataValidation
XData::copy_to_xdata( byte* & saveData )
{
	int x = storedDataSize();
	if ( _xdata )
	{
		uiDialog::copy_to_xdata( saveData );
		strncpy( (char*)saveData, _xdata, strlen( _xdata ) + 1 );
		saveData += strlen( _xdata ) + 1;
	}

	return DATA_OK;
}

///////////////////////////////////////////////////////////////////////////////

void
XData::reset()
{
	assert( _td );
	assert( hwndButton );
}


void
XData::reset( byte* & saveData )
{
	_bUserOverrideEnable = true;
	enable( uiDialog::enable() );

	reset( (char*)saveData );
	saveData += strlen( (char*)saveData ) + 1;
}


///////////////////////////////////////////////////////////////////////////////

void
XData::reset( char* data )
{
	if ( _xdata )
		free( _xdata );
	_xdata = (char*)malloc( strlen( data ) + 1 );
	assert( _xdata );
	strcpy( _xdata, data );
	reset();
}

///////////////////////////////////////////////////////////////////////////////

void
XData::activate( HWND hwnd )
{
	uiDialog::activate( hwnd );

	if ( hwnd == hwndButton )
	{
		STARTUPINFO StartupInfo = { 0 };
		PROCESS_INFORMATION ProcessInfo;

		char* szFilename = tmpnam( NULL );
		assert( szFilename );

		FILE* fp = fopen( szFilename, "wt" );
		assert( fp );
		if ( _xdata )
			fwrite( _xdata, sizeof( char ), strlen( _xdata ), fp );
		fclose( fp );

		char szProgramAndFile[ _MAX_PATH * 2 ];

		int success;
		success = GetLocalMachineStringRegistryEntry( "Software\\World Foundry\\GDK\\Attributes",
			"Editor", szProgramAndFile, sizeof( szProgramAndFile ) );
		if ( !success )
			strcpy( szProgramAndFile, "notepad.exe" );
		strcat( szProgramAndFile, " \"" );
		strcat( szProgramAndFile, szFilename );
		strcat( szProgramAndFile, "\"" );

		StartupInfo.cb = sizeof( StartupInfo );
		if ( CreateProcess( NULL, szProgramAndFile, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo ) )
		{
			WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
			if ( _xdata )
				free( _xdata );
			int len;
			_xdata = LoadTextFile( szFilename, len );
			if ( !_xdata || !*_xdata )
				free( _xdata ), _xdata = NULL;
		}
		else
			MessageBox( NULL, "Error", "Unable to start application", MB_OK );
		_unlink( szFilename );
	}
}

///////////////////////////////////////////////////////////////////////////////

double
XData::eval() const
{
	return _xdata ? 1.0 : 0.0;
}


bool
XData::enable( bool bEnabled )
{
	Button_SetText( hwndButton, _xdata ? "Edit..." : "New..." );
	if ( uiDialog::enable( bEnabled ) )
	{
		assert( hwndButton );
		Button_Enable( hwndButton, uiDialog::enable() );
		return true;
	}
	else
		return false;
}


int
XData::make_dialog_gadgets( HWND hPanel )
{
	int x;
	int width;

	assert( _td );

	assert( hPanel );
	assert( hInstance );
	assert( _td->name );

	int wLabel = uiDialog::make_dialog_gadgets( hPanel );

	hwndButton = theAttributes.CreateWindowEx(
		0,
		"BUTTON",
		"",
		0 | WS_TABSTOP,
		x = 5 + wLabel,
		theAttributes.y,
		width = 40,		// width
		16,			// height
		hPanel,
		NULL,
		hInstance,
		NULL );
	assert( hwndButton );
	SetWindowFont( hwndButton, (HFONT)theAttributes.ip->GetIObjParam()->GetAppHFont(), TRUE );
	SetWindowLong( hwndButton, GWL_USERDATA, (LONG)this );

	//_button = GetICustButton( hwndButton );
	//assert( _button );


	reset();

	return x + width + 1 + 7;
}
