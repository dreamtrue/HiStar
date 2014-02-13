/* Copyright (C) 2013 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

// TwsSocketClient.h : main header file for the TWSSOCKETCLIENT DLL
//

#if !defined(AFX_TWSSOCKETCLIENT_H__2979840B_11D6_11D6_B0ED_00B0D074179C__INCLUDED_)
#define AFX_TWSSOCKETCLIENT_H__2979840B_11D6_11D6_B0ED_00B0D074179C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTwsSocketClientApp
// See TwsSocketClient.cpp for the implementation of this class
//

class CTwsSocketClientApp : public CWinApp
{
public:
	CTwsSocketClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTwsSocketClientApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTwsSocketClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TWSSOCKETCLIENT_H__2979840B_11D6_11D6_B0ED_00B0D074179C__INCLUDED_)
