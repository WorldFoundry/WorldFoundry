// Debug.h : main header file for the DEBUG application
//

#if !defined(AFX_DEBUG_H__CAC7E742_FFE0_11D0_A343_00C0F01592AA__INCLUDED_)
#define AFX_DEBUG_H__CAC7E742_FFE0_11D0_A343_00C0F01592AA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDebugApp:
// See Debug.cpp for the implementation of this class
//

class CDebugApp : public CWinApp
{
public:
	CDebugApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDebugApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#define NITEMS( __array__ )		( sizeof( __array__ ) / sizeof( *__array__ ) )

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUG_H__CAC7E742_FFE0_11D0_A343_00C0F01592AA__INCLUDED_)
