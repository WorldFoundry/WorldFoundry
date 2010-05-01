// template.c

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../types.h"
#include "../debug.h"
#include "../util.h"
#include "levels/oas/oad.h"

#include "dialog.h"

#include "oaddlg.h"
#include "template.h"
#include "dialogbox.h"


dataValidation
Template::copy_to_xdata( byte* & saveData, typeDescriptor* td )
	{
	assert( _td );
	assert( _dlg );

	return DATA_OK;
	}


void
Template::reset( typeDescriptor* td )
	{
	assert( td );

	reset( (bool)td->def );
	}


void
Template::reset( template b )
	{
	assert( _dlg );

	_dlg->radio = b;

	_dlg->text = _dlg->radio ? _td->boolLabelOn : _td->boolLabelOff;
	}


void
Template::feel( int mouse )
	{
	Dialog* d = _dlg;
	assert( d );

	assert( d->radio == 0 || d->radio == 1 );
	d->radio ^= 1;
	reset( (bool)d->radio );

	draw_item( d );
	}


void
Template::make_dialog_gadgets( DialogBox* db )
	{
	assert( _td );

	int width = makeLabel( db->dlgFollow, _td->xdata.displayName, db->x, db->y );
	++db->dlgFollow, ++db->idxButton;

	makeButton( db->dlgFollow, "", DLG_BUTTON, db->x, db->y );
	_dlg = db->dlgFollow;
	_dlg->x += width + 5;
	_dlg->width = 9 * MAX( strlen( _td->boolLabelOff ), strlen( _td->boolLabelOn ) );

	db->main_feel[ db->idxFeel ].feel = feel_button_up,
	db->main_feel[ db->idxFeel++ ].index = db->idxButton;

	reset( _td );

	SET_BUTTON_INDEX( _dlg, db->idxGlue );
	SET_BUTTON_SHEET( _dlg, db->idxPropertySheet );
	++db->idxGlue;

	db->y += db->yInc;
	}
