/**********************************************************************
 *<
	FILE: util.h

	DESCRIPTION:

	CREATED BY: Rolf Berteig

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __UTIL__H
#define __UTIL__H

#include <max.h>
#include "resource.h"
#include <utilapi.h>
#include "../lib/wf_id.hp"

//TCHAR* GetString( int id );
extern ClassDesc* GetPropertiesDesc();
void Error( const char* szMsg );

#define NUM_ITEMS( _array_ )	( sizeof( (_array_) ) / sizeof( *(_array_) ) )

extern HINSTANCE hInstance;


class UpdateButtonsCallback : public RedrawViewsCallback
{
public:
	virtual void proc( Interface* ip );
};


struct DebuggingStream
{
	int id;
	char* name;
	char* szSwitch;
};


struct StreamDestination
{
	char* name;
	char* szSwitch;
};


class Toolbar : public UtilityObj
{
public:
	IUtil* iu;
	Interface* ip;
	HWND _hPanel;
	HWND _hPanelButtons;
	HWND _hPanelParameters;
	HWND _hPanelDebugStreams;
	HWND _hWnd;

	Toolbar();
	~Toolbar();
	void BeginEditParams( Interface* ip, IUtil* iu );
	void EndEditParams( Interface* ip, IUtil *iu );
	void SelectionSetChanged( Interface* ip, IUtil* iu );

	void DeleteThis() {}

	void Init( HWND hWnd );
	void Destroy( HWND hWnd );

	void Command( HWND, int );

	void _setButtons_Level();

protected:
	char szWorldFoundryDir[ _MAX_PATH ];
	char szLevelsDir[ _MAX_PATH ];
	HFONT _font;

	UpdateButtonsCallback buttonsProc;

	void MakeLevel();
	void RunLevel();
	void CleanLevel();
	void ExportSelectedObject();
	void LintLevel();

	int ReadCheckBox( HWND hwnd, const char* szKey, UINT button );
	int ReadRadioButton( HWND hwnd, const char* szKey, UINT button, UINT buttonDefault );
	int ReadInteger( HWND hwnd, const char* szKey, UINT button );
	void ReadString( HWND hwnd, const char* szKey, UINT button );

	void SaveCheckBox( HWND hwnd, const char* szKey, UINT button );
	void SaveRadioButton( HWND hwnd, const char* szKey, UINT button );
	void SaveInteger( HWND hwnd, const char* szKey, UINT button );
	void SaveString( HWND hwnd, const char* szKey, UINT button );
};

extern Toolbar theToolbar;

#endif
