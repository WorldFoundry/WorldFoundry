// lvlexp.hpp	LVL Exporter plugin for 3DS Max
//
//	(This is the new World Foundry replacement for LEVELCON.EXE)
//	Copyright 1996,1997 by Recombinant Ltd.  All rights reserved.
//

#ifndef _LVLEXP_HPP
#define _LVLEXP_HPP

#define LVL_EXP_VERSION "1.00"

#include "global.hpp"
#include <stl/vector.h>
#include <fstream.h>

//============================================================================

#define LEVELCON_VER "2.00.12"
// Version history:
// 2.00.00			First 3ds MAX version
// 2.00.01			Added path rotation and many bug fixes
// 2.00.02			Added template processing to INI file
// 2.00.03			Fixed path rotation key bug (AGAIN...), added light extraction
// 2.00.04			Fixed asset extraction (MovesBetweenRooms)
// 2.00.05			Added sloped surface extraction (first pass)
// 2.00.06	WBNIV	Store interpret INT32 and FIXED32 as strings
// 2.00.07	WBNIV	Added "warnings" dialog box
// 2.00.08	WBNIV	Rename aicomp2 to aicomp.exe
// 2.00.09	WBNIV	Added LEVELCONFLAG_SHORTCUT
// 2.00.10	WBNIV	Uses eval.dll to evaluate numbers
// 2.00.12	KTS		Changed PATH chunks so they don't have raw data followed by chunks

//============================================================================


class LVLExport : public SceneExport
{
public:
	LVLExport();
	~LVLExport();
	int ExtCount();
	const TCHAR* Ext(int n);			// Extensions supported for import/export modules
	const TCHAR* LongDesc();			// Long ASCII description (i.e. "Targa 2.0 Image File")
	const TCHAR* ShortDesc();			// Short ASCII description (i.e. "Targa")
	const TCHAR* AuthorName();			// ASCII Author name
	const TCHAR* CopyrightMessage(); 	// ASCII Copyright message
	const TCHAR* OtherMessage1();		// Other message #1
	const TCHAR* OtherMessage2();		// Other message #2
	unsigned int Version();				// Version number * 100 (i.e. v3.01 = 301)
	void		 ShowAbout(HWND hWnd);	// Show DLL's "About..." box
#if MAX_RELEASE < 2000
	int			 DoExport(const TCHAR *name,ExpInterface *ei,Interface *i);	// Export file
#else
	int			 DoExport(const TCHAR *name,ExpInterface *ei,Interface *i,int);	// Export file
#endif
	void		 ShowWarnings();
};


class LVLExportClassDesc : public ClassDesc
{
public:
	int 			IsPublic() { return TRUE; }
	void * 			Create(BOOL loading=FALSE) { return new LVLExport; }
	const TCHAR * 	ClassName() { return _T("LVL Export"); }
	SClass_ID 		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID 		ClassID() { return Class_ID(0x5ed50fd2, 0xd9d4026); }
	const TCHAR* 	Category() { return _T("");  }
};


#endif // _LVLEXP_HPP
