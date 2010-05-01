
/**********************************************************************
 *<
	FILE: 3dsimp.h

	DESCRIPTION:  .3DS file import module header file

	CREATED BY: Tom Hudson

	HISTORY: created 26 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

class WorldFoundryImport : public SceneImport {
public:
					WorldFoundryImport();
					~WorldFoundryImport();
	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
#if MAX_RELEASE < 2000
	int				DoImport(const TCHAR *name,ImpInterface *i,Interface *gi);	// Import file
#else
	int				DoImport(const TCHAR *name,ImpInterface *i,Interface *gi,int);	// Import file
#endif
	};

