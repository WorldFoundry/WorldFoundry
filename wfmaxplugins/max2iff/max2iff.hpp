// wrlexp.hpp	WRL Exporter plugin for 3DS Max
//
//	(This is the new World Foundry replacement for 3DS2VRML.EXE)
//	Copyright 1996,1997 by Recombinant Ltd.  All rights reserved.
//

#ifndef _WRLEXP_HPP
#define _WRLEXP_HPP

// kts this is not used
//#define WRL_EXP_VERSION "2.00"

//#define NOMINMAX		// windows.h vs. STL conflict

#include "global.hpp"
#include "../lib/wf_id.hp"

class WRLExport : public SceneExport
{
public:
	WRLExport();
	~WRLExport();
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
	DoExport( const TCHAR* name, ExpInterface* ,Interface* gi );
#else
	DoExport( const TCHAR* name, ExpInterface* ,Interface* gi, int );
#endif

	int			 ExportObject( INode* pNode, _IffWriter* iff, const max2iffOptions* );

protected:
	Options _options;
};


class WRLExportClassDesc : public ClassDesc
{
	public:
	int 			IsPublic() { return TRUE; }
	void * 			Create(BOOL loading=FALSE) { return new WRLExport; }
	const TCHAR * 	ClassName() { return _T("IFF Export"); }
	SClass_ID 		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID 		ClassID() { return Max2Iff_ClassID; }
	const TCHAR* 	Category() { return _T("");  }
};


#endif // _WRLEXP_HPP
