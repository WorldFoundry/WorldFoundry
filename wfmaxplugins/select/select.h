// select.h

#ifndef __SELECT__H
#define __SELECT__H

#define STRICT
#include <windows.h>
#include "max.h"
#include <stddef.h>
#include <xstddef>
#include <vector>
#include <windowsx.h>
#include "resource.h"
#include "utilapi.h"
#include "../lib/wf_id.hp"

class Oad
{
public:
	Oad( const char* szClassName );
	~Oad();

	const char* ClassName() const;
	const char* find( INode* pNode, const char* ) const;

protected:
	char* _szClassName;
	void* _oad;
};


class Item
{
public:
	Item( HWND, const char* szExpression, INode*, int, int col, LPARAM );
	~Item();

	const char* GetText() const;

	int _col;

protected:
	char* _szExpression;
	char* _szText;
};


class Column
{
public:
	Column( HWND, const char*, int );
	~Column();

	const char* GetText() const;

	std::vector< Item* > _items;
	UINT _idMenu;

protected:
	HWND _hwnd;
	LV_COLUMN lvc;
	char* _szLabel;
};


typedef char* string;

struct upper_comp
{
	bool operator()( const string& a, const string& b ) const
	{
		return strcmp( a, b );
	}
};


class Select
{
public:
	Select();
	~Select();

	void DoDialog();
	void FillInDialog( HWND );
	void SelectSelected();
	void SaveConfiguration();
	bool LoadConfiguration();

	//
	void OnEditSelectNone();
	void OnEditSelectAll();
	void OnEditInvertSelection();
	void OnEditSelectByRegularExpression();
	void OnViewLargeIcons()			{ _SetView( ID_VIEW_LARGEICONS, LVS_ICON ); }
	void OnViewSmallIcons()			{ _SetView( ID_VIEW_SMALLICONS, LVS_SMALLICON ); }
	void OnViewList()				{ _SetView( ID_VIEW_LIST, LVS_LIST ); }
	void OnViewDetails()			{ _SetView( ID_VIEW_DETAILS, LVS_REPORT ); }
	void OnViewArrangeIcons();
	void OnHelpAbout();

	// Right mouse button popup menu
	void OnAscending();
	void OnDescending();
	void OnInsertColumn();
	void OnDeleteColumn();

	Interface* ip;

	enum { IDM_ARRANGEBY = 2000 };
	enum { SELECT_WORLD_FOUNDRY_MENUITEM = 40300 };

	WNDPROC wndProc;

	HWND _hDlg;
	std::vector< Column* > _columns;

	std::vector< Oad* > _oad;
	int _sortDirection;
	int _sortByColumn;
	HMENU _columnRCmenu;

	void _SetView( DWORD mi, DWORD lvs );
	void _SetArrangeBy( DWORD mi );
	HMENU _hMenu;

	std::vector< char* > _szInsertColumnExpressions;

	std::vector< string > _szSelect;
	std::vector< string > _szSelectHelp;
	std::vector< string > _szSelectDisplayName;

	INode* _pRoot;
	HWND hwndList;
	HWND hwndHeader;
	WNDPROC _headerWndProc;

	int _nSelectedColumn;
	HMENU _menuEdit;
	HMENU _menuArrangeBy;

	char szOadDir[ _MAX_PATH ];
	void PositionButtons( HWND );

protected:
	HWND _hwndOk;
	HWND _hwndCancel;

	HIMAGELIST _hSmall, _hLarge;
	HICON _hIcon;
	HICON _hIconSphere;

	DWORD _miView;
	DWORD _miArrangeBy;

	void _InsertColumn( const char*, int );
	void _DeleteColumn( int );

	enum { _MAX_CONFIGURATION_LINE = 20000 };
};

extern Select theSelect;

#endif	// __SELECT__H

