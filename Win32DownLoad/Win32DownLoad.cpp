// Win32DownLoad.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Win32DownLoad.h"
#include "Win32DownLoadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWin32DownLoadApp

BEGIN_MESSAGE_MAP(CWin32DownLoadApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWin32DownLoadApp construction

CWin32DownLoadApp::CWin32DownLoadApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWin32DownLoadApp object

CWin32DownLoadApp theApp;


// CWin32DownLoadApp initialization

BOOL CWin32DownLoadApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CWin32DownLoadDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
