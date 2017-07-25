// Win32DownLoadDlg.h : header file
//

#pragma once
#include "resource.h"
#include <wininet.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


// CWin32DownLoadDlg dialog
class CWin32DownLoadDlg : public CDialog
{
// Construction
public:
	CWin32DownLoadDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WIN32DOWNLOAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnBtnDownload();

	afx_msg void OnBtnBrowser();

	afx_msg void OnEnChangeFileAddress();
	afx_msg void OnBtnPause();
public:
	afx_msg void OnBtnStop();
};
