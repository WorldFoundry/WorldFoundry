// attrib.h
// Coypright (c) 1997,1998 Recombinant Limited.  All Rights Reserved.
// by William B. Norris IV

#ifndef ATTRIB_H
#define ATTRIB_H

// process .oad files as IFF files
//#define DO_OAD_IFF
//#define DO_SAVE_OAD_IFF

#include "Max.h"
#include "resource.h"
#include "utilapi.h"
#include <windowsx.h>
#include "../lib/wf_id.hp"

class Oad;
class uiDialog;
class Symbol;
#include "pigtool.h"
#include "oas/oad.h"

TCHAR* GetString(int id);
ClassDesc* GetPropertiesDesc();
extern HINSTANCE hInstance;

class Attributes : public UtilityObj
{
public:
	IUtil *iu;
	Interface *ip;
	HWND hPanel;

	enum
	{
		SLOT_CLASS = 0,
		SLOT_OAD
	};

	Attributes();
	~Attributes();
	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void SelectionSetChanged(Interface *ip,IUtil *iu);

	void DeleteThis() {}

	void Init();
	void Destroy();

	void CreateAttributes( const char* szClassName = NULL );
	void DestroyAttributes( bool bApply = true );

	void Apply();
	void SetObjectSelected( int );
	void DestroyGadgets();
	void DestroyClass();

	uiDialog* findGadget( const char* szName, buttonType ) const;
	uiDialog* findGadget( const char* szName ) const;

	// public variables are better than globals
	int x;
	int y;
	HWND CreateWindowEx( DWORD dwExStyle,
		LPCTSTR lpClassName,
		LPCTSTR lpWindowName,
		DWORD dwStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu = NULL,
		HINSTANCE hInstance = ::hInstance,
		LPVOID lpParam = NULL
	);

	enum { _MAX_GADGETS = 250 };
	uiDialog* _gadgets[ _MAX_GADGETS ];
	int _numGadgets;

	HFONT _fontWingdings;
	HFONT _font;
	HWND _hwndClassName;
	INode* _theSelection;
	char szOadDir[ _MAX_PATH ];
	char szWorldFoundryDir[ _MAX_PATH ];
	char* szGroupName;

	void CopyAttributes();
	void PasteAttributes();
	void OnPrevious();
	void OnNext();
	void OnObjectName();
	void OnClassName();

	void FillInObjectList();
	void FillInClassReference( HWND hwndCombo, const char* szName = NULL );
	void refreshGadgetEnabledStates();
	void UpdateCopyPasteButtons( const char* szObjectName );
	void SetClassButton();

	char _szClassName[ 128 ];
public:
	HWND _hwndMax;
	HWND _hwndObjectName;
protected:
	HWND _hwndPrevious;
	HWND _hwndNext;
	HWND _hwndCopy;
	HWND _hwndPaste;

protected:
	void _Navigate( int );
public:
	HWND _hPanelClass;
protected:
	Oad* _oad;
	enum { _MAX_CLASSES = 50 };
	char* _szClasses[ _MAX_CLASSES ];
	int _numClasses;
public:
	HBRUSH _hBrushStatic;
};
extern Attributes theAttributes;


class Exception
{
public:
	long	data;
};


#if defined( NDEBUG )

//#define assert(exp)     ((void)0)

#else

//void max_assert( char*, char*, unsigned );
//#define assert(exp) (void)( (exp) || (max_assert(#exp, __FILE__, __LINE__), 0) )

#endif



#endif	// ATTRIB_H
