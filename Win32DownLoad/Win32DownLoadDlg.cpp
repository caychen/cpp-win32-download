// Win32DownLoadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Win32DownLoad.h"
#include "Win32DownLoadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HWND hWnd = NULL;
HWND hProgressBarWnd = NULL;
HWND hDownLoadBtnWnd = NULL;
HWND hBrowserBtnWnd = NULL;
HWND hPauseBtnWnd = NULL;
HWND hFileAddrWnd = NULL;
HWND hProgressRange = NULL;
HWND hStopBtnWnd = NULL;
BOOL bPaused = FALSE;
BOOL bStop = FALSE;

DWORD WINAPI PBThreadProc(LPVOID lpParameter)
{
	ShowWindow(hProgressBarWnd,SW_SHOW);
	PBRANGE range;

	SendMessage(hProgressBarWnd, PBM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELPARAM(0, 100)));   //设置进度条的范围
	SendMessage(hProgressBarWnd, PBM_GETRANGE,    //获取进度条的范围
		(WPARAM)TRUE,          //TRUE 表示返回值为范围的最小值,FALSE表示返回最大值
		(LPARAM)&range);

	DWORD dwFlags;
	//int lastError;
	if(!InternetGetConnectedState(&dwFlags, 0))//wininet.h
	{
		MessageBox(hWnd, _T("没有网络连接！"), _T("提示"), MB_OK);
		//lastError = GetLastError();
		return 0;
	}

	TCHAR strAgent[64] = {0};
	_stprintf_s(strAgent, _T("Agent %ld"), timeGetTime());//mmsystem.h,winmm.lib
	HINTERNET hopen;
	if(!(dwFlags & INTERNET_CONNECTION_PROXY))
	{
		hopen = InternetOpen(strAgent, INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, NULL, 0);
	}
	else
	{
		hopen = InternetOpen(strAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	}

	if(!hopen)
	{
		MessageBox(hWnd, _T("打不开网络连接！"), _T("提示"), MB_OK);
		return 0;
	}

	FILE *fp = NULL;
	TCHAR szUrlAdd[MAX_PATH] = {0}; 
	GetWindowText(hFileAddrWnd, szUrlAdd, MAX_PATH );

	//TCHAR pBuf[MAX_PATH] = {0};                            //存放路径的变量
	//GetCurrentDirectory(MAX_PATH, pBuf);                   //获取程序的当前目录
	//_tcscat_s(pBuf, _tcsrchr(szUrlAdd, '/'));

	TCHAR szFilePath[MAX_PATH] = {0};
	GetDlgItemText(hWnd,IDC_FILE_SAVEPATH,szFilePath,MAX_PATH);
	if(!_tcscmp(szFilePath,_T("")))
	{
		MessageBox(hWnd,_T("请设置保存路径后再下载"),_T("提示"),MB_OK);
		EnableWindow(hFileAddrWnd,TRUE);
		EnableWindow(hStopBtnWnd,FALSE);
		EnableWindow(hBrowserBtnWnd,TRUE);
		return 0;
	}

	_tfopen_s(&fp,szFilePath, _T("ab+"));
	if(fp == NULL)
	{
		MessageBox(hWnd, _T("打开本地文件失败！"), _T("提示"), MB_OK);
		EnableWindow(hFileAddrWnd,TRUE);
		return 0;
	}

	int fileSize = 0;
	TCHAR *pszHead = NULL;
	TCHAR szHead[256] = {0};

	if (bPaused)
	{
		bPaused = FALSE;
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		_stprintf_s(szHead, _T("Range: bytes=%d-\r\nAccept: */*\r\n\r\n"), fileSize);
		pszHead = szHead;
	}
	else
	{
		pszHead = _T("Accept: */*\r\n\r\n");
	}

	void *szTemp[16384] = {0};
	HINTERNET hConnect;
	if(!(hConnect = InternetOpenUrl(hopen, szUrlAdd, szHead, wcslen(szHead), INTERNET_FLAG_DONT_CACHE|INTERNET_FLAG_PRAGMA_NOCACHE, 0)))
	{
		fclose(fp);
		MessageBox(hWnd, _T("打开网络地址失败！"), _T("提示"), MB_OK);
		EnableWindow(hFileAddrWnd,TRUE);
		return 0;
	}

	DWORD dwByteToRead = 0, dwSizeOfRq = 4, dwBytes = 0, dwSize = 0, sum = fileSize;
	if(!HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH|HTTP_QUERY_FLAG_NUMBER, (LPVOID)&dwByteToRead, &dwSizeOfRq, NULL))
	{
		dwByteToRead = 0;
	}

	EnableWindow(hDownLoadBtnWnd, FALSE);
	EnableWindow(hPauseBtnWnd, TRUE);
	SetDlgItemText(hWnd, IDC_BTN_DOWNLOAD, _T("正在下载..."));
	TCHAR sumData[64] = {0};

	do
	{
		if(!InternetReadFile(hConnect, szTemp, 16384, &dwSize))
		{
			InternetCloseHandle(hopen);
			fclose(fp);
			MessageBox(hWnd, _T("网络中断！"), _T("提示"), MB_OK);
			EnableWindow(hFileAddrWnd,TRUE);
			return 0;
		}

		if(dwSize == 0)
		{
			break;
		}
		else
		{
			sum += dwSize;
			fwrite(szTemp, dwSize, 1, fp);
			double percentValue = (sum * 100.0) / (dwByteToRead + fileSize);
			
			_stprintf_s(sumData, _T("%.2f %%"), percentValue);

			//设置进度条当前值
			SendMessage(hProgressBarWnd, PBM_SETPOS, (WPARAM)percentValue, (LPARAM)0);

			//SetDlgItemText(hWnd, IDC_BTN_DOWNLOAD, sumData);
			SetDlgItemText(hWnd, IDC_PROGRESS_RANGE, sumData);

		}
	}while(!bPaused && !bStop);

	if (bPaused && !bStop) 
	{
		InternetCloseHandle(hopen);
		fclose(fp);
		MessageBox(hWnd, _T("已暂停！"), _T("提示"), MB_OK);
		SetDlgItemText(hWnd, IDC_BTN_PAUSE, _T("继续"));
		return 0;
	}
	if(bStop)//bStop = true
	{
		InternetCloseHandle(hopen);
		fclose(fp);
		MessageBox(hWnd, _T("已停止下载！"), _T("提示"), MB_OK);
		SetDlgItemText(hWnd, IDC_PROGRESS_RANGE, _T("0 %"));
		EnableWindow(hFileAddrWnd,TRUE);
		EnableWindow(hDownLoadBtnWnd,TRUE);
		EnableWindow(hPauseBtnWnd,FALSE);
		EnableWindow(hStopBtnWnd,FALSE);
		SetDlgItemText(hWnd,IDC_BTN_DOWNLOAD,_T("下载"));
		SendMessage(hProgressBarWnd, PBM_SETPOS, (WPARAM)0, (LPARAM)0); //将进度条复位
		SetDlgItemText(hWnd,IDC_FILE_SAVEPATH,_T(""));
		EnableWindow(hBrowserBtnWnd,TRUE);
		bStop = FALSE;
		return 0;
	}

	EnableWindow(hDownLoadBtnWnd , TRUE);
	SetDlgItemText(hWnd, IDC_BTN_DOWNLOAD, _T("下载"));

	EnableWindow(hPauseBtnWnd , FALSE);
	SetDlgItemText(hWnd, IDC_BTN_PAUSE, _T("暂停"));

	InternetCloseHandle(hopen);
	fclose(fp);

	MessageBox(hWnd, _T("下载成功"), _T("提示"), MB_OK);
	SendMessage(hProgressBarWnd, PBM_SETPOS, (WPARAM)range.iLow, (LPARAM)0); //将进度条复位
	SetDlgItemText(hWnd, IDC_PROGRESS_RANGE, _T("0 %"));
	EnableWindow(hFileAddrWnd,TRUE);
	EnableWindow(hStopBtnWnd,FALSE);

	return 0;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CWin32DownLoadDlg dialog




CWin32DownLoadDlg::CWin32DownLoadDlg(CWnd* pParent /*=NULL*/)
: CDialog(CWin32DownLoadDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWin32DownLoadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWin32DownLoadDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD, &CWin32DownLoadDlg::OnBtnDownload)
	ON_BN_CLICKED(IDC_BTN_BROWSER, &CWin32DownLoadDlg::OnBtnBrowser)
	ON_EN_CHANGE(IDC_FILE_ADDRESS, &CWin32DownLoadDlg::OnEnChangeFileAddress)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CWin32DownLoadDlg::OnBtnPause)
	ON_BN_CLICKED(IDC_BTN_STOP, &CWin32DownLoadDlg::OnBtnStop)
END_MESSAGE_MAP()


// CWin32DownLoadDlg message handlers

BOOL CWin32DownLoadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	hWnd = m_hWnd;
	hDownLoadBtnWnd = GetDlgItem(IDC_BTN_DOWNLOAD)->m_hWnd;
	hProgressBarWnd = GetDlgItem(IDC_PROGRESS)->m_hWnd;
	hPauseBtnWnd = GetDlgItem(IDC_BTN_PAUSE)->m_hWnd;
	hFileAddrWnd = GetDlgItem(IDC_FILE_ADDRESS)->m_hWnd;
	hProgressRange = GetDlgItem(IDC_PROGRESS_RANGE)->m_hWnd;
	hStopBtnWnd = GetDlgItem(IDC_BTN_STOP)->m_hWnd;
	hBrowserBtnWnd = GetDlgItem(IDC_BTN_BROWSER)->m_hWnd;
	SetWindowText(_T("下载"));

	SetDlgItemText(IDC_PROGRESS_RANGE,_T("0 %"));
	GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_PAUSE)->EnableWindow(FALSE);

	SetDlgItemText(IDC_FILE_ADDRESS, _T(""));

	CString fileAddr;
	GetDlgItemText(IDC_FILE_ADDRESS,fileAddr);
	if(fileAddr.IsEmpty())
	{
		GetDlgItem(IDC_BTN_BROWSER)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BTN_BROWSER)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWin32DownLoadDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWin32DownLoadDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWin32DownLoadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWin32DownLoadDlg::OnBtnDownload()
{
	// TODO: Add your control notification handler code here

	CString addr;
	GetDlgItemText(IDC_FILE_ADDRESS,addr);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PBThreadProc,NULL, 0, 0); //创建线程
	GetDlgItem(IDC_FILE_ADDRESS)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_BROWSER)->EnableWindow(FALSE);
//	GetDlgItem(IDC_BTN_PAUSE)->EnableWindow(TRUE);
}

void CWin32DownLoadDlg::OnBtnBrowser()
{
	// TODO: Add your control notification handler code here
	CString addr,ext;
	CString fileName,fileExt,filePath;
	GetDlgItemText(IDC_FILE_ADDRESS,addr);
	fileName = addr.Mid(addr.ReverseFind('/') + 1);
	fileExt = addr.Mid(addr.ReverseFind('.') + 1);

	CFileDialog saveDlg(FALSE,fileExt,fileName);
	//saveDlg.m_ofn.lpstrFilter = _T("可执行文件(*.exe)\0*.exe\0压缩文件(*.rar)\0*.rar");

	if(IDOK == saveDlg.DoModal())
	{
		filePath = saveDlg.GetPathName();
		SetDlgItemText(IDC_FILE_SAVEPATH,filePath);
	}
	addr.ReleaseBuffer();
}

void CWin32DownLoadDlg::OnEnChangeFileAddress()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString fileAddr;
	GetDlgItemText(IDC_FILE_ADDRESS,fileAddr);

	if(fileAddr.IsEmpty())
	{
		GetDlgItem(IDC_BTN_BROWSER)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BTN_BROWSER)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DOWNLOAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	}
}

void CWin32DownLoadDlg::OnBtnPause()
{
	// TODO: Add your control notification handler code here
	if (!bPaused) 
	{
		bPaused = TRUE;
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	}
	else
	{
		//bPaused = FALSE;
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PBThreadProc, NULL, 0, 0); //创建线程
	}
}

void CWin32DownLoadDlg::OnBtnStop()
{
	// TODO: Add your control notification handler code here
	bStop = TRUE;
}
