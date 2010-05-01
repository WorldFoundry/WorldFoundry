// radio.c

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../types.h"
#include "../debug.h"
#include "../util.h"
#include "source/oas/oad.h"

#include "dialog.h"

#include "oaddlg.h"
#include "radio.h"
#include "dialogbox.h"


int
RadioButton::storedDataSize()
	{
	return sizeof( long );
	}


RadioButton::~RadioButton()
	{
	for ( int i=0; i<_nItems; ++i )
		free( _menuItems[ i ] );
	}


dataValidation
RadioButton::copy_to_xdata( byte* & saveData, typeDescriptor* td )
	{
	assert( _td );

	(*(long*)saveData) = BUTTON_INT32;	saveData += 4;
//	debug( "name = %s", td->name );
	strncpy( saveData, td->name, strlen( td->name ) + 1 );	// include NUL
	saveData += strlen( td->name ) + 1;
	(*(long*)saveData) = radioVariable;		saveData += 4;

	return DATA_OK;
	}


void
RadioButton::reset( typeDescriptor* td )
	{
	assert( td );

	reset( td->def );
	}


void
RadioButton::reset( int i )
	{
	assert( _td->min <= i && i <= _td->max );

	radioVariable = i;
	}


void
RadioButton::reset( void* & saveData )
	{
	reset( *(fixed32*)saveData );
	saveData = (void*)( (byte*)saveData + 4 );
	}


void
RadioButton::make_dialog_gadgets( DialogBox* db )
	{
	int maxWidth = 0;

	assert( _td );

	makeLabel( db->dlgFollow, _td->xdata.displayName, db->x, db->y );
	++db->dlgFollow, ++db->idxButton;

	_nItems = 0;

	char* strBegin = _td->string;
	char* strSeperator;

	do
		{
		int len;
		strSeperator = strchr( strBegin, SEPERATOR );

		if ( strSeperator )
			len = strSeperator - strBegin;
		else
			{ // Take to end of string
			len = strlen( strBegin );
			}

		_menuItems[ _nItems ] = (char*)malloc( len + 1 );
		assert( _menuItems[ _nItems ] );
		strncpy( _menuItems[ _nItems ], strBegin, len );
		*( _menuItems[ _nItems ] + len ) = '\0';

		int w = 4 + len * 8 + 4;
		if ( w > maxWidth )
			maxWidth = w;

		++_nItems;

		strBegin = strSeperator + 1;
		}
	while ( strSeperator );

	assert( _nItems > 0 );
	assert( _td->min == 0 );
	assert( _td->max+1 == _nItems );

	int xSeperator = 5;
	for ( int i=0; i<_nItems; ++i )
		{
		makeButton( db->dlgFollow, _menuItems[ i ], DLG_BUTTON, db->x, db->y );
		db->dlgFollow->x = (db->dlgFollow-1)->x + (db->dlgFollow-1)->width + xSeperator;
		db->dlgFollow->width = maxWidth;

		db->mainrad[ db->idxRadio ].index = db->idxButton;
		db->mainrad[ db->idxRadio ].feel = feel_radio;
		db->mainrad[ db->idxRadio ].variable = &radioVariable;
		db->mainrad[ db->idxRadio ].value = i;
		++db->idxRadio;
		++db->dlgFollow, ++db->idxButton;

		xSeperator = 0;
		}
	--db->dlgFollow, --db->idxButton;

	reset( _td );

	SET_BUTTON_INDEX( db->dlgFollow, db->idxGlue );
	SET_BUTTON_SHEET( db->dlgFollow, db->idxPropertySheet );
	++db->idxGlue;

	db->y += db->yInc;
	}
