// colour.h

#ifndef __UTIL__H
#define __UTIL__H

#include "Max.h"
#include "resource.h"
#include "utilapi.h"
#include <windowsx.h>
#include "oas/pigtool.h"
#include "oas/oad.h"
#include "../lib/wf_id.hp"

TCHAR* GetString( int id );

extern ClassDesc* GetPropertiesDesc();

extern HINSTANCE hInstance;

class ClassColour;

class Colour : public UtilityObj
{
public:
	IUtil* iu;
	Interface* ip;
	HWND _hPanel;

	Colour();
	~Colour();
	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void SelectionSetChanged(Interface *ip,IUtil *iu);

	void DeleteThis() {}

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);

	void LoadScheme();
	void SaveScheme();
	void Apply();


	HFONT _font;
	char szOadDir[ _MAX_PATH ];
	char szWorldFoundryDir[ _MAX_PATH ];

protected:
	void LoadColourSchemeFile();

//	enum {
//		SLOT_CLASS,
//		SLOT_OAD
//	};
	enum { _MAX_CLASSES = 50 };
	int _numClasses;
	char* _szClasses[ _MAX_CLASSES ];
	ClassColour* _class[ _MAX_CLASSES ];

	char szColourSchemeFilename[ _MAX_PATH ];
	char szColourSchemeInitialDir[ _MAX_PATH ];
};

extern Colour theColour;

#endif
