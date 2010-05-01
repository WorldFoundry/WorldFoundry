// lvlexp.hpp	LVL Exporter plugin for 3DS Max
//
//	(This is the new World Foundry replacement for LEVELCON.EXE)
//	Copyright 1996,1997 by Recombinant Ltd.  All rights reserved.
//

#ifndef _LVLEXP_HPP
#define _LVLEXP_HPP

#define LVL_EXP_VERSION "1.00"

#include "global.hpp"
//#include <stl/vector.h>
//#include <fstream.h>
#include "scene.hpp"
//#include "../lib/wf_id.hp"

//============================================================================

#define LEVELCON_VER "1.0.4"
// Version history:
// 1.0.0

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
};


class LVLExportClassDesc : public ClassDesc
{
public:
	int 			IsPublic() { return TRUE; }
	void * 			Create(BOOL loading=FALSE) { return new LVLExport; }
	const TCHAR * 	ClassName() { return Max2IffLvl_ClassName; }
	SClass_ID 		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID 		ClassID() { return Max2IffLvl_ClassID; }
	const TCHAR* 	Category() { return _T("");  }
};





#endif // _LVLEXP_HPP
