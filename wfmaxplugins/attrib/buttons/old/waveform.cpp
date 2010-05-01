// waveform.c

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../types.h"
#include "../debug.h"
#include "../util.h"
#include "source/oas/oad.h"

#include "dialog.h"
#include "oaddlg.h"
#include "dialogbox.h"
#include "waveform.h"

int
Waveform::storedDataSize()
	{
	return 0;
	}


dataValidation
Waveform::copy_to_xdata( byte* & saveData, typeDescriptor* )
	{
	assert( 0 );
	return DATA_OK;
	}


void
Waveform::reset( typeDescriptor* td )
	{
	}


void
Waveform::reset( void* & saveData )
	{
	}


void
Waveform::drawNode( int nNode, int colorLine, int colorNode )
	{
	assert( 0 <= nNode && nNode < _nNodes );

	_xWaveform = _dlg->x;
	_yWaveform = _dlg->y + _dlg->height - 1;

	int x1, y1;

	x1 = _x[nNode-1] + _xWaveform;
	y1 = -_y[nNode-1] + _yWaveform;
	if ( nNode > 0 )
		{
		int x2 = _x[nNode] + _xWaveform;
		int y2 = -_y[nNode] + _yWaveform;
		theDialogBox->line( x1, y1, x2, y2, colorLine );
		}
	theDialogBox->drawKnob( x1, y1, 3, colorNode );

	x1 = _x[nNode] + _xWaveform;
	y1 = -_y[nNode] + _yWaveform;
	if ( nNode < _nNodes-1 )
		{
		int x2 = _x[nNode+1] + _xWaveform;
		int y2 = -_y[nNode+1] + _yWaveform;
		theDialogBox->line( x1, y1, x2, y2, colorLine );
		}
	theDialogBox->drawKnob( x1, y1, 3, colorNode );

	if ( nNode < _nNodes-1 )
		{
		x1 = _x[nNode+1] + _xWaveform;
		y1 = -_y[nNode+1] + _yWaveform;
		theDialogBox->drawKnob( x1, y1, 3, colorNode );
		}
	}


void
Waveform::see()
	{
	Dialog* d = _dlg;

	draw_outbox( d, MDGRAY );

	_xWaveform = _dlg->x;
	_yWaveform = _dlg->y + _dlg->height - 1;

	for ( int i=0; i<_nNodes; ++i )
		{
		int x1 = _x[i] + _xWaveform;
		int y1 = -_y[i] + _yWaveform;
		if ( i != _nNodes-1 )
			{ // Don't draw from the last point
			int x2 = _x[i+1] + _xWaveform;
			int y2 = -_y[i+1] + _yWaveform;
			theDialogBox->line( x1, y1, x2, y2, WHITE );
			}
		theDialogBox->drawKnob( x1, y1, 3, RED );
		}
	}


static void
see_waveform( Dialog* d )
	{
	assert( BUTTON_INDEX( d ) );
	DialogGlue* dg = &theDialogBox->mainglue[ BUTTON_INDEX( d ) ];

	Waveform* theGadget = (Waveform*)dg->theGadget;
	assert( theGadget );

	if ( BUTTON_SHEET( d ) == theDialogBox->nPropertySheet )
		theGadget->see();
	}


int
Waveform::hitKnob( int xMouse, int yMouse, int radius )
	{
	_xWaveform = _dlg->sx;
	_yWaveform = _dlg->sy + _dlg->height - 1;

	for ( int i=0; i<_nNodes; ++i )
		{
		int xScr = _x[i] + _xWaveform;
		int yScr = -_y[i] + _yWaveform;

		if ( (xScr-radius/2 <= xMouse && xMouse <= xScr+radius/2 )
		&&   (yScr-radius/2 <= yMouse && yMouse <= yScr+radius/2 )
		)
			return i;
		}

	return -1;
	}


void
Waveform::check_xbounds( int nKnob )
	{
	if ( nKnob == 0 )
		_x[nKnob] = 0;
	else if ( nKnob == _nNodes-1 )
		_x[nKnob] = _dlg->width-1;
	else
		{
		if ( _x[nKnob] < 0 ) _x[nKnob] = 0;
	 	if ( _x[nKnob] >= _dlg->width ) _x[nKnob] = _dlg->width-1;
		if ( _x[nKnob] < _x[nKnob-1] ) _x[nKnob] = _x[nKnob-1];
		if ( _x[nKnob] > _x[nKnob+1] ) _x[nKnob] = _x[nKnob+1];
		}
	}


void
Waveform::check_ybounds( int nKnob )
	{
	if ( _y[nKnob] < 0 ) _y[nKnob] = 0;
	if ( _y[nKnob] >= _dlg->height ) _y[nKnob] = _dlg->height-1;
	}


void
Waveform::feel( int mouse )
	{
	Dialog* d = _dlg;

	xScrMouse = GC->mouse_x;
	yScrMouse = GC->mouse_y;

	int nKnob;
	if ( (nKnob = hitKnob( xScrMouse, yScrMouse, 3*2 )) != -1 )
		{
		xMouse = yMouse = 0;

		do
			{
			gfx_wait_input();
			xScrMouse = GC->mouse_x;
			yScrMouse = GC->mouse_y;

			out_frame( d );
			drawNode( nKnob, MDGRAY, MDGRAY );

			_x[nKnob] = xScrMouse - d->sx;
			check_xbounds( nKnob );
			_y[nKnob] = d->height - (yScrMouse - d->sy);
			check_ybounds( nKnob );

			drawNode( nKnob, WHITE, RED );
			}
		until ( !GC->mouse_button );

		// Update the button based on what was selected
		//reset();
		refresh();
		}
	}


void
Waveform::make_dialog_gadgets( DialogBox* db )
	{
	assert( _td );

	_wScreen = db->si.sc_width;
	_hScreen = db->si.sc_height;

	int widthLabel = makeLabel( db->dlgFollow, _td->xdata.displayName, db->x, db->y );
	++db->dlgFollow, ++db->idxButton;

	makeButton( db->dlgFollow, "Grid", DLG_BUTTON, db->x, db->y + db->yInc );
	++db->dlgFollow, ++db->idxButton;

	makeButton( db->dlgFollow, "Snap", DLG_BUTTON, db->x, db->y + db->yInc*2 );
	++db->dlgFollow, ++db->idxButton;


	_dlg = db->dlgFollow;
	makeButton( _dlg, "WAVEFORM", DLG_BUTTON, db->x, db->y );
	_dlg->x += widthLabel + 5;
	_dlg->width = 240;
	_dlg->height = db->yInc * 4.5;


	for ( int i=0; i<_nNodes; ++i )
		{
		_x[i] = _dlg->width / _nNodes * i;
		_y[i] = rand() % _dlg->height;
		}
	_x[_nNodes-1] = _dlg->width-1;


	db->main_feel[ db->idxFeel ].feel = feel_button_down;
	db->main_feel[ db->idxFeel++ ].index = db->idxButton;
	db->mainsee[ db->idxSee ].see = see_waveform;
	db->mainsee[ db->idxSee++ ].index = db->idxButton;

	reset( _td );

	_dlg->flags = db->idxGlue;
	SET_BUTTON_INDEX( _dlg, db->idxGlue );
	SET_BUTTON_SHEET( _dlg, db->idxPropertySheet );
	++db->idxGlue;

	db->y += db->yInc * 5;
	}
