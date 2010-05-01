// help.c

#include <assert.h>
#include <stdio.h>

#include "exprtn.h"
#include "keys.h"

#include "../util.h"
#include "../debug.h"

#include "oaddlg.h"
#include "help.h"
#include "dialogbox.h"


UserMouse cursorHelp = {
	0, 0,
		{ // mask
		0x80,0x7C,
		0xC0,0xCE,
		0xE1,0x87,
		0xF1,0x87,
		0xF8,0x06,
		0xFC,0x0C,
		0xFE,0x18,
		0xFF,0x30,
		0xF8,0x30,
		0xD8,0x00,
		0x8C,0x30,
		0x0C,0x30,
		0x06,0x00,
		0x06,0x00,
		0x00,0x00,
		0x00,0x00,
		},
		{ // data
		0x80,0x7C,
		0xC0,0xCE,
		0xE1,0x87,
		0xF1,0x87,
		0xF8,0x06,
		0xFC,0x0C,
		0xFE,0x18,
		0xFF,0x30,
		0xF8,0x30,
		0xD8,0x00,
		0x8C,0x30,
		0x0C,0x30,
		0x06,0x00,
		0x06,0x00,
		0x00,0x00,
		0x00,0x00,
		}
	};


int
HelpButton::storedDataSize()
	{
	return 0;
	}


dataValidation
HelpButton::copy_to_xdata( byte* & saveData, typeDescriptor* )
	{
	assert( 0 );
	return DATA_OK;
	}

HelpButton::~HelpButton()
	{
	}


void
HelpButton::make_dialog_gadgets( DialogBox* db )
	{
	_dlg = db->dlgFollow;

	makeButton( _dlg, "?", DLG_BUTTON, 0, db->y );
	_dlg->key = '?';
	_dlg->x = (_dlg-1)->x + (_dlg-1)->width + 10;
	_dlg->width = 75;

	db->main_feel[ db->idxFeel ].feel = feel_button_up,
	db->main_feel[ db->idxFeel++ ].index = db->idxButton;

	SET_BUTTON_SHEET( _dlg, 0 );			// Global button
	SET_BUTTON_INDEX( _dlg, db->idxGlue );
	++db->idxGlue;
	}


void
HelpButton::feel( int mouse )
	{
	int xScrMouse, yScrMouse;

	gfx_user_mform( &cursorHelp );
	gfx_set_mform( C_USER, TRUE );

	theDialogBox->startHelpSystem();

	do
		{
		gfx_wait_input();
		xScrMouse = GC->mouse_x - theDialogBox->dlgRoot->sx;
		yScrMouse = GC->mouse_y - theDialogBox->dlgRoot->sy;

		theDialogBox->updateHelpSystem( xScrMouse, yScrMouse );
		}
	until ( GC->mouse_button & 2 );

	theDialogBox->stopHelpSystem();

	gfx_set_mform( C_ARROW, TRUE );
	}
