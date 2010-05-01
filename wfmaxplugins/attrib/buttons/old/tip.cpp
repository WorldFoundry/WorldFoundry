// tip.c -- "Tip of the Day"
//
// Copyright 1996 Cave Logic Studios, Ltd.  All Rights Reserved.
//
// 04 Feb 96	WBNIV	Created
//

#include <assert.h>
#include <stdio.h>
#include <malloc.h>

#include "exprtn.h"
#include "keys.h"

#include "../util.h"
#include "../debug.h"

#include "oaddlg.h"
#include "tip.h"
#include "dialogbox.h"

#include "tipofday.3de"			// dialog definition

static TipButton* theTipButton;


int
TipButton::storedDataSize()
	{
	return 0;
	}


dataValidation
TipButton::copy_to_xdata( byte* & saveData, typeDescriptor* )
	{
	assert( 0 );
	return DATA_OK;
	}

TipButton::TipButton( typeDescriptor* td ) : uiDialog( td )
	{
	theTipButton = this;
	}


TipButton::~TipButton()
	{
	}


void
TipButton::make_dialog_gadgets( DialogBox* db )
	{
	// Calculate "Tip of the Day" file and number of entries
	const char* oadPath = GetOadDir();

	sprintf( _tipsFile, "%stips.txt", oadPath );

	FILE* fp = fopen( _tipsFile, "rt" );
	assert( fp );

	_nTips = 0;
	char szTipText[ 1024 ];
	while ( fgets( szTipText, sizeof( szTipText ), fp ) )
		++_nTips;

	fclose( fp );


	_dlg = db->dlgFollow;

	makeButton( _dlg, "Tip", DLG_BUTTON, 0, db->y );
	//_dlg->key = '?';
	_dlg->x = (_dlg-1)->x + (_dlg-1)->width + 10;
	_dlg->width = 75;

	db->main_feel[ db->idxFeel ].feel = feel_button_up,
	db->main_feel[ db->idxFeel++ ].index = db->idxButton;

	SET_BUTTON_SHEET( _dlg, 0 );			// Global button
	SET_BUTTON_INDEX( _dlg, db->idxGlue );
	++db->idxGlue;
	}


void
TipButton::nextTip()
	{
	++_nTipOfTheDay;
	if ( _nTipOfTheDay >= _nTips )
		_nTipOfTheDay = 0;

	drawTipText();
	}

void
TipButton::previousTip()
	{
	--_nTipOfTheDay;
	if ( _nTipOfTheDay < 0 )
		_nTipOfTheDay = _nTips-1;

	drawTipText();
	}


static void
tip_feel_exit( Dialog* d, int mouse )
	{
	int status;

	if ( mouse && !press_button( d ) )
		return;

	dialog_done = 1;
	}


static void
tip_feel_next_tip( Dialog* d, int mouse )
	{
	int status;

	if ( mouse && !press_button( d ) )
		return;

	assert( theTipButton );
	theTipButton->nextTip();
	}


static void
tip_feel_previous_tip( Dialog* d, int mouse )
	{
	int status;

	if ( mouse && !press_button( d ) )
		return;

	assert( theTipButton );
	theTipButton->previousTip();
	}


static FeelSub tip_feel[]=
	{
	OK, tip_feel_exit,
	NEXT_TIP, tip_feel_next_tip,
	PREV_TIP, tip_feel_previous_tip,
	-1, FNULL
	};


void
wrapText( const char* const s, const int x, const int y, const int w,
	const int h, const int fc, const int bc )
	{
	int nCharsPerLine = w/8;
	int yPos = y;
//	debug( "width: [%d]  nCharsPerLine: [%d]", w, nCharsPerLine );
//	debug( "s: [%s]", s );

	int nCharsToPrint = 0;
	char szLineToPrint[ 132 ];
	char* szLine = (char*)s;
	char* szFollow = szLine;
	char* space;

	for ( /* ALL THE TEXT */ ;; )
		{
		while ( *szLine == ' ' )
			++szLine;

		for ( /* EACH LINE */ ;; )
			{
			space = strchrs( szFollow, " ." );

			if ( space && *space == '.' )
				++space;

			if ( !space || ( space - szLine > nCharsPerLine ) )
				break;

			nCharsToPrint = space - szLine;

			szFollow = space+1;

			if ( *space == '' )
				break;
			}

		if ( !*szLine )
			break;

		assert( nCharsToPrint < sizeof( szLineToPrint ) );
		strncpy( szLineToPrint, szLine, nCharsToPrint );
		szLineToPrint[ nCharsToPrint ] = '\0';

//		puts( szLineToPrint );
		gfx_8x16_text( szLineToPrint, x, yPos, fc, bc );
		yPos += 16;

		szLine += nCharsToPrint;
		if ( *(szFollow-1) == '' )
			++szLine;
		}
	}


void
TipButton::drawTipText()
	{
	cnull << "_tipsFile: [" << _tipsFile << "]" << endl;

	FILE* fp = fopen( _tipsFile, "rt" );
	assert( fp );

	const MAX_TIP_TEXT = 2000;
	char* szTipText = (char*)alloca( MAX_TIP_TEXT );
	assert( szTipText );

	cnull << "_nTipOfTheDay = " << _nTipOfTheDay << endl;
	for ( int i = 0; i < _nTipOfTheDay; ++i )
		{
		cnull << "reading tips #" << i << endl;
		fgets( szTipText, MAX_TIP_TEXT, fp );
		}
	cnull << "reading tip..." << endl;
	fgets( szTipText, MAX_TIP_TEXT, fp );
	cnull << "tip=" << szTipText << endl;
	cnull << "terminate string" << endl;
	*( szTipText + strlen( szTipText ) - 1 ) = '\0';		// Remove trailing LF
	cnull << "tip=" << szTipText << endl;

	cnull << "close file...";
	assert( fp );
	fclose( fp );
	cnull << "closed" << endl;

	cnull << "render tip" << endl;

	gfx_cblock( Tip[TIP_TEXT].sx+1, Tip[TIP_TEXT].sy+1,
		Tip[TIP_TEXT].width-2, Tip[TIP_TEXT].height-2, DKGRAY );
	wrapText( szTipText, 4+Tip[TIP_TEXT].sx, 8+Tip[TIP_TEXT].sy,
		Tip[TIP_TEXT].width - 8, Tip[TIP_TEXT].height - 16, WHITE, DKGRAY );
	}


void
TipButton::feel( int mouse )
	{
	init_dialog( Tip, NULL, NULL );
	ready_dialog( Tip, NULL, NULL, tip_feel, NULL, NULL, NULL );
	center_dialog( Tip );
	save_under_dialog( Tip );
	draw_dialog( Tip );

	//CRASH//debug( "Render icon" );
	gfx_blit( kyle_icon_data, Tip[ICON].sx, Tip[ICON].sy,
		Tip[ICON].width, Tip[ICON].height, 15 );

	_nTipOfTheDay = rand() % _nTips;
	drawTipText();

	do_dialog( Tip, -1 );
	restore_under_dialog();

	dialog_done = 0;			// override inside dialog box's setting
	}
