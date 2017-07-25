// Win32DownLoad.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWin32DownLoadApp:
// See Win32DownLoad.cpp for the implementation of this class
//

class CWin32DownLoadApp : public CWinApp
{
public:
	CWin32DownLoadApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWin32DownLoadApp theApp;