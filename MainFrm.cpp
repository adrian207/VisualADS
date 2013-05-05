// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "VisualADS.h"
#include "VisualADSDoc.h"
#include "MainFrm.h"
#include "VisualADSView.h"
#include "MyMessageBox.h"
#include "SocketManager.h"

#include "DoWithAPIHook.h"

#include<sstream>

#include <vector>
#include <fstream>
#include <string>
#include <Tlhelp32.h>

using namespace std;

#include "ADCNDPGenerator.h"
#include "ADDomain.h"
#include "ADComputer.h"
#include "ADDirectorySearch.h"
#include "ADWMISearch.h"
#include "ADGraphGenerator.h"
#include "ADDotReader.h"
#include "DLInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMER_PROGRESSBAR 0
#define TIMER_NOTOPMOST 1

#define ID_GUI_MESSAGE 0x2050

int aaa = 0;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CBCGPFrameWnd)


BEGIN_MESSAGE_MAP(CMainFrame, CBCGPFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_2000, ID_VIEW_APPLOOK_WIN7, OnAppLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_2000, ID_VIEW_APPLOOK_WIN7, OnUpdateAppLook)
	ON_COMMAND(ID_VIEW_WORKSPACE, OnViewWorkspace)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORKSPACE, OnUpdateViewWorkspace)
	ON_COMMAND(ID_VIEW_WORKSPACE2, OnViewWorkspace2)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORKSPACE2, OnUpdateViewWorkspace2)
	ON_COMMAND(ID_VIEW_OUTPUT, OnViewOutput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT, OnUpdateViewOutput)
	ON_COMMAND(ID_VIEW_PROPERTIES, OnViewPropGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIES, OnUpdateViewPropGrid)
	ON_REGISTERED_MESSAGE(BCGM_ON_RIBBON_CUSTOMIZE, OnRibbonCustomize)
	ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)

    ON_COMMAND(ID_BTN_LOGINFO_SEARCH,OnBtnLogInfoSearch)

	ON_COMMAND(ID_EDIT_DCNAME, OnEditDCName)
	ON_COMMAND(ID_EDIT_USERNAME, OnEditUsername)
	ON_COMMAND(ID_EDIT_PASSWORD, OnEditPassword)
	ON_COMMAND(ID_EDIT_CNDP_FILE_PATH_NAME, OnEditCNDPFilePathName)
	ON_COMMAND(ID_EDIT_RACER_IP, OnEditRacerIP)
	ON_COMMAND(ID_BTN_LOGON, OnBtnLogOn)
	ON_COMMAND(ID_BTN_CNDP_GEN, OnBtnCNDPGenerate)
	ON_COMMAND(ID_BTN_CNDPCD_GEN, OnBtnStartCNDPCD)
	ON_COMMAND(ID_BTN_CNDPCD_STOP, OnBtnStopCNDPCD)
	ON_COMMAND(ID_BTN_ADDDOMAIN, OnBtnAddDomain)
	ON_COMMAND(ID_BTN_REMOVEDOMAIN, OnBtnRemoveDomain)
	ON_COMMAND(ID_BTN_START_RACER, OnBtnStartRacer)
	ON_COMMAND(ID_BTN_DETECT, OnBtnDetect)
	ON_COMMAND(ID_BTN_CLEAR, OnBtnClear)

	ON_MESSAGE(WM_SOCKET, On_SocketEvent)

	ON_COMMAND(ID_CHECK_DESIGN, OnToggleDesignMode)
	ON_UPDATE_COMMAND_UI(ID_CHECK_DESIGN, OnUpdateToggleDesignMode)
	ON_COMMAND(ID_CHECK_CHINESE, OnToggleChinese)
	ON_UPDATE_COMMAND_UI(ID_CHECK_CHINESE, OnUpdateToggleChinese)
	ON_COMMAND(ID_CHECK_ENGLISH, OnToggleEnglish)
	ON_UPDATE_COMMAND_UI(ID_CHECK_ENGLISH, OnUpdateToggleEnglish)

	ON_MESSAGE(ID_GUI_MESSAGE + 0, On_MyMessageBox_Error)
	ON_MESSAGE(ID_GUI_MESSAGE + 1, On_Output_ConsoleList)
	ON_MESSAGE(ID_GUI_MESSAGE + 2, On_Output_RacerProCache)
	ON_MESSAGE(ID_GUI_MESSAGE + 3, On_Output_ReportList)
	ON_MESSAGE(ID_GUI_MESSAGE + 23, On_Property_AddKeyValue_Basic)
	ON_MESSAGE(ID_GUI_MESSAGE + 24, On_Property_AddKeyValue_Extend)
	ON_MESSAGE(ID_GUI_MESSAGE + 25, On_Property_AddKeyValue_Father)
	ON_MESSAGE(ID_GUI_MESSAGE + 26, On_Property_AddKeyValue_ChildGroup)
	ON_MESSAGE(ID_GUI_MESSAGE + 27, On_Property_AddKeyValue_ChildUser)
	ON_MESSAGE(ID_GUI_MESSAGE + 28, On_Property_AddKeyValue_Access)
	ON_MESSAGE(ID_GUI_MESSAGE + 29, On_Property_AddKeyValue_Network)
	ON_MESSAGE(ID_GUI_MESSAGE + 5, On_Property_ClearKeyValues)
	ON_MESSAGE(ID_GUI_MESSAGE + 6, On_Property_UpdateKeyValues)
	ON_MESSAGE(ID_GUI_MESSAGE + 7, On_ProgressBar_Start)
	ON_MESSAGE(ID_GUI_MESSAGE + 8, On_ProgressBar_Stop)
	ON_MESSAGE(ID_GUI_MESSAGE + 9, On_Canvas_Invalidate)
	ON_MESSAGE(ID_GUI_MESSAGE + 10, On_Output_Update_Connection)
END_MESSAGE_MAP()

HHOOK glhHook;
HWND g_hWndJava = NULL;
HWND g_hWndMainWindow = NULL;
int a = 0;
CMainFrame *pThis = NULL;

LRESULT CALLBACK MouseProc(
						   int nCode,      // hook code
						   WPARAM wParam,  // message identifier
						   LPARAM lParam   // mouse coordinates
						   )
{
	if(wParam==WM_LBUTTONDOWN)
    {
		//SetActiveWindow(g_hWndJava);
		if (g_hWndJava && g_hWndMainWindow)
		{
			//SetParent(g_hWndJava, g_hWndMainWindow);
		}

		//Output_ReportList(pThis->itos(a), pThis->itos(a ++));

		POINT pt = ((MSLLHOOKSTRUCT*)(lParam))-> pt;
		CRect rect;

		CMainFrame *pThis = (CMainFrame *) AfxGetApp()->m_pMainWnd;

		pThis->GetWindowRect(rect);
		pt.x -= rect.left;
		pt.y -= rect.top;

		pt.x -= 8;

		CBCGPBaseRibbonElement* selectedElement = pThis->m_wndRibbonBar.HitTest(CPoint(pt.x, pt.y));
		if (selectedElement)
		{
			if (selectedElement->GetParentCategory() == pThis->m_pCategory2)
			{
				CVisualADSApp *pApp = (CVisualADSApp *) AfxGetApp();
				
				pApp->showAboutDlg();
				//selectedElement->Redraw();
				//selectedElement->SetText("ddddw");
				POINT ptOrigin;
				GetCursorPos(&ptOrigin);
				//mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, (pt.x - 60) * 65536 / 1440, (pt.y) * 65536 / 900, 0, 0);
				SetCursorPos(pt.x + 100, pt.y - 10);
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				SetCursorPos(ptOrigin.x, ptOrigin.y);
				
				return 1;
			}
		}
    }
	return CallNextHookEx(glhHook, nCode, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_nAppLook = theApp.GetInt (_T("ApplicationLook"), ID_VIEW_APPLOOK_2010_2);

	pThis = this;

	m_hWndJava = NULL;
	iToken = 1;
	iLastToken = 1;
	iTry = 0;
	///*
	m_initDCName = _T("kira");
	m_initUsername = _T("administrator");
	m_initPassword = _T("123456Ad");
	m_initDCName2 = _T("lacus");
	m_initUsername2 = _T("administrator");
	m_initPassword2 = _T("123456Ad");
	//*/

	m_initRacerIP = _T("127.0.0.1");
	//m_initRacerIP = _T("");
	m_bSelfStartRacer = FALSE;
	g_bReboot = FALSE;

	m_strServerIP = _T("127.0.0.1");
	m_strPort = _T("53719");
	m_pCurServer = NULL;

	/*
	m_initDCName = _T("klbnt");
	m_initUsername = _T("administrator");
	m_initPassword = _T("buaanrdcklbnt");
	*/

	glhHook = SetWindowsHookEx(WH_MOUSE, MouseProc, 0, GetCurrentThreadId());

	m_iDomains = 1;
	m_pPanelLogOn = NULL;
	m_bDesignMode = FALSE;
	m_iProgress = 0;
	g_pMainFrame = this;

	initInputDir();
	initPolicyDir();
	initEXEDir();

	m_bChineseOrEnglish = Global_GetLanguage();

	doWithTheAPIHookStuff();
}

CMainFrame::~CMainFrame()
{
	stopServer();
	UnhookWindowsHookEx(glhHook);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBCGPFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (m_bChineseOrEnglish) 
	{
		if (!CreateRibbonBar_Chinese())
		{
			TRACE0("Failed to create ribbon bar\n");
			return -1;      // fail to create
		}
	}
	else
	{
		if (!CreateRibbonBar_English())
		{
			TRACE0("Failed to create ribbon bar\n");
			return -1;      // fail to create
		}
	}
	

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	initCNDPFilePathName();
	if (m_bChineseOrEnglish) 
	{
		initUI_Chinese();
	}
	else
	{
		initUI_English();
	}
	OnAppLook (m_nAppLook);
	return 0;
}

int CMainFrame::initUI_Chinese()
{
	// TODO: add your status bar panes here:
	m_wndStatusBar.AddElement (new CBCGPRibbonStatusBarPane (
		ID_STATUSBAR_PANE1, _T("���������缼���������ص�ʵ����"), TRUE), _T("���������缼���������ص�ʵ����"));

	///////////////////////������////////////////////////
	m_pProgress = new CBCGPRibbonProgressBar (1, 150);
	m_pProgress->SetInfiniteMode(FALSE);
	m_wndStatusBar.AddExtendedElement(m_pProgress, _T("����ָʾ��"), TRUE);
	m_pStatusLabel = new CBCGPRibbonStatusBarPane(ID_STATUSBAR_PANE2, _T("��������"), TRUE);
	m_pStatusLabel->SetText(_T("׼������"));
	m_wndStatusBar.AddExtendedElement(m_pStatusLabel, _T("��ǰ״̬"));
	///////////////////////������///////////////////////

	// Load control bar icons:
	CBCGPToolBarImages imagesWorkspace;
	imagesWorkspace.SetImageSize (CSize (16, 16));
	imagesWorkspace.SetTransparentColor (RGB (255, 0, 255));
	imagesWorkspace.Load (IDB_WORKSPACE);
	imagesWorkspace.SmoothResize(globalData.GetRibbonImageScale());

	/*
	if (!m_wndWorkSpace.Create (_T("View  1"), this, CRect (0, 0, 200, 200),
		TRUE, ID_VIEW_WORKSPACE,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Workspace bar\n");
		return -1;      // fail to create
	}

	m_wndWorkSpace.SetIcon (imagesWorkspace.ExtractIcon (0), FALSE);

	if (!m_wndWorkSpace2.Create (_T("View 2"), this, CRect (0, 0, 200, 200),
		TRUE, ID_VIEW_WORKSPACE2,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Workspace bar 2\n");
		return -1;      // fail to create
	}
	

	m_wndWorkSpace2.SetIcon (imagesWorkspace.ExtractIcon (1), FALSE);
	*/
	if (!m_wndOutput.Create (_T("���"), this, CRect (0, 0, 150, 150),
		TRUE /* Has gripper */, ID_VIEW_OUTPUT,
		WS_CHILD | WS_VISIBLE | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create output bar\n");
		return -1;      // fail to create
	}

	m_wndOutput.SetIcon (imagesWorkspace.ExtractIcon (2), FALSE);

	if (!m_wndPropGrid.Create (_T("����"), this, CRect (0, 0, 200, 200),
		TRUE, 
		ID_VIEW_PROPERTIES,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties Bar\n");
		return -1;		// fail to create
	}

	m_wndPropGrid.SetIcon (imagesWorkspace.ExtractIcon (3), FALSE);
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	//m_wndWorkSpace.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndWorkSpace2.EnableDocking(CBRS_ALIGN_ANY);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	m_wndPropGrid.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	EnableAutoHideBars(CBRS_ALIGN_ANY);
	//DockControlBar (&m_wndWorkSpace);
	//m_wndWorkSpace2.AttachToTabWnd (&m_wndWorkSpace, BCGP_DM_STANDARD, FALSE, NULL);
	DockControlBar(&m_wndPropGrid);
	DockControlBar(&m_wndOutput);

	return 0;
}

int CMainFrame::initUI_English()
{
	// TODO: add your status bar panes here:
	m_wndStatusBar.AddElement (new CBCGPRibbonStatusBarPane (
		ID_STATUSBAR_PANE1, _T("Beihang University - Key Laboratory of Beijing Network Technology"), TRUE), _T("Beihang University - Key Laboratory of Beijing Network Technology"));

	///////////////////////������////////////////////////
	m_pProgress = new CBCGPRibbonProgressBar (1, 150);
	m_pProgress->SetInfiniteMode(FALSE);
	m_wndStatusBar.AddExtendedElement(m_pProgress, _T("Process Indicator"), TRUE);
	m_pStatusLabel = new CBCGPRibbonStatusBarPane(ID_STATUSBAR_PANE2, _T("Running"), TRUE);
	m_pStatusLabel->SetText(_T("Get Ready"));
	m_wndStatusBar.AddExtendedElement(m_pStatusLabel, _T("Current State"));
	///////////////////////������///////////////////////

	// Load control bar icons:
	CBCGPToolBarImages imagesWorkspace;
	imagesWorkspace.SetImageSize (CSize (16, 16));
	imagesWorkspace.SetTransparentColor (RGB (255, 0, 255));
	imagesWorkspace.Load (IDB_WORKSPACE);
	imagesWorkspace.SmoothResize(globalData.GetRibbonImageScale());

	/*
	if (!m_wndWorkSpace.Create (_T("View  1"), this, CRect (0, 0, 200, 200),
		TRUE, ID_VIEW_WORKSPACE,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Workspace bar\n");
		return -1;      // fail to create
	}

	m_wndWorkSpace.SetIcon (imagesWorkspace.ExtractIcon (0), FALSE);

	if (!m_wndWorkSpace2.Create (_T("View 2"), this, CRect (0, 0, 200, 200),
		TRUE, ID_VIEW_WORKSPACE2,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Workspace bar 2\n");
		return -1;      // fail to create
	}
	

	m_wndWorkSpace2.SetIcon (imagesWorkspace.ExtractIcon (1), FALSE);
	*/
	if (!m_wndOutput.Create (_T("Output"), this, CRect (0, 0, 150, 150),
		TRUE /* Has gripper */, ID_VIEW_OUTPUT,
		WS_CHILD | WS_VISIBLE | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create output bar\n");
		return -1;      // fail to create
	}

	m_wndOutput.SetIcon (imagesWorkspace.ExtractIcon (2), FALSE);

	if (!m_wndPropGrid.Create (_T("Properties"), this, CRect (0, 0, 200, 200),
		TRUE, 
		ID_VIEW_PROPERTIES,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties Bar\n");
		return -1;		// fail to create
	}

	m_wndPropGrid.SetIcon (imagesWorkspace.ExtractIcon (3), FALSE);
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	//m_wndWorkSpace.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndWorkSpace2.EnableDocking(CBRS_ALIGN_ANY);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	m_wndPropGrid.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	EnableAutoHideBars(CBRS_ALIGN_ANY);
	//DockControlBar (&m_wndWorkSpace);
	//m_wndWorkSpace2.AttachToTabWnd (&m_wndWorkSpace, BCGP_DM_STANDARD, FALSE, NULL);
	DockControlBar(&m_wndPropGrid);
	DockControlBar(&m_wndOutput);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CBCGPFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	startServer();
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CBCGPFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CBCGPFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

CString CMainFrame::itos(int i)
{
	CString strTemp;
	strTemp.Format(_T("%d"), i);
	return strTemp;
}

BOOL CMainFrame::FolderExist(CString strPath)
{
	WIN32_FIND_DATA wfd;
	BOOL rValue = FALSE;
	HANDLE hFind = FindFirstFile(strPath, &wfd);
	if ((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		rValue = TRUE;
	}
	FindClose(hFind);
	return rValue;
}

void CMainFrame::initInputDir()
{
	CString cstrDir;
	TCHAR cBuffer[260];
	GetCurrentDirectory(MAX_PATH, cBuffer);
	CString cstrPath(cBuffer);
	if (FolderExist(cstrPath + _T("\\input")))
	{
		cstrDir = cstrPath;
	}
	else
	{
		int nCount = cstrPath.ReverseFind(_T('\\'));
		cstrPath.Delete(nCount, cstrPath.GetLength() - nCount);
		
		if (FolderExist(cstrPath + _T("\\input")))
		{
			cstrDir = cstrPath;
		}
		else
		{
			MyMessageBox_Error(_T("initInputDir Error"), _T("Error"));
			return;
		}
	}
	cstrDir += _T("\\input\\");
	m_strInputDir = cstrDir;
}

void CMainFrame::initPolicyDir()
{
	CString cstrDir;
	TCHAR cBuffer[260];
	GetCurrentDirectory(MAX_PATH, cBuffer);
	CString cstrPath(cBuffer);
	if (FolderExist(cstrPath + _T("\\policy")))
	{
		cstrDir = cstrPath;
	}
	else
	{
		int nCount = cstrPath.ReverseFind(_T('\\'));
		cstrPath.Delete(nCount, cstrPath.GetLength() - nCount);
		
		if (FolderExist(cstrPath + _T("\\policy")))
		{
			cstrDir = cstrPath;
		}
		else
		{
			MyMessageBox_Error(_T("initPolicyDir Error"), _T("Error"));
			return;
		}
	}
	cstrDir += _T("\\policy\\");
	m_strPolicyDir = cstrDir;
}

void CMainFrame::initEXEDir()
{
	CString cstrDir;
	TCHAR cBuffer[260];
	GetCurrentDirectory(MAX_PATH, cBuffer);
	CString cstrPath(cBuffer);
	if (FolderExist(cstrPath + _T("\\Debug")))
	{
		cstrDir = cstrPath + _T("\\Debug\\");
	}
	else
	{
		cstrDir = cstrPath;
	}
	m_strExePath = cstrDir;
}

void CMainFrame::initCNDPFilePathName()
{
	CString strCNDPFilePathName = m_strPolicyDir + _T("CNDP_FileService.txt");
	m_pEditFilePathName->SetEditText(strCNDPFilePathName);
}

BOOL CMainFrame::getLanguage()
{
	CStdioFile pFile;
	if (!pFile.Open(m_strInputDir + _T("\\Language.ini"), CFile::modeRead | CFile::typeText))
	{
		return TRUE;
	}
	
	CString strText;
	BOOL bChineseOrEnglish;
	pFile.ReadString(strText);
	if (strText == _T("1"))
	{
		bChineseOrEnglish = TRUE;
	}
	else
	{
		bChineseOrEnglish = FALSE;
	}
	
	pFile.Close();
	return bChineseOrEnglish;
}

void CMainFrame::setLanguage(BOOL bChineseOrEnglish)
{
	CStdioFile pFile;
	if (!pFile.Open(m_strInputDir + _T("\\Language.ini"), CFile::modeCreate | CFile::modeWrite))
	{
		MyMessageBox_Error(_T("setLanguage"));
		return;
	}
	
	if (bChineseOrEnglish)
	{
		pFile.WriteString(_T("1"));
	}
	else
	{
		pFile.WriteString(_T("0"));
	}
	
	pFile.Close();
}

BOOL CMainFrame::deleteFile(CString strFilePathName)
{
	SHFILEOPSTRUCT FileOp={0};
	FileOp.fFlags = FOF_ALLOWUNDO |   //����Żػ���վ
		FOF_NOCONFIRMATION; //������ȷ�϶Ի���
	FileOp.pFrom = strFilePathName;
	FileOp.pTo = NULL;      //һ��Ҫ��NULL
	FileOp.wFunc = FO_DELETE;    //ɾ������
	return SHFileOperation(&FileOp) == 0;
}

void CMainFrame::setCNDPCDVisible(BOOL bVisible)
{
	deleteFile(m_strPolicyDir + _T("needShow.txt"));
	CStdioFile pFile;
	TRY
	{
		pFile.Open(m_strPolicyDir + _T("needShow.txt"), CFile::modeCreate | CFile::modeWrite);
	}
	CATCH( CFileException, e )
	{
		int a = e->m_cause;
		return;
	}
	END_CATCH


// 	if (!pFile.Open(m_strPolicyDir + _T("needShow.txt"), CFile::modeCreate | CFile::modeWrite))
// 	{
// 		MyMessageBox_Error(_T("setCNDPCDVisible"));
// 		return;
// 	}
	
	if (bVisible)
	{
		pFile.WriteString(_T("needShow=1"));
	}
	else
	{
		pFile.WriteString(_T("needShow=0"));
	}
	pFile.Close();
}


void CMainFrame::OnAppLook(UINT id)
{
	CBCGPDockManager::SetDockMode (BCGP_DT_SMART);

	m_nAppLook = id;

	CBCGPTabbedControlBar::m_StyleTabWnd = CBCGPTabWnd::STYLE_3D;

	m_wndRibbonBar.SetScenicLook(FALSE);
	m_wndRibbonBar.EnableMinimizeButton(FALSE);

	switch (m_nAppLook)
	{
	case ID_VIEW_APPLOOK_2007:
	case ID_VIEW_APPLOOK_2007_1:
	case ID_VIEW_APPLOOK_2007_2:
	case ID_VIEW_APPLOOK_2007_3:
		// enable Office 2007 look:

		switch (m_nAppLook)
		{
		case ID_VIEW_APPLOOK_2007:
			CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_2007_1:
			CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_2007_2:
			CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_Silver);
			break;

		case ID_VIEW_APPLOOK_2007_3:
			CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_Aqua);
			break;
		}

		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManager2007));
		CBCGPDockManager::SetDockMode (BCGP_DT_SMART);
		break;

	case ID_VIEW_APPLOOK_2010_1:
	case ID_VIEW_APPLOOK_2010_2:
	case ID_VIEW_APPLOOK_2010_3:
		// enable Office 2010 look:
		switch (m_nAppLook)
		{
		case ID_VIEW_APPLOOK_2010_1:
			CBCGPVisualManager2010::SetStyle (CBCGPVisualManager2010::VS2010_Blue);
			break;

		case ID_VIEW_APPLOOK_2010_2:
			CBCGPVisualManager2010::SetStyle (CBCGPVisualManager2010::VS2010_Black);
			break;

		case ID_VIEW_APPLOOK_2010_3:
			CBCGPVisualManager2010::SetStyle (CBCGPVisualManager2010::VS2010_Silver);
			break;
		}

		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManager2010));

		m_wndRibbonBar.SetScenicLook();
		m_wndRibbonBar.EnableMinimizeButton();

		CBCGPDockManager::SetDockMode (BCGP_DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WIN7:
		// enable Windows 7 look:
		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManagerScenic));

		m_wndRibbonBar.SetScenicLook();

		CBCGPDockManager::SetDockMode (BCGP_DT_SMART);
		break;

	default:
		ASSERT (FALSE);
	}

	CBCGPDockManager* pDockManager = GetDockManager ();
	if (pDockManager != NULL)
	{
		ASSERT_VALID (pDockManager);
		pDockManager->AdjustBarFrames ();
	}

	CBCGPTabbedControlBar::ResetTabs ();

	RecalcLayout ();
	RedrawWindow (NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt (_T("ApplicationLook"), m_nAppLook);
}

void CMainFrame::OnUpdateAppLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio (m_nAppLook == pCmdUI->m_nID);
}

CString CMainFrame::i18n(CString strText)
{
	if (m_bChineseOrEnglish)
	{
		return strText;
	}
	else
	{
		if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else if (strText == _T(""))
		{
			return _T("");
		}
		else
		{
			return _T("i18n Error.");
		}
	}
}

BOOL CMainFrame::CreateRibbonBar_Chinese()
{
	if (!m_wndRibbonBar.Create(this))
	{
		return FALSE;
	}

	// Load panel images:
	m_PanelImages.SetImageSize (CSize (16, 16));
	m_PanelImages.Load (IDB_RIBBON_ICONS);

	// Init main button:
	m_MainButton.SetImage (IDB_RIBBON_MAIN);
	m_MainButton.SetToolTipText (_T("�ļ�"));
	m_MainButton.SetText (_T("\nf"));

	m_wndRibbonBar.SetMainButton (&m_MainButton, CSize (45, 45));

	CBCGPRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory (
		_T("�ļ�"), IDB_RIBBON_FILESMALL, IDB_RIBBON_FILELARGE);

	pMainPanel->Add (new CBCGPRibbonButton (ID_FILE_NEW, _T("�½�(&N)"), 0, 0));
	pMainPanel->Add (new CBCGPRibbonButton (ID_FILE_OPEN, _T("��(&O)"), 1, 1));

	pMainPanel->Add (new CBCGPRibbonButton (ID_FILE_SAVE, _T("����(&S)"), 2, 2));

	pMainPanel->Add (new CBCGPRibbonButton (ID_FILE_SAVE_AS, _T("���Ϊ(&A)"), 3, 3));

	CBCGPRibbonButton* pBtnPrint = new CBCGPRibbonButton (ID_FILE_PRINT, _T("��ӡ(&P)"), 4, 4);

	pBtnPrint->AddSubItem (new CBCGPRibbonLabel (_T("Ԥ������ӡ�ĵ�")));
	pBtnPrint->AddSubItem (new CBCGPRibbonButton (ID_FILE_PRINT, _T("��ӡ(&P)"), 4, 4, TRUE));
	pBtnPrint->AddSubItem (new CBCGPRibbonButton (ID_FILE_PRINT_DIRECT, _T("���ٴ�ӡ(&Q)"), 7, 7, TRUE));
	pBtnPrint->AddSubItem (new CBCGPRibbonButton (ID_FILE_PRINT_PREVIEW, _T("��ӡԤ��(&V)"), 6, 6, TRUE));

	pBtnPrint->SetKeys (_T("p"), _T("w"));

	pMainPanel->Add (pBtnPrint);

	pMainPanel->AddRecentFilesList (_T("���ʹ�õ���ͼ"), 300, TRUE /* Show Pins */);

	pMainPanel->AddToBottom (new CBCGPRibbonMainPanelButton (ID_TOOLS_OPTIONS, _T("ѡ��(&I)"), 9));
	pMainPanel->AddToBottom (new CBCGPRibbonMainPanelButton (ID_APP_EXIT, _T("�˳�(&X)"), 8));

	// Add "Home" category with "Clipboard" panel:
	CBCGPRibbonCategory* pCategory = m_wndRibbonBar.AddCategory (
		_T("&��������"),
		IDB_RIBBON_HOMESMALL,
		IDB_RIBBON_HOMELARGE);
	m_pCategory1 = pCategory;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Clipboard" panel:

	CBCGPRibbonPanel* pPanelClipboard = pCategory->AddPanel(_T("������"), m_PanelImages.ExtractIcon (1));
	CBCGPRibbonButton* pBtnPaste = new CBCGPRibbonButton (ID_EDIT_PASTE, _T("ճ��\nv"), 0, 0);
	pBtnPaste->SetMenu (IDR_PASTE_MENU, TRUE);
	pPanelClipboard->Add (pBtnPaste);
	pPanelClipboard->Add (new CBCGPRibbonButton (ID_EDIT_CUT, _T("����\nx"), 1));
	pPanelClipboard->Add (new CBCGPRibbonButton (ID_EDIT_COPY, _T("����\nc"), 2));
	pPanelClipboard->Add (new CBCGPRibbonButton (ID_EDIT_CLEAR, _T("ɾ��\nr"), 3));

	pPanelClipboard->SetKeys (_T("zc"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Window" panel:

	CBCGPRibbonPanel* pPanelWindow = pCategory->AddPanel(_T("�����"), m_PanelImages.ExtractIcon (2));
	//pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_WORKSPACE, _T("������1\n1")));
	//pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_WORKSPACE2, _T("������2\n2")));
	pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_OUTPUT, _T("�����\no")));
	pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_PROPERTIES, _T("������\ng")));
	pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_STATUS_BAR, _T("״̬��\ns")));
	pPanelWindow->SetKeys (_T("zw"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "LogOn" panel:
	m_pPanelLogOn = pCategory->AddPanel(_T("�����Ա��¼"));
	m_pEditDCName = new CBCGPRibbonEdit(ID_EDIT_DCNAME, 100, _T("������ƻ�IP:"));
	m_pEditDCName->SetEditText(m_initDCName);
	m_pEditUsername = new CBCGPRibbonEdit(ID_EDIT_USERNAME, 100, _T("�����Ա�˻�:"));
	m_pEditUsername->SetEditText(m_initUsername);
	m_pEditPassword = new CMyRibbonEdit(ID_EDIT_PASSWORD, 100, _T("�����Ա����:"));
	m_pEditPassword->SetEditText(m_initPassword);
	CBCGPRibbonButton* pBtnAddDomain = new CBCGPRibbonButton(ID_BTN_ADDDOMAIN, _T("��������"), 7 + 6, 7);
	CBCGPRibbonButton* pBtnRemoveDomain = new CBCGPRibbonButton(ID_BTN_REMOVEDOMAIN, _T("ɾ������"), 8 + 6, 8);
	CBCGPRibbonButton* pBtnLogOn = new CBCGPRibbonButton(ID_BTN_LOGON, _T("��¼\nl"), 3 + 6, 3);

	m_npEditDCNames.push_back(m_pEditDCName);
	m_npEditUsernames.push_back(m_pEditUsername);
	m_npEditPasswords.push_back(m_pEditPassword);

	m_pPanelLogOn->Add (m_pEditDCName);
	m_pPanelLogOn->Add (m_pEditUsername);
	m_pPanelLogOn->Add (m_pEditPassword);
	/*
	for (int i = 0; i < m_npEditDCNames.size(); i ++)
	{
		CBCGPRibbonEdit *pEditDCName = m_npEditDCNames[i];
		CBCGPRibbonEdit *pEditUsername = m_npEditUsernames[i];
		CBCGPRibbonEdit *pEditPassword = m_npEditPasswords[i];
		m_pPanelLogOn->Add(pEditPassword);
		m_pPanelLogOn->Add(pEditUsername);
		m_pPanelLogOn->Add(pEditDCName);
		m_pPanelLogOn->AddSeparator();
	}
	*/

	m_pPanelLogOn->Add (pBtnAddDomain);
	m_pPanelLogOn->Add (pBtnRemoveDomain);
	m_pPanelLogOn->Add (pBtnLogOn);
	m_pPanelLogOn->SetKeys (_T("zl"));

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "CNDPFileGenerate" panel:
	CBCGPRibbonPanel* pPanelCNDP = pCategory->AddPanel(_T("CNDP��������"), m_PanelImages.ExtractIcon (2));
	
	m_pEditFilePathName = new CBCGPRibbonEdit(ID_EDIT_CNDP_FILE_PATH_NAME, 100, _T("CNDP�ļ�·��:"));
	m_pEditFilePathName->SetEditText(m_initFilePathName);
	
	CBCGPRibbonButton* pBtnCNDPGen = new CBCGPRibbonButton(ID_BTN_CNDP_GEN, _T("����CNDP�ļ�"), 7 + 8, 7);
	
	pPanelCNDP->Add(m_pEditFilePathName);
	pPanelCNDP->Add(pBtnCNDPGen);
	pPanelCNDP->SetKeys (_T("zl"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "CNDP" panel:
	CBCGPRibbonPanel* pPanelCNDPCD = pCategory->AddPanel(_T("CNDPCD����"), m_PanelImages.ExtractIcon (2));
	
	CBCGPRibbonButton* pBtnCNDPCDGen = new CBCGPRibbonButton(ID_BTN_CNDPCD_GEN, _T("����CNDPCD����"), 7 + 9, 7);
	CBCGPRibbonButton* pBtnCNDPCDStop = new CBCGPRibbonButton(ID_BTN_CNDPCD_STOP, _T("�ر�CNDPCD����"), 7 + 10, 7);
	
	pPanelCNDPCD->Add(pBtnCNDPCDGen);
	pPanelCNDPCD->Add(pBtnCNDPCDStop);
	pPanelCNDPCD->SetKeys (_T("zl"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Detect" panel:
	CBCGPRibbonPanel* pPanelDetect = pCategory->AddPanel(_T("���Գ�ͻ���"));
	m_pEditRacerIP = new CBCGPRibbonEdit(ID_EDIT_RACER_IP, 100, _T("��������IP:"));
	m_pEditRacerIP->SetEditText(m_initRacerIP);
	CBCGPRibbonButton* pBtnStartRacer = new CBCGPRibbonButton(ID_BTN_START_RACER, _T("��������\ns"), 4 + 6, 4);
	CBCGPRibbonButton* pBtnDetect = new CBCGPRibbonButton(ID_BTN_DETECT, _T("��ͻ���\nd"), 5 + 6, 5);
	CBCGPRibbonButton* pBtnClear = new CBCGPRibbonButton(ID_BTN_CLEAR, _T("��ջ���\nc"), 6 + 6, 6);

	pPanelDetect->Add (m_pEditRacerIP);
	pPanelDetect->Add (pBtnStartRacer);
	pPanelDetect->Add (pBtnDetect);
	pPanelDetect->Add (pBtnClear);
	pPanelDetect->SetKeys (_T("zd"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Design" panel:
	m_pCheckBoxChinese = new CBCGPRibbonCheckBox (ID_CHECK_CHINESE, _T("�����û�����\nd"));
	//m_pCheckBoxEnglish = new CBCGPRibbonCheckBox (ID_CHECK_ENGLISH, _T("Ӣ���û�����\nd"));
	CBCGPRibbonPanel* pPanelDesign = pCategory->AddPanel(_T("���ģʽ"));
	pPanelDesign->Add (new CBCGPRibbonCheckBox (ID_CHECK_DESIGN, _T("�༭ͼ��Ԫ��\nd")));
	pPanelDesign->Add (m_pCheckBoxChinese);
	//pPanelDesign->Add (m_pCheckBoxEnglish);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	
	//Create Logoninfo Searchingbutton
	CBCGPRibbonPanel* pLogonInfoSearching = pCategory->AddPanel(_T("��Ϣ��ѯ"));
	CBCGPRibbonButton* pBtnInfoSearch = new CBCGPRibbonButton(ID_BTN_LOGINFO_SEARCH, _T("��½��Ϣ��ѯ\ns"), 7 + 11, 0);
	pLogonInfoSearching->Add (pBtnInfoSearch);
	pLogonInfoSearching->SetKeys (_T("zd"));

	// Add some hidden (non-ribbon) elements:
	CBCGPRibbonUndoButton* pUndo = new CBCGPRibbonUndoButton (ID_EDIT_UNDO, _T("Undo"), 4);

	pUndo->AddUndoAction (_T("Undo Item 1"));
	pUndo->AddUndoAction (_T("Undo Item 2"));
	pUndo->AddUndoAction (_T("Undo Item 3"));
	pUndo->AddUndoAction (_T("Undo Item 4"));
	pUndo->AddUndoAction (_T("Undo Item 5"));

	pCategory->AddHidden (pUndo);

	// Add "<TODO>" category with "Clipboard" panel:
	CBCGPRibbonCategory* pTODOCategory = m_wndRibbonBar.AddCategory (
		_T("&���ڱ���Ʒ"), 
		0 /* TODO: ID-category small images */, 
		0 /* TODO: ID-category large images */);
	m_pCategory2 = pTODOCategory;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Help" panel:
	CBCGPRibbonPanel* pPanelHelp = pTODOCategory->AddPanel(_T("���������"), m_PanelImages.ExtractIcon (1));
	pPanelHelp->Add(new CBCGPRibbonLabel (_T("VisualADS v1.0.0��C++��BCG���Graphviz������ɡ�")));
	pPanelHelp->Add(new CBCGPRibbonLabel (_T("ʹ�ù������������360����HOSTS�޸ĵ���ʾ��������")));
	pPanelHelp->Add(new CBCGPRibbonLabel (_T("����֧������ϵ��veotax@sae.buaa.edu.cn")));

	// Add quick access toolbar commands:
	CBCGPRibbonQATDefaultState qatState;

	qatState.AddCommand (ID_FILE_NEW, FALSE);
	qatState.AddCommand (ID_FILE_OPEN, FALSE);
	qatState.AddCommand (ID_FILE_SAVE);
	qatState.AddCommand (ID_FILE_PRINT_DIRECT);
	qatState.AddCommand (ID_FILE_PRINT_PREVIEW, FALSE);
	qatState.AddCommand (ID_EDIT_UNDO);

	m_wndRibbonBar.SetQuickAccessDefaultState (qatState);

	// Add "Style" button to the right of tabs:
	CBCGPRibbonButton* pStyleButton = new CBCGPRibbonButton (-1, _T("����\ns"), -1, -1);
	pStyleButton->SetMenu (IDR_THEME_MENU, TRUE /* Right align */);

	m_wndRibbonBar.AddToTabs (pStyleButton);

	// Add "About" button to the right of tabs:
	m_wndRibbonBar.AddToTabs (new CBCGPRibbonButton (ID_APP_ABOUT, _T("\na"), m_PanelImages.ExtractIcon (0)));

	return TRUE;
}

BOOL CMainFrame::CreateRibbonBar_English()
{
	if (!m_wndRibbonBar.Create(this))
	{
		return FALSE;
	}

	// Load panel images:
	m_PanelImages.SetImageSize (CSize (16, 16));
	m_PanelImages.Load (IDB_RIBBON_ICONS);

	// Init main button:
	m_MainButton.SetImage (IDB_RIBBON_MAIN);
	m_MainButton.SetToolTipText (_T("File"));
	m_MainButton.SetText (_T("\nf"));

	m_wndRibbonBar.SetMainButton (&m_MainButton, CSize (45, 45));

	CBCGPRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory (
		_T("File"), IDB_RIBBON_FILESMALL, IDB_RIBBON_FILELARGE);

	pMainPanel->Add (new CBCGPRibbonButton (ID_FILE_NEW, _T("New(&N)"), 0, 0));
	pMainPanel->Add (new CBCGPRibbonButton (ID_FILE_OPEN, _T("Open(&O)"), 1, 1));

	pMainPanel->Add (new CBCGPRibbonButton (ID_FILE_SAVE, _T("Save(&S)"), 2, 2));

	pMainPanel->Add (new CBCGPRibbonButton (ID_FILE_SAVE_AS, _T("Save As(&A)"), 3, 3));

	CBCGPRibbonButton* pBtnPrint = new CBCGPRibbonButton (ID_FILE_PRINT, _T("Print(&P)"), 4, 4);

	pBtnPrint->AddSubItem (new CBCGPRibbonLabel (_T("Preview and Print")));
	pBtnPrint->AddSubItem (new CBCGPRibbonButton (ID_FILE_PRINT, _T("Print(&P)"), 4, 4, TRUE));
	pBtnPrint->AddSubItem (new CBCGPRibbonButton (ID_FILE_PRINT_DIRECT, _T("Quick Print(&Q)"), 7, 7, TRUE));
	pBtnPrint->AddSubItem (new CBCGPRibbonButton (ID_FILE_PRINT_PREVIEW, _T("Preview(&V)"), 6, 6, TRUE));

	pBtnPrint->SetKeys (_T("p"), _T("w"));

	pMainPanel->Add (pBtnPrint);

	pMainPanel->AddRecentFilesList (_T("Last Views"), 300, TRUE /* Show Pins */);

	pMainPanel->AddToBottom (new CBCGPRibbonMainPanelButton (ID_TOOLS_OPTIONS, _T("Options(&I)"), 9));
	pMainPanel->AddToBottom (new CBCGPRibbonMainPanelButton (ID_APP_EXIT, _T("Exit(&X)"), 8));

	// Add "Home" category with "Clipboard" panel:
	CBCGPRibbonCategory* pCategory = m_wndRibbonBar.AddCategory (
		_T("&Basic"),
		IDB_RIBBON_HOMESMALL,
		IDB_RIBBON_HOMELARGE);
	m_pCategory1 = pCategory;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Clipboard" panel:

	CBCGPRibbonPanel* pPanelClipboard = pCategory->AddPanel(_T("Clipboard"), m_PanelImages.ExtractIcon (1));
	CBCGPRibbonButton* pBtnPaste = new CBCGPRibbonButton (ID_EDIT_PASTE, _T("Paste\nv"), 0, 0);
	pBtnPaste->SetMenu (IDR_PASTE_MENU, TRUE);
	pPanelClipboard->Add (pBtnPaste);
	pPanelClipboard->Add (new CBCGPRibbonButton (ID_EDIT_CUT, _T("Cut\nx"), 1));
	pPanelClipboard->Add (new CBCGPRibbonButton (ID_EDIT_COPY, _T("Copy\nc"), 2));
	pPanelClipboard->Add (new CBCGPRibbonButton (ID_EDIT_CLEAR, _T("Clear\nr"), 3));

	pPanelClipboard->SetKeys (_T("zc"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Window" panel:

	CBCGPRibbonPanel* pPanelWindow = pCategory->AddPanel(_T("Sidebars"), m_PanelImages.ExtractIcon (2));
	//pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_WORKSPACE, _T("������1\n1")));
	//pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_WORKSPACE2, _T("������2\n2")));
	pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_OUTPUT, _T("Output\no")));
	pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_PROPERTIES, _T("Properties\ng")));
	pPanelWindow->Add (new CBCGPRibbonCheckBox (ID_VIEW_STATUS_BAR, _T("Status\ns")));
	pPanelWindow->SetKeys (_T("zw"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "LogOn" panel:
	m_pPanelLogOn = pCategory->AddPanel(_T("Domain-Admin Logon"));
	m_pEditDCName = new CBCGPRibbonEdit(ID_EDIT_DCNAME, 100, _T("DC Host or IP :"));
	m_pEditDCName->SetEditText(m_initDCName);
	m_pEditUsername = new CBCGPRibbonEdit(ID_EDIT_USERNAME, 100, _T("Admin Username:"));
	m_pEditUsername->SetEditText(m_initUsername);
	m_pEditPassword = new CMyRibbonEdit(ID_EDIT_PASSWORD, 100, _T("Admin Password:"));
	m_pEditPassword->SetEditText(m_initPassword);
	CBCGPRibbonButton* pBtnAddDomain = new CBCGPRibbonButton(ID_BTN_ADDDOMAIN, _T("Add Domain"), 7 + 6, 7);
	CBCGPRibbonButton* pBtnRemoveDomain = new CBCGPRibbonButton(ID_BTN_REMOVEDOMAIN, _T("Remove Domain"), 8 + 6, 8);
	CBCGPRibbonButton* pBtnLogOn = new CBCGPRibbonButton(ID_BTN_LOGON, _T("Log on\nl"), 3 + 6, 3);

	m_npEditDCNames.push_back(m_pEditDCName);
	m_npEditUsernames.push_back(m_pEditUsername);
	m_npEditPasswords.push_back(m_pEditPassword);

	m_pPanelLogOn->Add (m_pEditDCName);
	m_pPanelLogOn->Add (m_pEditUsername);
	m_pPanelLogOn->Add (m_pEditPassword);
	/*
	for (int i = 0; i < m_npEditDCNames.size(); i ++)
	{
		CBCGPRibbonEdit *pEditDCName = m_npEditDCNames[i];
		CBCGPRibbonEdit *pEditUsername = m_npEditUsernames[i];
		CBCGPRibbonEdit *pEditPassword = m_npEditPasswords[i];
		m_pPanelLogOn->Add(pEditPassword);
		m_pPanelLogOn->Add(pEditUsername);
		m_pPanelLogOn->Add(pEditDCName);
		m_pPanelLogOn->AddSeparator();
	}
	*/

	m_pPanelLogOn->Add (pBtnAddDomain);
	m_pPanelLogOn->Add (pBtnRemoveDomain);
	m_pPanelLogOn->Add (pBtnLogOn);
	m_pPanelLogOn->SetKeys (_T("zl"));

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "CNDPFileGenerate" panel:
	CBCGPRibbonPanel* pPanelCNDP = pCategory->AddPanel(_T("CNDP Generation"), m_PanelImages.ExtractIcon (2));
	
	m_pEditFilePathName = new CBCGPRibbonEdit(ID_EDIT_CNDP_FILE_PATH_NAME, 100, _T("CNDP File Path:"));
	m_pEditFilePathName->SetEditText(m_initFilePathName);
	
	CBCGPRibbonButton* pBtnCNDPGen = new CBCGPRibbonButton(ID_BTN_CNDP_GEN, _T("Generate"), 9 + 6, 9);
	
	pPanelCNDP->Add(m_pEditFilePathName);
	pPanelCNDP->Add(pBtnCNDPGen);
	pPanelCNDP->SetKeys (_T("zl"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "CNDP" panel:
	CBCGPRibbonPanel* pPanelCNDPCD = pCategory->AddPanel(_T("CNDPCD Tool"), m_PanelImages.ExtractIcon (2));
	
	CBCGPRibbonButton* pBtnCNDPCDGen = new CBCGPRibbonButton(ID_BTN_CNDPCD_GEN, _T("Start"), 10 + 6, 10);
	CBCGPRibbonButton* pBtnCNDPCDStop = new CBCGPRibbonButton(ID_BTN_CNDPCD_STOP, _T("Stop"), 11 + 6, 11);
	
	pPanelCNDPCD->Add(pBtnCNDPCDGen);
	pPanelCNDPCD->Add(pBtnCNDPCDStop);
	pPanelCNDPCD->SetKeys (_T("zl"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Detect" panel:
	CBCGPRibbonPanel* pPanelDetect = pCategory->AddPanel(_T("Conflict Detection"));
	m_pEditRacerIP = new CBCGPRibbonEdit(ID_EDIT_RACER_IP, 100, _T("RacerPro IP:"));
	m_pEditRacerIP->SetEditText(m_initRacerIP);
	CBCGPRibbonButton* pBtnStartRacer = new CBCGPRibbonButton(ID_BTN_START_RACER, _T("Start RacerPro\ns"), 4 + 6, 4);
	CBCGPRibbonButton* pBtnDetect = new CBCGPRibbonButton(ID_BTN_DETECT, _T("Detect\nd"), 5 + 6, 5);
	CBCGPRibbonButton* pBtnClear = new CBCGPRibbonButton(ID_BTN_CLEAR, _T("Clear Cache\nc"), 6 + 6, 6);

	pPanelDetect->Add (m_pEditRacerIP);
	pPanelDetect->Add (pBtnStartRacer);
	pPanelDetect->Add (pBtnDetect);
	pPanelDetect->Add (pBtnClear);
	pPanelDetect->SetKeys (_T("zd"));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Design" panel:
	m_pCheckBoxChinese = new CBCGPRibbonCheckBox (ID_CHECK_CHINESE, _T("Chinese UI\nd"));
	//m_pCheckBoxEnglish = new CBCGPRibbonCheckBox (ID_CHECK_ENGLISH, _T("Ӣ���û�����\nd"));
	CBCGPRibbonPanel* pPanelDesign = pCategory->AddPanel(_T("Design Mode"));
	pPanelDesign->Add (new CBCGPRibbonCheckBox (ID_CHECK_DESIGN, _T("Edit Figure\nd")));
	pPanelDesign->Add (m_pCheckBoxChinese);
	//pPanelDesign->Add (m_pCheckBoxEnglish);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	
	//Create Logoninfo Searchingbutton
	CBCGPRibbonPanel* pLogonInfoSearching = pCategory->AddPanel(_T("Query Information"));
	CBCGPRibbonButton* pBtnInfoSearch = new CBCGPRibbonButton(ID_BTN_LOGINFO_SEARCH, _T("Logon Infomation Query\ns"), 12 + 6, 12);
	pLogonInfoSearching->Add (pBtnInfoSearch);
	pLogonInfoSearching->SetKeys (_T("zd"));

	// Add some hidden (non-ribbon) elements:
	CBCGPRibbonUndoButton* pUndo = new CBCGPRibbonUndoButton (ID_EDIT_UNDO, _T("Undo"), 4);

	pUndo->AddUndoAction (_T("Undo Item 1"));
	pUndo->AddUndoAction (_T("Undo Item 2"));
	pUndo->AddUndoAction (_T("Undo Item 3"));
	pUndo->AddUndoAction (_T("Undo Item 4"));
	pUndo->AddUndoAction (_T("Undo Item 5"));

	pCategory->AddHidden (pUndo);

	// Add "<TODO>" category with "Clipboard" panel:
	CBCGPRibbonCategory* pTODOCategory = m_wndRibbonBar.AddCategory (
		_T("&About"), 
		0 /* TODO: ID-category small images */, 
		0 /* TODO: ID-category large images */);
	m_pCategory2 = pTODOCategory;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create "Help" panel:
	CBCGPRibbonPanel* pPanelHelp = pTODOCategory->AddPanel(_T("About and Help"), m_PanelImages.ExtractIcon (1));
	pPanelHelp->Add(new CBCGPRibbonLabel (_T("VisualADS v1.0.0 is developed by C++, BCGControlBar and Graphviz.")));
	pPanelHelp->Add(new CBCGPRibbonLabel (_T("ʹ�ù������������360����HOSTS�޸ĵ���ʾ��������")));
	pPanelHelp->Add(new CBCGPRibbonLabel (_T("����֧������ϵ��veotax@sae.buaa.edu.cn")));

	// Add quick access toolbar commands:
	CBCGPRibbonQATDefaultState qatState;

	qatState.AddCommand (ID_FILE_NEW, FALSE);
	qatState.AddCommand (ID_FILE_OPEN, FALSE);
	qatState.AddCommand (ID_FILE_SAVE);
	qatState.AddCommand (ID_FILE_PRINT_DIRECT);
	qatState.AddCommand (ID_FILE_PRINT_PREVIEW, FALSE);
	qatState.AddCommand (ID_EDIT_UNDO);

	m_wndRibbonBar.SetQuickAccessDefaultState (qatState);

	// Add "Style" button to the right of tabs:
	CBCGPRibbonButton* pStyleButton = new CBCGPRibbonButton (-1, _T("Change Skin\ns"), -1, -1);
	pStyleButton->SetMenu (IDR_THEME_MENU, TRUE /* Right align */);

	m_wndRibbonBar.AddToTabs (pStyleButton);

	// Add "About" button to the right of tabs:
	m_wndRibbonBar.AddToTabs (new CBCGPRibbonButton (ID_APP_ABOUT, _T("\na"), m_PanelImages.ExtractIcon (0)));

	return TRUE;
}

LRESULT CMainFrame::OnRibbonCustomize (WPARAM wp, LPARAM lp)
{
	ShowOptions (0);
	return 1;
}

void CMainFrame::OnToolsOptions()
{
	ShowOptions (0);
}

void CMainFrame::ShowOptions (int nPage)
{
	// Create "Customize" page:
	CBCGPRibbonCustomizePage pageCustomize (&m_wndRibbonBar);

	// Add "popular" items:
	CList<UINT, UINT> lstPopular;

	lstPopular.AddTail (ID_FILE_NEW);
	lstPopular.AddTail (ID_FILE_OPEN);
	lstPopular.AddTail (ID_FILE_SAVE);
	lstPopular.AddTail (ID_FILE_PRINT_PREVIEW);
	lstPopular.AddTail (ID_FILE_PRINT_DIRECT);
	lstPopular.AddTail (ID_EDIT_UNDO);

	pageCustomize.AddCustomCategory (_T("Popular Commands"), lstPopular);

	// Add hidden commands:
	CList<UINT,UINT> lstHidden;
	m_wndRibbonBar.GetItemIDsList (lstHidden, TRUE);

	pageCustomize.AddCustomCategory (_T("Commands not in the Ribbon"), lstHidden);

	// Add all commands:
	CList<UINT,UINT> lstAll;
	m_wndRibbonBar.GetItemIDsList (lstAll);

	pageCustomize.AddCustomCategory (_T("All Commands"), lstAll);

	// Create property sheet:
	CBCGPPropertySheet propSheet (_T("Options"), this, nPage);
	propSheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;

	propSheet.SetLook (CBCGPPropertySheet::PropSheetLook_List, 124 /* List width */);

	propSheet.AddPage (&pageCustomize);

	// TODO: add your option pages here:
	// COptionsPage1 pageOptions1;
	// propSheet.AddPage (&pageOptions1);
	//
	// COptionsPage1 pageOptions2;
	// propSheet.AddPage (&pageOptions2);

	if (propSheet.DoModal() != IDOK)
	{
		return;
	}
}

void CMainFrame::OnViewWorkspace() 
{
	ShowControlBar (&m_wndWorkSpace,
					!(m_wndWorkSpace.IsVisible ()),
					FALSE, TRUE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewWorkspace(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (m_wndWorkSpace.IsVisible ());
	pCmdUI->Enable (!GetDockManager ()->IsPrintPreviewValid ());
}

void CMainFrame::OnViewWorkspace2() 
{
	ShowControlBar (&m_wndWorkSpace2,
					!(m_wndWorkSpace2.IsVisible ()),
					FALSE, TRUE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewWorkspace2(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (m_wndWorkSpace2.IsVisible ());
	pCmdUI->Enable (!GetDockManager ()->IsPrintPreviewValid ());
}

void CMainFrame::OnViewOutput() 
{
	ShowControlBar (&m_wndOutput,
					!(m_wndOutput.IsVisible ()),
					FALSE, TRUE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewOutput(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (m_wndOutput.IsVisible ());
	pCmdUI->Enable (!GetDockManager ()->IsPrintPreviewValid ());
}

void CMainFrame::OnViewPropGrid() 
{
	ShowControlBar (&m_wndPropGrid,
					!(m_wndPropGrid.IsVisible ()),
					FALSE, TRUE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewPropGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (m_wndPropGrid.IsVisible ());
	pCmdUI->Enable (!GetDockManager ()->IsPrintPreviewValid ());
}

void CMainFrame::OnEditPassword()
{
	//m_pEditPassword->SetEditText("***");
}

void CMainFrame::OnBtnLogOn()
{
	HANDLE hLogOnThread = CreateThread(NULL, 0, LogOnThread, this, 0, NULL);
	if (hLogOnThread == NULL)
	{
		MyMessageBox_Error(_T("OnBtnLogOn"));
	}
	else
	{
		CloseHandle(hLogOnThread);
	}
}

void CMainFrame::OnBtnCNDPGenerate()
{
	CString strCNDPFilePathName = m_pEditFilePathName->GetEditText();
	
	ADCNDPGenerator *generator = new ADCNDPGenerator(globalDomains, strCNDPFilePathName);
	generator->execute();
}

// DWORD CMainFrame::FindProcessByName(CString& strFileName)
// {
// 	HANDLE   hProcSnap = NULL; // ���̿��վ��
// 	PROCESSENTRY32 pe32  = {0}; // ϵͳ��Ϣ
// 	// ����ϵͳ����
// 	hProcSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
// 	if( hProcSnap == INVALID_HANDLE_VALUE )
// 		return FALSE;
// 	// ��ʹ��PROCESSENTRY32�ṹǰ�������ߴ�
// 	pe32.dwSize = sizeof(PROCESSENTRY32);
// 	// �������̿���, ����ָ���ļ���
// 	if( Process32First( hProcSnap, &pe32 ) )
// 	{
// 		do
// 		{
// 			// �ȽϽ������ֺʹ��������ļ���
// 			CString strExe( pe32.szExeFile );
// 			//LogFormatedMsg( "C:\\list.txt", "%s", strExe ); // debug code
// 			DWORD dwProcessID = pe32.th32ProcessID; // �����б�Ľ���ID
// 			// ����ͬ���ֵĽ���, ���Ҳ�ͬ�ڵ�ǰ����ID, ����ǰһ�������е�ʵ��
// 			if( strExe.CompareNoCase( strFileName ) == 0
// 				&& dwProcessID != GetCurrentProcessId() )
// 				return dwProcessID;
// 		}
// 		while( Process32Next( hProcSnap, &pe32 ) );
// 	}
// 	// �˳�ǰҪ������ն���
// 	CloseHandle( hProcSnap );
// 	return FALSE;
// }

// BOOL CALLBACK YourEnumProc(HWND hWnd, LPARAM lParam) 
// { 
// 	DWORD dwProcessId;
// 	char buf[1024];
// 	GetWindowThreadProcessId(hWnd, &dwProcessId);
// 	GetWindowText(hWnd, buf, 1024);
// 	LPWNDINFO pInfo = (LPWNDINFO)lParam;
// 	if (strcmp(g_strTitle, buf) == 0) //dwProcessId == pInfo->dwProcessId && 
// 	{
// 		pInfo-> hWnd = hWnd;
// 		printf("%d\n",pInfo->hWnd);
// 		return FALSE;
// 	}
// 	printf("%d\n",pInfo->hWnd);
// 	return TRUE;
// }

// HWND CMainFrame::GetProcessMainWnd(DWORD dwProcessId) 
// { 
// 	WNDINFO wi;
// 	wi.dwProcessId = dwProcessId; 
// 	wi.hWnd = NULL; 
// 	EnumWindows(YourEnumProc,(LPARAM)&wi); 
// 	return wi.hWnd; 
// }

//	HWND hJWnd = (HWND)FindWindow(NULL,"���������������Գ�ͻ��⹤��");
// 	CString ss = "javaw.exe";
// 	DWORD pid = FindProcessByName(ss);

CString g_strTitle;
int g_flag = 0;

typedef struct tagWNDINFO 
{ 
	DWORD dwProcessId; 
	HWND hWnd; 
} WNDINFO, *LPWNDINFO; 

BOOL CALLBACK YourEnumProc(HWND hWnd, LPARAM lParam) 
{ 
	char buf[1024];
	GetWindowText(hWnd, buf, 1024);
	LPWNDINFO pInfo = (LPWNDINFO)lParam;
	if (strcmp(g_strTitle, buf) == 0) //dwProcessId == pInfo->dwProcessId && 
	{
		pInfo->hWnd = hWnd;
		//printf("%d\n",pInfo->hWnd);
		return FALSE;
	}
	//printf("%d\n",pInfo->hWnd);
	return TRUE;
}

HWND CMainFrame::GetGlobalHandleByTitle(CString strTitle) 
{
	g_strTitle = strTitle;

	WNDINFO wi;
	wi.hWnd = NULL; 
	EnumWindows(YourEnumProc, (LPARAM)&wi); 
    //ȷ������������
	g_flag = 1;
	return wi.hWnd; 
}

void CMainFrame::executeWithHide(CString strPathName)
{
	TCHAR szDesktop[256] = _T("MyDesktop");
    HDESK hDesk = CreateDesktop(szDesktop, NULL, NULL, 0, GENERIC_ALL|DESKTOP_CREATEWINDOW, NULL);
    if (hDesk)
    {        
        STARTUPINFO si = {sizeof(si)};
        //si.dwFlags = STARTF_USESHOWWINDOW;
        //si.wShowWindow = SW_HIDE;
		memset(&si, 0, sizeof(STARTUPINFO));
        si.lpDesktop = szDesktop;
		si.cb = sizeof(STARTUPINFO);
        PROCESS_INFORMATION pi = {0};
        TCHAR szApp[255];
		//strPathName = "java.exe -jar " + strPathName;
		strPathName = "E:\\���\\crack tools\\����2010�������\\Data\\System Defense\\�ļ�����\\hashcalc\\HashCalc.exe";
		_tcscpy(szApp, strPathName);
        if (CreateProcess(NULL, szApp, NULL, NULL, FALSE, 0, NULL,NULL, &si, &pi))
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        else
        {
			DWORD aaa = GetLastError();
            AfxMessageBox(_T("Failed."));
        }
        CloseDesktop(hDesk);
    }
}

void CMainFrame::OnBtnStartCNDPCD()
{
	HANDLE hCNDPCDThread = CreateThread(NULL, 0, CNDPCDThread, this, 0, NULL);
	if (hCNDPCDThread == NULL)
	{
		MyMessageBox_Error(_T("OnBtnStartCNDPCD"));
	}
	else
	{
		CloseHandle(hCNDPCDThread);
	}
}

void CMainFrame::OnBtnStopCNDPCD()
{
	::SendMessage(m_hWndJava, WM_CLOSE, NULL, NULL);
}

void CMainFrame::OnBtnLoad()
{
	HANDLE hLoadThread = CreateThread(NULL, 0, LoadThread, this, 0, NULL);
	if (hLoadThread == NULL)
	{
		MyMessageBox_Error(_T("OnBtnLoad"));
	}
	else
	{
		CloseHandle(hLoadThread);
	}
}

void CMainFrame::OnBtnAddDomain()
{
	m_iDomains ++;
	CBCGPRibbonEdit *pEditDCName;
	CBCGPRibbonEdit *pEditUsername;
	CBCGPRibbonEdit *pEditPassword;

	if (m_bChineseOrEnglish)
	{
		pEditDCName = new CBCGPRibbonEdit(ID_EDIT_DCNAME, 100, _T("������ƻ�IP:"));
		pEditUsername = new CBCGPRibbonEdit(ID_EDIT_USERNAME, 100, _T("�����Ա�˻�:"));
		pEditPassword = new CMyRibbonEdit(ID_EDIT_PASSWORD, 100, _T("�����Ա����:"));
	}
	else
	{
		pEditDCName = new CBCGPRibbonEdit(ID_EDIT_DCNAME, 100, _T("DC Name or IP:"));
		pEditUsername = new CBCGPRibbonEdit(ID_EDIT_USERNAME, 100, _T("Admin Name:"));
		pEditPassword = new CMyRibbonEdit(ID_EDIT_PASSWORD, 100, _T("Admin Pass:"));
	}

	if (m_iDomains == 2)
	{
		pEditDCName->SetEditText(m_initDCName2);
		pEditUsername->SetEditText(m_initUsername2);
		pEditPassword->SetEditText(m_initPassword2);
	}

	m_npEditDCNames.push_back(pEditDCName);
	m_npEditUsernames.push_back(pEditUsername);
	m_npEditPasswords.push_back(pEditPassword);

	int iCount = m_pPanelLogOn->GetCount();
	m_pPanelLogOn->Insert(pEditPassword, iCount - 3);
	m_pPanelLogOn->Insert(pEditUsername, iCount - 3);
	m_pPanelLogOn->Insert(pEditDCName, iCount - 3);
	m_pPanelLogOn->InsertSeparator(iCount - 3);

	m_pPanelLogOn->SetJustifyColumns();
	m_wndRibbonBar.ForceRecalcLayout();
	//m_wndRibbonBar.AdjustSizeImmediate();

	//ShowControlBar(&m_wndRibbonBar, FALSE, FALSE, TRUE);
	//ShowControlBar(&m_wndRibbonBar, TRUE, FALSE, TRUE);
	//RecalcLayout();
}

void CMainFrame::OnBtnRemoveDomain()
{
	if (m_iDomains <= 1)
	{
		MyMessageBox_Error(_T("�޷�ɾ�����һ����"), _T("������ʾ��Ϣ"));
		return;
	}
	m_iDomains --;
	
	for (int i = 0; i < 4; i ++)
	{
		int iCount = m_pPanelLogOn->GetCount();
		if (iCount - 4 < 0)
		{
			MyMessageBox_Error(_T("OnBtnRemoveDomain"));
			return;
		}
		m_pPanelLogOn->Remove(iCount - 4);
	}

	m_npEditDCNames.pop_back();
	m_npEditUsernames.pop_back();
	m_npEditPasswords.pop_back();

	m_pPanelLogOn->SetJustifyColumns();
	m_wndRibbonBar.ForceRecalcLayout();
	//m_wndRibbonBar.AdjustSizeImmediate();
}

void CMainFrame::OnBtnStartRacer()
{
	
	CString strRacerIP = m_pEditRacerIP->GetEditText();

	if (strRacerIP != _T(""))
	{
		if (!ADDirectorySearch::checkIP(strRacerIP))
		{
			MyMessageBox_Error(_T("�������IP��ַ��ʽ����ȷ��"), _T("������ʾ��Ϣ"));
			return;
		}
	}

	if (strRacerIP != _T("") && strRacerIP != _T("127.0.0.1"))
	{
		if (DLInterface::checkConnection(strRacerIP))
		{
			CString strText = _T("��⵽���������Ѿ���Զ�̼����") + strRacerIP + _T("������");
			MyMessageBox_Error(strText, _T("������ʾ��Ϣ"));
		}
		else
		{
			CString strText = _T("��������δ��Զ�̼����") + strRacerIP + _T("������");
			MyMessageBox_Error(strText, _T("������ʾ��Ϣ"));
		}
	}
	else
	{
		DLInterface *dlInterface = new DLInterface();
		delete dlInterface;
		dlInterface = NULL;
	}
}

void CMainFrame::OnBtnDetect()
{
	HANDLE hDetectThread = CreateThread(NULL, 0, DetectThread, this, 0, NULL);
	if (hDetectThread == NULL)
	{
		MyMessageBox_Error(_T("OnBtnDetect"));
	}
	else
	{
		CloseHandle(hDetectThread);
	}
}

void CMainFrame::OnBtnSocket()
{
	CNDPCDThread(NULL);
// 	HANDLE hSocketThread = CreateThread(NULL, 0, SocketThread, this, 0, NULL);
// 	if (hSocketThread == NULL)
// 	{
// 		MyMessageBox_Error(_T("OnBtnSocket"));
// 	}
// 	else
// 	{
// 		CloseHandle(hSocketThread);
// 	}
}

void CMainFrame::OnBtnClear()
{
	CVisualADSDoc *pDoc = (CVisualADSDoc *) GetActiveDocument();

	CVisualADSApp *pApp = (CVisualADSApp *) AfxGetApp();
	//pApp->OpenDocumentFile(_T("C:\\1.txt"));
	//pApp->CloseAllDocuments(TRUE);

	
	//pDoc->MyOnClose();
	
	//pDoc->clearFilePathName();

	for (int i = 0; i < globalDomains.size(); i ++)
	{
		if (globalDomains[i] != NULL)
		{
			delete globalDomains[i];
		}
	}
	globalDomains.clear();

	CVisualADSView * pView = (CVisualADSView *) GetActiveView();
	pView->deleteAllShapes();
	pView->deleteAllLinks();

	COutputBar *outputBar = &m_wndOutput;
	outputBar->clearConsoleList();
	outputBar->clearReportList();
	outputBar->clearCacheList();

	//pDoc->DeleteContents();
	//pDoc->clearFilePathName();
	//pDoc->MyOnClose();
}

void CMainFrame::On_SocketEvent()
{
	MyMessageBox_Error("ss");
}

void CMainFrame::OnToggleDesignMode()
{
	CVisualADSView *pView = (CVisualADSView*) GetActiveView();
	m_bDesignMode = 1 - m_bDesignMode;
	pView->removeFrameShape();
	pView->GetCurrentModel()->SetDesignMode(m_bDesignMode);
	pView->FOPInvalidate();
	pView->ResetToSelectMode();
	RecalcLayout ();
}

void CMainFrame::OnUpdateToggleDesignMode(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (m_bDesignMode);
	pCmdUI->Enable (!GetDockManager ()->IsPrintPreviewValid ());
}

void CMainFrame::OnToggleChinese()
{
	m_bChineseOrEnglish = 1 - m_bChineseOrEnglish;
	g_bReboot = TRUE;
	setLanguage(m_bChineseOrEnglish);
	SendMessage(WM_CLOSE);
}

void CMainFrame::OnUpdateToggleChinese(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (m_bChineseOrEnglish);
	pCmdUI->Enable (!GetDockManager ()->IsPrintPreviewValid ());
}

void CMainFrame::OnToggleEnglish()
{
	m_bChineseOrEnglish = 1 - m_bChineseOrEnglish;
}

void CMainFrame::OnUpdateToggleEnglish(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (m_bChineseOrEnglish);
	pCmdUI->Enable (!GetDockManager ()->IsPrintPreviewValid ());
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == TIMER_PROGRESSBAR)
	{
		m_iProgress ++;
		if (m_iProgress > 100)
			m_iProgress -= 150;
		m_pProgress->SetPos(m_iProgress);
	}
	else if (nIDEvent == TIMER_NOTOPMOST)
	{
		if (m_hWndJava)
		{
			//::SetParent(m_hWndJava, m_hWnd);
		}
// 		if (!m_hWndJava)
// 		{
// 			
// 			return;
// 		}
// 		HWND hWndTop = ::GetForegroundWindow();
// 		if (hWndTop != m_hWnd)
// 		{
// 			::SetWindowPos(m_hWndJava, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
// 			::SetWindowPos(m_hWndJava, hWndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
// 			//Output_ReportList(itos(a), itos(a ++));
// 		}
	}

	CBCGPFrameWnd::OnTimer(nIDEvent);
}

LRESULT CMainFrame::On_MyMessageBox_Error(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strText = pParam->strParam1;
	CString strCaption = pParam->strParam2;
	delete pParam;
	pParam = NULL;

	if (strCaption == _T(""))
	{
		strText = strText + _T(" Error.");
		strCaption = _T("Error");
	}

	BCGP_MSGBOXPARAMS params;
	
	params.lpszCaption = strCaption;
	params.lpszText = strText;
	params.bDrawButtonsBanner = TRUE;
	params.bUseNativeControls = FALSE;
	
	params.dwStyle = MB_OK;
	params.bIgnoreStandardButtons = FALSE;
	params.bShowCheckBox = FALSE;
	
	params.dwStyle |= MB_ICONEXCLAMATION;
	
	int result = BCGPMessageBoxIndirect(&params);
	
	On_ProgressBar_Stop(NULL, NULL);

	return 0;
}

LRESULT CMainFrame::On_Output_ConsoleList(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strNo = pParam->strParam1;
	CString strContent = pParam->strParam2;
	delete pParam;
	pParam = NULL;
	
	COutputBar *outputBar = &m_wndOutput;
	outputBar->insert2ConsoleList(strNo, strContent);
	
	return 0;
}

LRESULT CMainFrame::On_Output_RacerProCache(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strNo = pParam->strParam1;
	CString strContent = pParam->strParam2;
	delete pParam;
	pParam = NULL;
	
	COutputBar *outputBar = &m_wndOutput;
	outputBar->insert2CacheList(strNo, strContent);

	return 0;
}

LRESULT CMainFrame::On_Output_ReportList(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strNo = pParam->strParam1;
	CString strContent = pParam->strParam2;
	delete pParam;
	pParam = NULL;

	COutputBar *outputBar = &m_wndOutput;
	outputBar->insert2ReportList(strNo, strContent);

	return 0;
}

LRESULT CMainFrame::On_Property_AddKeyValue_Basic(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strKey = pParam->strParam1;
	CString strValue = pParam->strParam2;
	delete pParam;
	pParam = NULL;

	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->addBasicProperty(strKey, strValue);

	return 0;
}

LRESULT CMainFrame::On_Property_AddKeyValue_Extend(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strKey = pParam->strParam1;
	CString strValue = pParam->strParam2;
	delete pParam;
	pParam = NULL;

	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->addExtensiveProperty(strKey, strValue);

	return 0;
}

LRESULT CMainFrame::On_Property_AddKeyValue_Father(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strKey = pParam->strParam1;
	CString strValue = pParam->strParam2;
	delete pParam;
	pParam = NULL;
	
	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->addFatherProperty(strKey, strValue);
	
	return 0;
}

LRESULT CMainFrame::On_Property_AddKeyValue_ChildGroup(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strKey = pParam->strParam1;
	CString strValue = pParam->strParam2;
	delete pParam;
	pParam = NULL;
	
	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->addChildGroupProperty(strKey, strValue);
	
	return 0;
}

LRESULT CMainFrame::On_Property_AddKeyValue_ChildUser(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strKey = pParam->strParam1;
	CString strValue = pParam->strParam2;
	delete pParam;
	pParam = NULL;
	
	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->addChildUserProperty(strKey, strValue);
	
	return 0;
}

LRESULT CMainFrame::On_Property_AddKeyValue_Access(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strKey = pParam->strParam1;
	CString strValue = pParam->strParam2;
	delete pParam;
	pParam = NULL;
	
	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->addAccessProperty(strKey, strValue);
	
	return 0;
}

LRESULT CMainFrame::On_Property_AddKeyValue_Network(WPARAM wParam, LPARAM lParam)
{
	PMParam *pParam = (PMParam *) wParam;
	CString strKey = pParam->strParam1;
	CString strValue = pParam->strParam2;
	delete pParam;
	pParam = NULL;
	
	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->addNetworkProperty(strKey, strValue);
	
	return 0;
}

LRESULT CMainFrame::On_Property_ClearKeyValues(WPARAM wParam, LPARAM lParam)
{
	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->clearProperties();

	return 0;
}

LRESULT CMainFrame::On_Property_UpdateKeyValues(WPARAM wParam, LPARAM lParam)
{
	CPropertiesBar *propertiesBar = &m_wndPropGrid;
	propertiesBar->updateProperties();

	return 0;
}

LRESULT CMainFrame::On_ProgressBar_Start(WPARAM wParam, LPARAM lParam)
{
	m_iProgress = -50;
	m_bProgressing = TRUE;
	m_pProgress->SetInfiniteMode(TRUE);
	m_pStatusLabel->SetText(_T("Running.."));
	m_pStatusLabel->Redraw();
	SetTimer(TIMER_PROGRESSBAR, 10, NULL);

	return 0;
}

LRESULT CMainFrame::On_ProgressBar_Stop(WPARAM wParam, LPARAM lParam)
{
	if (m_bProgressing == TRUE)
	{
		m_iProgress = 0;
		m_bProgressing = FALSE;
		m_pProgress->SetInfiniteMode(FALSE);
		m_pProgress->SetPos(m_iProgress);
		m_pStatusLabel->SetText(_T("Get Ready"));
		m_pStatusLabel->Redraw();
		KillTimer(TIMER_PROGRESSBAR);
	}

	return 0;
}

LRESULT CMainFrame::On_Canvas_Invalidate(WPARAM wParam, LPARAM lParam)
{
	CVisualADSView *pView = (CVisualADSView*) GetActiveView();
	ADDotReader* dotReader = new ADDotReader(pView, globalDomains);
	delete dotReader;
	dotReader = NULL;
	pView->FOPInvalidate();

	return 0;
}

LRESULT CMainFrame::On_Output_Update_Connection(WPARAM wParam, LPARAM lParam)
{
	updateServer();

	return 0;
}

#include <Wbemidl.h>
#include "ADPermissionSearch.h"

DWORD WINAPI LogOnThread(LPVOID lpParameter)
{
	//ADDirectorySearch::logOnByUserPassword("domaintest.net", "administrator", "123456Ad");
	//ADPermissionSearch::queryRegistryForShares("veotax");
	//ADPermissionSearch::modifyPrivilege(SE_BACKUP_NAME, TRUE);
	//ADPermissionSearch::getNTFSSecDescriptorByCreateFile("veotax", "C:\\no_one_can_read_me_haha");
	//ADDirectorySearch::logOff();
	//return 1;

	/*
	ADWMISearch *wmiSearcher = new ADWMISearch();
	IWbemServices *pSvc = wmiSearcher->connect("luo-pc", "127.0.0.1", "", "", "");
	IWbemClassObject *pLSSSClass = wmiSearcher->getWMIClass(pSvc, "Win32_LogicalShareSecuritySetting");

	//��ȡ���е�LSSSʵ��
	vector<IWbemClassObject*> npObjects = wmiSearcher->query(pSvc, "", "", "Select * from Win32_LogicalShareSecuritySetting");
	vector<CString> nStrPaths = wmiSearcher->getStringsFromObjects(npObjects, "__PATH");

	//��ȡ��һ��SD
	IWbemClassObject *pOutParams = wmiSearcher->callMethod(pSvc, pLSSSClass, "Win32_LogicalShareSecuritySetting",
		nStrPaths[0], "GetSecurityDescriptor");
	IWbemClassObject *pSDClass = wmiSearcher->getWMIClass(pSvc, "Win32_SecurityDescriptor");
	CString strSDClassPath = wmiSearcher->getStringFromObject(pSDClass, "__PATH");
	IWbemClassObject *pSDInstance = wmiSearcher->getObjectFromObjectWithCheck(pOutParams, "Descriptor");
	CString strSDInstancePath = wmiSearcher->getStringFromObject(pSDInstance, "__PATH");

	//��Win32_SecurityDescriptorHelper��Win32SDToSDDL������SDת����String
	IWbemClassObject *pHelperClass = wmiSearcher->getWMIClass(pSvc, "Win32_SecurityDescriptorHelper");
	IWbemClassObject *pOutParams2 = wmiSearcher->callStaticMethod(pSvc, pHelperClass, "Win32_SecurityDescriptorHelper",
		"Win32SDToSDDL", wmiSearcher->genObjectArguments("Descriptor", pSDInstance));
	CString strSDDL = wmiSearcher->getStringFromObject(pOutParams2, "SDDL");

	//CString strSDPath = wmiSearcher->getStringFromObject(pSDInstance, "__PATH");
	//IWbemClassObject *pDACLInstance = wmiSearcher->getWMIObjectFromObject(pSDInstance, "Descriptor");
	//IWbemClassObject *pOutParams2 = wmiSearcher->callMethod(pSvc, pSDClass, "Win32_SecurityDescriptor", strSDPath, "aa");

	wmiSearcher->clearObjects(npObjects);
	wmiSearcher->disconnect(pSvc);
	return 0;
	*/


	CMainFrame *pThis = (CMainFrame *) lpParameter;
	ProgressBar_Start();
	/*
	ADDotReader* dotReader2 = new ADDotReader(NULL, NULL);
	CPoint pt = dotReader2->getBBFromLine(4);
	int b = pt.y;
	return;
	*/
	vector<CString> nstrDCNames;
	vector<CString> nstrUsernames;
	vector<CString> nstrPasswords;

	for (int i = 0; i < pThis->m_npEditDCNames.size(); i ++)
	{
		nstrDCNames.push_back(pThis->m_npEditDCNames[i]->GetEditText());
		nstrUsernames.push_back(pThis->m_npEditUsernames[i]->GetEditText());
		nstrPasswords.push_back(pThis->m_npEditPasswords[i]->GetEditText());
		if (nstrDCNames[i] == _T("") || nstrUsernames[i] == _T("") || nstrPasswords[i] == _T(""))
		{
			MyMessageBox_Error(_T("�벹ȫ��������������Ա�˻��������Ϣ��"), _T("������ʾ��Ϣ"));
			return 0;
		}
	}

	int iCntDomains =  pThis->m_npEditDCNames.size();
	pThis->globalDomains.clear();
	for (i = 0; i < iCntDomains; i ++)
	{
		ADDomain *domain = new ADDomain(nstrDCNames[i], nstrUsernames[i], nstrPasswords[i]);
		if (domain->dNSName == _T(""))
		{
			delete domain;
			domain = NULL;
			return 0;
		}
		else
		{
			pThis->globalDomains.push_back(domain);
		}
	}

	for (i = 0; i < pThis->globalDomains.size(); i ++)
	{
		pThis->globalDomains[i]->setGlobalDomains(pThis->globalDomains);
		pThis->globalDomains[i]->refreshAllShareFolderInfos();
		pThis->globalDomains[i]->addToProperties();
		pThis->globalDomains[i]->updateComputerCNs();
	}

	/*
	CString strDCName = pThis->m_pEditDCName->GetEditText();
	CString strUsername = pThis->m_pEditUsername->GetEditText();
	CString strPassword = pThis->m_pEditPassword->GetEditText();

	if (strDCName == _T(""))
	{
		MyMessageBox_Error(_T("�����������������"), _T("������ʾ��Ϣ"));
		return 0;
	}
	else if (strUsername == _T(""))
	{
		MyMessageBox_Error(_T("�����������Ա�˻�"), _T("������ʾ��Ϣ"));
		return 0;
	}
	else if (strPassword == _T(""))
	{
		MyMessageBox_Error(_T("�����������Ա����"), _T("������ʾ��Ϣ"));
		return 0;
	}

	ADDomain *domain = new ADDomain(strDCName, strUsername, strPassword);
	if (domain->dNSName == _T(""))
	{
		delete domain;
		domain = NULL;
		return 0;
	}

	pThis->globalDomains.push_back(domain);
	domain->setGlobalDomains(pThis->globalDomains);
	domain->refreshAllShareFolderInfos();
	domain->addToProperties();
	*/

	ADGraphGenerator* graphGenerator = new ADGraphGenerator(pThis->globalDomains, _T("ADDomain_Buffer"));
	delete graphGenerator;
	graphGenerator = NULL;

	Canvas_Invalidate();
	ProgressBar_Stop();
	return 1;
}

DWORD WINAPI LoadThread(LPVOID lpParameter)
{
	CMainFrame *pThis = (CMainFrame *) lpParameter;
	ProgressBar_Start();

	for (int i = 0; i < pThis->globalDomains.size(); i ++)
	{
		pThis->globalDomains[i]->setGlobalDomains(pThis->globalDomains);
		
		//pThis->globalDomains[i]->refreshAllShareFolderInfos();
	}

	for (i = 0; i < pThis->globalDomains.size(); i ++)
	{
		pThis->globalDomains[i]->clarifyAllGroupsAndUsers(); //Only used here !!
		pThis->globalDomains[i]->refreshAllGroupMembers_LoadMode(FALSE);
		Output_ConsoleList(_T("Finished parsing domain: ") + pThis->globalDomains[i]->dNSName + _T("."));
	}
	
	for (i = 0; i < pThis->globalDomains.size(); i ++)
	{
		pThis->globalDomains[i]->addToProperties();
	}

	Output_ConsoleList(_T("Cache restoring finished!"));

	ADGraphGenerator* graphGenerator = new ADGraphGenerator(pThis->globalDomains, _T("ADDomain_Buffer"));
	delete graphGenerator;
	graphGenerator = NULL;
	
	Canvas_Invalidate();
	ProgressBar_Stop();
	return 1;
}

DWORD WINAPI DetectThread(LPVOID lpParameter)
{
	CMainFrame *pThis = (CMainFrame *) lpParameter;
	ProgressBar_Start();

	//DLInterface *dlInterface;
	//dlInterface = new DLInterface(NULL);
	//return;
	
	if (pThis->globalDomains.size() == 0)
	{
		CString strText = _T("�����Ƚ�������Ϣ�ɼ���");
		MyMessageBox_Error(strText, _T("������ʾ��Ϣ"));
		ProgressBar_Stop();
		return 0;

		DLInterface *pDLInterfaceForTest;
		
		CString strRacerIP = pThis->m_pEditRacerIP->GetEditText();
		if (strRacerIP != _T("") && strRacerIP != _T("127.0.0.1") && ADDirectorySearch::checkIP(strRacerIP))
		{
			//no check again!!!
			/*
			if (!DLInterface::checkConnection(strRacerIP))
			{
				CString strText = _T("��������δ��Զ�̼����") + strRacerIP + _T("������");
				MyMessageBox_Error(strText, _T("������ʾ��Ϣ"));
				ProgressBar_Stop();
				return 0;
			}
			else
			*/
			{
				pDLInterfaceForTest = new DLInterface(strRacerIP);
				//dlInterfaceForTest->endEngine();
				delete pDLInterfaceForTest;
				pDLInterfaceForTest = NULL;
			}
		}
		else
		{
			pDLInterfaceForTest = new DLInterface();
			//dlInterfaceForTest->endEngine();
			delete pDLInterfaceForTest;
			pDLInterfaceForTest = NULL;
		}
		
		ProgressBar_Stop();
		return 1;
	}
	else
	{
		

		int iConflictNo;
		DLInterface *pDLInterfaceShare;
		DLInterface *pDLInterfaceNTFS;
		//DLInterface *pDLInterfaceInternal;

// 		BOOL bFileInterface = FALSE;
// 		if (bFileInterface)
// 		{
// 			iConflictNo = 0;
// 			pDLInterfaceShare = new DLInterface(pThis->globalDomains, TRUE, TRUE); //Share
// 			pDLInterfaceShare->queryAllConflicts_FileInterface(pThis->globalDomains, iConflictNo);
// 			delete pDLInterfaceShare;
// 			pDLInterfaceShare = NULL;
// 			
// 			iConflictNo = 0;
// 			pDLInterfaceNTFS = new DLInterface(pThis->globalDomains, FALSE, TRUE); //NTFS
// 			pDLInterfaceNTFS->queryAllConflicts_FileInterface(pThis->globalDomains, iConflictNo);
// 			delete pDLInterfaceNTFS;
// 			pDLInterfaceNTFS = NULL;
// 		}
// 		else
// 		{
		CString strRacerIP = pThis->m_pEditRacerIP->GetEditText();
		if (strRacerIP != _T("") && strRacerIP != _T("127.0.0.1") && ADDirectorySearch::checkIP(strRacerIP))
		{
			if (!DLInterface::checkConnection(strRacerIP))
			{
				CString strText = _T("��������δ��Զ�̼����") + strRacerIP + _T("������");
				MyMessageBox_Error(strText, _T("������ʾ��Ϣ"));
				ProgressBar_Stop();
				return 0;
			}
			else
			{
				pDLInterfaceShare = new DLInterface(pThis->globalDomains, strRacerIP, 1);
				if (pThis->m_bSelfStartRacer == FALSE)
				{
					pThis->m_bSelfStartRacer = pDLInterfaceShare->getSelfStarted();
				}

				iConflictNo = 0;
				pDLInterfaceShare->queryAllConflicts(pThis->globalDomains, iConflictNo);
				delete pDLInterfaceShare;
				pDLInterfaceShare = NULL;

				iConflictNo = 0;
				pDLInterfaceNTFS = new DLInterface(pThis->globalDomains, strRacerIP, 0);
				pDLInterfaceNTFS->queryAllConflicts(pThis->globalDomains, iConflictNo);
				delete pDLInterfaceNTFS;
				pDLInterfaceNTFS = NULL;

				/*
				iConflictNo = 0;
				pDLInterfaceInternal = new DLInterface(pThis->globalDomains, strRacerIP, 2);
				pDLInterfaceInternal->queryAllConflicts(pThis->globalDomains, iConflictNo);
				delete pDLInterfaceInternal;
				pDLInterfaceInternal = NULL;
				*/
			}
		}
		else
		{
			if (!DLInterface::checkEngine())
			{
				CString strText = _T("��������δ�ڱ��ؼ����������");
				MyMessageBox_Error(strText, _T("������ʾ��Ϣ"));
				ProgressBar_Stop();
				return 0;
			}

			DLInterface *pDLInterfaceMix;
			pDLInterfaceMix = new DLInterface(pThis->globalDomains, TYPE_MIX);
			if (pThis->m_bSelfStartRacer == FALSE)
			{
				pThis->m_bSelfStartRacer = pDLInterfaceMix->getSelfStarted();
			}
			
			iConflictNo = 0;
			pDLInterfaceMix->queryAllConflicts(pThis->globalDomains, iConflictNo);
			delete pDLInterfaceMix;
			pDLInterfaceMix = NULL;

// 			pDLInterfaceShare = new DLInterface(pThis->globalDomains, TYPE_SHARE);
// 			if (pThis->m_bSelfStartRacer == FALSE)
// 			{
// 				pThis->m_bSelfStartRacer = pDLInterfaceShare->getSelfStarted();
// 			}
// 
// 			iConflictNo = 0;
// 			pDLInterfaceShare->queryAllConflicts(pThis->globalDomains, iConflictNo);
// 			delete pDLInterfaceShare;
// 			pDLInterfaceShare = NULL;
// 
// 			iConflictNo = 0;
// 			pDLInterfaceNTFS = new DLInterface(pThis->globalDomains, TYPE_NTFS);
// 			pDLInterfaceNTFS->queryAllConflicts(pThis->globalDomains, iConflictNo);
// 			delete pDLInterfaceNTFS;
// 			pDLInterfaceNTFS = NULL;
		}
/*	} */

		ProgressBar_Stop();
		return 1;
	}
}

void CMainFrame::OnClose() 
{
	if (m_bSelfStartRacer)
	{
		DLInterface::endEngine();
	}

	if (m_hWndJava)
	{
		::SendMessage(m_hWndJava, WM_CLOSE, 0, 0);
	}

	if (g_bReboot)
	{
        CString strExePathName = m_strExePath + _T("VisualADS.exe");
        STARTUPINFO StartInfo;
        PROCESS_INFORMATION procStruct;
        memset(&StartInfo, 0, sizeof(STARTUPINFO));
        StartInfo.cb = sizeof(STARTUPINFO);
        ::CreateProcess(
            (LPCTSTR) strExePathName,
            NULL,
            NULL,
            NULL,
            FALSE,
            NORMAL_PRIORITY_CLASS,
            NULL,
            NULL,
            &StartInfo,
            &procStruct);
     }

	CBCGPFrameWnd::OnClose();
}






BOOL CMainFrame::logOnByUserPassword(CString strDomainName, CString strUserName, CString strPassword)
{
	CString strUPNUserName = strUserName + _T("@") + strDomainName;
	
	HANDLE hToken;
	BOOL bResult;
	//bResult = LogonUser(strUserName, strDomainName, strPassword, LOGON32_LOGON_NEW_CREDENTIALS, LOGON32_PROVIDER_DEFAULT,
	//	&hToken);
	if (strDomainName != _T(""))
	{
		bResult = LogonUser(strUPNUserName, _T(""), strPassword, LOGON32_LOGON_NEW_CREDENTIALS, LOGON32_PROVIDER_DEFAULT, 
			&hToken); //LOGON32_LOGON_NEW_CREDENTIALS or LOGON32_LOGON_NETWORK_CLEARTEXT
	}
	else
	{
		bResult = LogonUser(strUserName, _T("."), strPassword, LOGON32_LOGON_NEW_CREDENTIALS, LOGON32_PROVIDER_DEFAULT, 
			&hToken); //LOGON32_LOGON_NEW_CREDENTIALS
	}
	if (bResult == FALSE)
	{
		//MyMessageBox_Error(_T("logOnByUserPassword Error."), _T("Error"));
		return FALSE;
	}
	else
	{
		bResult = ImpersonateLoggedOnUser(hToken);
		if (bResult == FALSE)
		{
			//MyMessageBox_Error(_T("logOnByUserPassword Error."), _T("Error"));
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

BOOL CMainFrame::logOff()
{
	BOOL bResult;
	bResult = RevertToSelf();
	if (bResult == FALSE)
	{
		//MyMessageBox_Error(_T("logOff Error."), _T("Error"));
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void CMainFrame::TraceUserLogonInfo(CString cn, CString NetBIOSName, CString DNSname,CString UserName, CString PassWord)
{
	CString Computer_User_name;
	CString Computer_name;
	CString LogonTime;
	CString LogofTime;
	CString LogonStyle;

	vector <UserLogonInfo> LogonInfo;
	UserLogonInfo precent;


    DWORD Last_Time;

	HANDLE hdle;
	DWORD  Logtatal;
	
	BOOL    bRetVal					= FALSE;
	DWORD  dwEvLogCounter			;

	
	EVENTLOGRECORD *ptr;
	BYTE buff[4096];
	DWORD  dwNumberOfBytesToRead	= 0;
	DWORD read_len, next_len;
	ptr=(EVENTLOGRECORD *)&buff;
	
	TCHAR lpUNCServerName[_MAX_PATH + 1];


	


	wsprintf(lpUNCServerName, _T("\\\\%s"), cn);
	logOnByUserPassword(NetBIOSName, UserName, PassWord);
	hdle=OpenEventLog(lpUNCServerName, "Security");// Security	
	logOff();

	if (hdle==NULL)
	{
		MyMessageBox_Error(_T("TraceUserLogonInfo"));
	}
	else
	{

		GetNumberOfEventLogRecords(hdle,&Logtatal);
		CString UserFilePath;
//		CString cnNetbiosName;
		string cnNetbiosName;
		DWORD dwCounter;
		UserFilePath.Format("UserLog\\%s@%s.txt",DNSname,cn);
		ifstream infile(UserFilePath);
		
		if(infile)
		{
			infile>>dwCounter;
			dwEvLogCounter=dwCounter;
		}
		else
			dwEvLogCounter=1;

		infile.close();
		CString ltt;
		ltt.Format("%d",Logtatal+1-dwEvLogCounter);
		MessageBox(ltt);
		while(1)
		{
			
			bRetVal=ReadEventLog(hdle,EVENTLOG_FORWARDS_READ|EVENTLOG_SEEK_READ,
				dwEvLogCounter,ptr,dwNumberOfBytesToRead,&read_len,&next_len);
			
			
			if(!bRetVal)
			{
				if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					dwNumberOfBytesToRead=next_len;
					continue;
				}
				else
					break;
				
			}
			if(dwEvLogCounter == Logtatal)
				Last_Time = ptr ->TimeWritten;
			dwEvLogCounter++;//Ϊ��ȡ������¼��׼��


			
			if(ptr->EventID==528 || ptr->EventID==540)//��¼�û��Ժε�½��ʽ(2,3,10)�ĵ�½
			{

				

				PEVENTLOGRECORD pELR = 0;
				LPBYTE			pData = 0;
				HMODULE hModule = 0;
				
				LPBYTE			pStrings = 0;
				UINT uStringOffset;
				TCHAR *szExpandedString;
				LPVOID lpszBuffer = 0;
				//char buf[1024];
				pELR	= (PEVENTLOGRECORD)ptr;
				

			
				UINT  uStepOfString = 0;		
				pStrings = (LPBYTE)GlobalAlloc(GPTR, (pELR->DataOffset - pELR->StringOffset) * sizeof(BYTE));
				memcpy(pStrings, (LPBYTE)pELR + pELR->StringOffset, pELR->DataOffset - pELR->StringOffset);
				
				szExpandedString =(TCHAR *)GlobalAlloc(GPTR,  (pELR->DataOffset - pELR->StringOffset + 1024) * sizeof(TCHAR));
			


			
			  //hModule = LoadLibraryEx("C:\\Windows\\winsxs\\x86_microsoft-windows-msauditevtlog_31bf3856ad364e35_6.1.7600.16385_none_c718d071d9c10a2d\\MsAuditE.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
				hModule = LoadLibraryEx("c:\\windows\\System32\\MsAuditE.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);//ws03res,MsAuditE,xpsp2res,NtMarta,MsObjs
				
				//LOAD_LIBRARY_AS_DATAFILE DONT_RESOLVE_DLL_REFERENCES
				if(hModule)
				{
					TCHAR **_sz = (TCHAR**)GlobalAlloc(GPTR, (pELR->NumStrings) * sizeof(TCHAR *));
					register UINT z;
					
					uStringOffset = 0;
					for(z = 0; z < pELR->NumStrings; z++)
					{
						_sz[z] = (TCHAR *)GlobalAlloc(GPTR, 
							(strlen((TCHAR *)pStrings + uStringOffset) + 1) * sizeof(TCHAR));
						strcpy(_sz[z], (TCHAR *)pStrings + uStringOffset);
						
						uStringOffset += strlen((TCHAR *)pStrings + uStringOffset) + 1;
					}


				                
				 					
					FormatMessageA(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_HMODULE |
						//FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_ARGUMENT_ARRAY,
						hModule, pELR->EventID, 0, (LPTSTR) &lpszBuffer, 1024, 
						_sz
						);
					
					for(z = 0; z < pELR->NumStrings; z++)
					{
						SafeDeletePointer(_sz[z], strlen(_sz[z]));
						_sz[z] = 0;
					}
					SafeDeletePointer(_sz, (pELR->NumStrings) * sizeof(TCHAR *));
					_sz = 0;
				
					if(lpszBuffer)
					{
					
						strcpy(szExpandedString, (TCHAR *)lpszBuffer);
						uStringOffset = strlen(szExpandedString);
					}
					
					if(lpszBuffer)
						LocalFree(lpszBuffer);

					FreeLibrary(hModule);
					
					
			
				}
				else
				{
					MyMessageBox_Error("�޷��ҵ������ֶ��ļ�����ȡʧ��!");
					continue;
				}
				CString sszExpandedString=szExpandedString;
			
				stringstream tt( sszExpandedString.GetBuffer(sszExpandedString.GetLength()) );//( string(sszExpandedString) );
				sszExpandedString.ReleaseBuffer();
			


				string lss;
				CString user="",domain="",LogId="";
				CString user1,domain1;
				TCHAR Log_Style;
				


				tt.seekg( sszExpandedString.Find("��¼����")+9 );
				tt>>lss;
				if(lss=="10")
					Log_Style='r';
				else if(lss=="2")
					Log_Style='2';
				else if(lss=="3")
					Log_Style='3';
				else
					continue;


				tt.seekg(  sszExpandedString.Find("�û���")+7 );
				tt>>lss;
				while(lss!="��:")
				{
					user1.Format("%s",lss.c_str() );
					user=' '+user1;
					tt>>lss;
				}
				if(user=="")
					user="A";
				else
					user.Delete(0,1);//ɾ����һ������ո�
				if(user==cn+'$')//����system  ��Ϣ
					continue;
				

				tt.seekg(  sszExpandedString.Find("��")+3);
				tt>>lss;
				while(lss!="��¼")
				{
					domain1.Format("%s",lss.c_str() );
					domain=' '+domain1;
					tt>>lss;
				}
				if(domain=="")
					domain="N";
				else
					domain.Delete(0,1);//ɾ����һ������ո�

				if(user=="A"&&domain=="N")
					continue;

				tt.seekg( sszExpandedString.Find("��¼ ID")+8 );
				tt>>lss;
				LogId.Format("%s",lss.c_str() );
				LogId.MakeLower();
			//	MessageBox(LogId);

							

				precent.EventId=ptr->EventID;
				precent.Computer_name=cn;
				precent.LogonId=LogId;
				struct tm *p;
				precent.D_LogonTime=ptr->TimeGenerated;
				p=localtime((time_t *)&(ptr->TimeGenerated));
				precent.WriteTime.Format("%d/%d/%d %d:%d:%d",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
				precent.User_name.Format("%s\\%s",domain,user);
				precent.LogonStyle=Log_Style;
				precent.dwCounter=dwEvLogCounter;
				LogonInfo.push_back(precent);


			}
			else if( ptr->EventID==551 || ptr->EventID==538 )// �û�������ע����ʽ
			{



				PEVENTLOGRECORD pELR = 0;
				LPBYTE			pData = 0;
				HMODULE hModule = 0;
				HKEY   hk						= (HKEY)0;
				LPBYTE			pStrings = 0;
				UINT uStringOffset;
				TCHAR *szExpandedString;
				LPVOID lpszBuffer = 0;
				//char buf[1024];
				pELR	= (PEVENTLOGRECORD)ptr;
				
				
				UINT  uStepOfString = 0;		
				pStrings = (LPBYTE)GlobalAlloc(GPTR, pELR->DataOffset - pELR->StringOffset * sizeof(BYTE));
				memcpy(pStrings, (LPBYTE)pELR + pELR->StringOffset, pELR->DataOffset - pELR->StringOffset);
				
				szExpandedString = (TCHAR *)GlobalAlloc(GPTR,  (pELR->DataOffset - pELR->StringOffset + 1024) * sizeof(TCHAR));
				
				hModule = LoadLibraryEx("c:\\windows\\System32\\MsAuditE.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
				if(hModule)
				{
					TCHAR **_sz = (TCHAR**)GlobalAlloc(GPTR, (pELR->NumStrings) * sizeof(TCHAR *));
					register UINT z;
					
					uStringOffset = 0;
					for(z = 0; z < pELR->NumStrings; z++)
					{
						_sz[z] = (TCHAR *)GlobalAlloc(GPTR, 
							(strlen((TCHAR *)pStrings + uStringOffset) + 1) * sizeof(TCHAR));
						strcpy(_sz[z], (TCHAR *)pStrings + uStringOffset);
						
						uStringOffset += strlen((TCHAR *)pStrings + uStringOffset) + 1;
					}
					
					FormatMessageA(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_HMODULE | 
						//FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_ARGUMENT_ARRAY,
						hModule, pELR->EventID, 0, (LPTSTR) &lpszBuffer, 1024, 
						_sz
						);
					
					for(z = 0; z < pELR->NumStrings; z++)
					{
						SafeDeletePointer(_sz[z], strlen(_sz[z]));
						_sz[z] = 0;
					}
					SafeDeletePointer(_sz, (pELR->NumStrings) * sizeof(TCHAR *));
					_sz = 0;
					
					if(lpszBuffer)
					{
						strcpy(szExpandedString, (TCHAR *)lpszBuffer);
						uStringOffset = strlen(szExpandedString);
					}
					
					if(lpszBuffer)
						LocalFree(lpszBuffer);
					
					FreeLibrary(hModule);
					
					
					
				}
				else
				{
					MyMessageBox_Error("�޷��ҵ������ֶ��ļ�����ȡʧ��!");
					continue;
				}
				//551|538
				CString sszExpandedString=szExpandedString;
				
				stringstream tt( sszExpandedString.GetBuffer(sszExpandedString.GetLength()) );//( string(sszExpandedString) );
				sszExpandedString.ReleaseBuffer();
			

			
				string lss;
				CString user="",domain="",LogId="";
				CString user1,domain1;
				TCHAR Log_Style='N';
				
				
	
				
				tt.seekg(  sszExpandedString.Find("�û���")+7 );
				tt>>lss;
				while(lss!="��:")
				{
					user1.Format("%s",lss.c_str() );
					user=' '+user1;
					tt>>lss;
				}
				if(user=="")
					user="A";
				else
					user.Delete(0,1);//ɾ����һ������ո�
				
				if(user==cn+'$')//����system  ��Ϣ
					continue;
				
				tt.seekg(  sszExpandedString.Find("��")+3);
				tt>>lss;
				while(lss!="��¼")
				{
					domain1.Format("%s",lss.c_str() );
					domain=' '+domain1;
					tt>>lss;
				}
				if(domain=="")
					domain="N";
				else
					domain.Delete(0,1);//ɾ����һ������ո�
				
				tt.seekg( sszExpandedString.Find("��¼ ID")+8 );
				tt>>lss;
				LogId.Format("%s",lss.c_str() );
				LogId.MakeLower();

				
				
				precent.EventId=ptr->EventID;
				precent.Computer_name=cn;
				precent.LogonId=LogId;
				struct tm *p;
				precent.D_LogonTime=ptr->TimeGenerated;
				p=localtime((time_t *)&(ptr->TimeGenerated));
				precent.WriteTime.Format("%d/%d/%d %d:%d:%d",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
				precent.User_name.Format("%s\\%s",domain,user);
				precent.LogonStyle=Log_Style;
				precent.dwCounter=dwEvLogCounter;
				LogonInfo.push_back(precent);
				

				
					
			}				
				
		}
		
	}
	CloseEventLog(hdle);

	vector <UserLogonInfo> LogonInfo_final;
	UserLogonInfo SingalLog;
	//	CString opo;
	int iii=LogonInfo.size();
	for(int i=0; i< LogonInfo.size() ;i++)
	{
		
		//		opo.Format("%s %s %d\n%s",LogonInfo[i].User_name,LogonInfo[i].Computer_name,LogonInfo[i].LogonStyle,LogonInfo[i].WriteTime);
		if(LogonInfo[i].EventId==528 || LogonInfo[i].EventId==540 )
		{
//			int iflag=LogonInfo.size();
			for(int j=i; j<LogonInfo.size(); j++)
				
				if(LogonInfo[j].EventId == 551 || LogonInfo[j].EventId==538 )
				{
					
					if( LogonInfo[i].LogonId ==LogonInfo[j].LogonId )
					{

						if(LogonInfo[i].D_LogonTime +15 > LogonInfo[j].D_LogonTime)
							break;

						SingalLog.User_name = LogonInfo[i].User_name;
						SingalLog.Computer_name = LogonInfo[i].Computer_name;
						SingalLog.D_LogonTime=LogonInfo[i].D_LogonTime;
						SingalLog.LogonTime = LogonInfo[i].WriteTime;
						SingalLog.LogofTime = LogonInfo[j].WriteTime;
						SingalLog.LogonStyle = LogonInfo[i].LogonStyle;
						SingalLog.dwCounter=LogonInfo[i].dwCounter;
						LogonInfo_final.push_back(SingalLog);
						break;
					}
				}
					
				
		}
	}

	


	CString UserFilePath;
	CStdioFile file;
	
	int num;
	vector < UserLogonInfo >  Updata_LogonInfo;
	UserLogonInfo Updata;
	DWORD Begin_Time,End_Time;
	string up1,up2,up3,up4;
	DWORD dwpas;
	UserFilePath.Format("%s\\%s@%s.txt","UserLog",DNSname,cn);	
	
	
	int fileflag=1;
	if(GetFileAttributes(UserFilePath)==-1)
		fileflag=0;
	
	
	if(fileflag==0)
	{
		file.Open(UserFilePath, CFile::modeCreate| CFile::modeReadWrite| CFile::modeNoTruncate);
		file.Close();
	}

	ifstream infile(UserFilePath);
	if(fileflag)
	{
		infile>>dwpas;
		infile>>num>>Begin_Time>>End_Time;
		for(int m=0; m  < num ;m++)
		{
			infile>>up1>>up2;
			Updata.User_name.Format("%s",up1.c_str());
			Updata.Computer_name.Format("%s",up2.c_str());
			infile>>Updata.D_LogonTime;
			infile>>up3>>up4;
			Updata.LogonTime.Format("%s %s",up3.c_str(),up4.c_str());
			infile>>up3>>up4;
			Updata.LogofTime.Format("%s %s",up3.c_str(),up4.c_str());
			infile>>Updata.LogonStyle;
			Updata_LogonInfo.push_back(Updata);
		}


		for(int k=0; k < LogonInfo_final.size(); k++)
		{
			if(LogonInfo_final[k].D_LogonTime<Begin_Time)
			{
				Updata_LogonInfo.insert(Updata_LogonInfo.begin(),LogonInfo_final[k]);
				Begin_Time = LogonInfo_final[k].D_LogonTime;
			}
			else if(LogonInfo_final[k].D_LogonTime>End_Time)
			{
				Updata_LogonInfo.push_back(LogonInfo_final[k]);
				End_Time = LogonInfo_final[k].D_LogonTime;	
				dwpas=LogonInfo_final[k].dwCounter;
			}
			else
			{
				for(m=0; m < num ;m++ )
				{
					if( LogonInfo_final[k].D_LogonTime < Updata_LogonInfo[m].D_LogonTime )
					{
						Updata_LogonInfo.insert(Updata_LogonInfo.begin()+m,LogonInfo_final[k]);
						break;
					}
					else if( LogonInfo_final[k].D_LogonTime == Updata_LogonInfo[m].D_LogonTime )
					{
						if( LogonInfo_final[k].Computer_name!=Updata_LogonInfo[m].Computer_name )
							Updata_LogonInfo.insert(Updata_LogonInfo.begin()+m,LogonInfo_final[k]);
						break;
					}
				}			
			}
		}
	}

	else
	{
		for(int k=0; k < LogonInfo_final.size(); k++)
			Updata_LogonInfo.push_back(LogonInfo_final[k]);
		if(LogonInfo_final.size() > 0)
		{
			dwpas=LogonInfo_final[k-1].dwCounter;
			Begin_Time=LogonInfo_final[0].D_LogonTime;
			End_Time=LogonInfo_final[k-1].D_LogonTime;	
		}
	}

	if( Updata_LogonInfo.size() == 0 )
	{
		dwpas=Logtatal;
		Begin_Time=End_Time=Last_Time;

	}

	infile.close();
	ofstream infile1(UserFilePath);
		
	infile1<<dwpas<<endl;
	infile1<<Updata_LogonInfo.size()<<' '<<Begin_Time<<' '<<End_Time<<endl;
	for(int p=0 ; p < Updata_LogonInfo.size(); p++)
	{
		up1=Updata_LogonInfo[p].User_name.GetBuffer(Updata_LogonInfo[p].User_name.GetLength());
		Updata_LogonInfo[p].User_name.ReleaseBuffer();

		up2=Updata_LogonInfo[p].Computer_name.GetBuffer(Updata_LogonInfo[p].Computer_name.GetLength());
		Updata_LogonInfo[p].Computer_name.ReleaseBuffer();

		up3=Updata_LogonInfo[p].LogonTime.GetBuffer(Updata_LogonInfo[p].LogonTime.GetLength());
		Updata_LogonInfo[p].LogonTime.ReleaseBuffer();
			
		up4=Updata_LogonInfo[p].LogofTime.GetBuffer(Updata_LogonInfo[p].LogofTime.GetLength());
		Updata_LogonInfo[p].LogofTime.ReleaseBuffer();

		infile1<<up1<<' '
		<<up2<<' '
		<<Updata_LogonInfo[p].D_LogonTime<<' '
		<<up3<<' '
		<<up4<<' '
		<<Updata_LogonInfo[p].LogonStyle<<endl;		
	}
	infile.close();
	

}




void CMainFrame::ShowLogInfoOutputBar(CString DNSname,CString cn)
{
	CString  filePath;

	CFileFind finder;
	BOOL bFind=FALSE;
	filePath.Format("UserLog\\%s@%s.txt",DNSname,cn);
	bFind=finder.FindFile(filePath);
	if(!bFind)
		return;


	int num;
	DWORD Begin_Time,End_Time;
	string up1,up2,up3,up4;
	UserLogonInfo Out;
	
	CString LogonStyle;


	DWORD pas;
	ifstream infile(filePath);
	if(!infile)
		return;
	infile>>pas;
	infile>>num>>Begin_Time>>End_Time;
	for(int m=0; m  < num ;m++)
	{
			infile>>up1>>up2;
			Out.User_name.Format("%s",up1.c_str());
			Out.Computer_name.Format("%s",up2.c_str());
			infile>>Out.D_LogonTime;
			infile>>up3>>up4;
			Out.LogonTime.Format("%s %s",up3.c_str(),up4.c_str());
			infile>>up3>>up4;
			Out.LogofTime.Format("%s %s",up3.c_str(),up4.c_str());
			infile>>Out.LogonStyle;

			if(Out.LogonStyle=='2')
				LogonStyle="����ʽ��½";
			else if(Out.LogonStyle=='3')
				LogonStyle="����";
			else
				LogonStyle="Զ�̽���";
			//	:SendMessage(m_wndOutput.m_Logwnd,  WM_LBUTTONDOWN  ,1,0);
			
			m_wndOutput.insert2LogonInfo(Out.Computer_name, Out.LogonTime, Out.LogofTime,
					Out.User_name, LogonStyle);

	}	

	infile.close();
	


}

void CMainFrame::OnBtnLogInfoSearch()
{
	//��ȡ��־��Ϣ
	
	
	HANDLE hLogOnSearchThread = CreateThread(NULL, 0, LogOnSearchThread, this, 0, NULL);
	if (hLogOnSearchThread == NULL)
	{
		MyMessageBox_Error(_T("OnBtnLogOnSearch"));
	}
	else
	{
		CloseHandle(hLogOnSearchThread);
	}
}

DWORD WINAPI LogOnSearchThread(LPVOID lpParameter)
{
	
	CMainFrame *pThis = (CMainFrame *) lpParameter;
	BOOL    bRetVal					= FALSE;
	DWORD  dwEvLogCounter			= 1;
	CString cn;
	
	EVENTLOGRECORD *ptr;
	BYTE buff[4096];
	DWORD  dwNumberOfBytesToRead	= 0;
	
	ptr=(EVENTLOGRECORD *)&buff;
	pThis -> m_wndOutput.clearLogonInfo();
	
	
//		pThis -> TraceUserLogonInfo("WIN2003-SP2-1","tech","tech.adtest.net","administrator","123456Ad");
	
	//	int ssss=this->globalDomains.size();
	for(int i =0; i < pThis->globalDomains.size(); i++)
	{
		//		int jjjj=this->globalDomains[i]->computers.size();
		for(int j =0; j < pThis->globalDomains[i]->g_computers.size(); j++)
		{ 
			cn=pThis-> globalDomains[i] ->g_computers[j]->cn;
			if(pThis-> globalDomains[i] ->g_computers[j]->run==1&& cn!=pThis ->globalDomains[i] ->dcName)
				
				pThis -> TraceUserLogonInfo(cn ,pThis -> globalDomains[i] ->netBIOSName ,pThis -> globalDomains[i] ->dNSName,
				pThis -> globalDomains[i] ->userName ,pThis -> globalDomains[i] -> passWord);
			
			
			pThis -> ShowLogInfoOutputBar(pThis -> globalDomains[i] ->dNSName,cn);	
			
			
		}
		
	}
	return 1;
}

DWORD WINAPI CNDPCDThread(LPVOID lpParameter)
{
	CMainFrame *pThis = (CMainFrame *) lpParameter;
	ProgressBar_Start();

	CString strJavaPathName = pThis->m_strInputDir + _T("CNDPCD.jar");
	CString strJavaTitle = _T("���������������Գ�ͻ��⹤��");
	// 	CString strJavaPathName = _T("C:\\Windows\\System32\\mspaint.exe");
	// 	CString strJavaTitle = _T("Untitled - Paint");
	
	HWND hWnd = pThis->GetGlobalHandleByTitle(strJavaTitle);
	if (!hWnd)
	{
		ShellExecute(NULL, NULL, strJavaPathName, NULL, NULL, SW_SHOWNORMAL);
		pThis->setCNDPCDVisible(FALSE);
		//executeWithHide(strCNDPCDPath);
	}
	
	int iTry = 0;
    do
	{
		hWnd = pThis->GetGlobalHandleByTitle(strJavaTitle);
		iTry ++;
		if (iTry == 50)
		{
			MyMessageBox_Error(_T("����δ����"), _T("������ʾ��Ϣ"));
			ProgressBar_Stop();
			return 0;
		}
		Sleep(300);
	}
    while (hWnd == NULL);
	
	
	
	//::SetWindowPos(hWnd, HWND_TOPMOST, 400, 400, 100, 100, SWP_SHOWWINDOW);
	::SetParent(hWnd, pThis->m_hWnd);
	
	pThis->m_hWndJava = hWnd;
	g_hWndJava = hWnd;
	g_hWndMainWindow = pThis->m_hWnd;
	CView *pView = (CView *) pThis->GetActiveView();
	RECT rect, rect2;
	pView->GetClientRect(&rect);
    pView->ClientToScreen (&rect);
	pThis->GetWindowRect(&rect2);
	rect.left = rect.left - rect2.left - 8;
	rect.bottom = rect.bottom - rect2.top + 0;
	rect.right = rect.right - rect2.left - 8;
	rect.top = rect.top - rect2.top + 0;
	int nWidht = GetSystemMetrics(SM_CXVSCROLL);
	int nHeight = GetSystemMetrics(SM_CYVSCROLL);
	//::SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left + nWidht, rect.bottom - rect.top + nHeight, SWP_SHOWWINDOW);
	//::SetWindowPos(hWnd, NULL, rect.left, rect.top, rect.right - rect.left + nWidht, rect.bottom - rect.top + nHeight, SWP_SHOWWINDOW);
	::MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left + nWidht, rect.bottom - rect.top + nHeight, TRUE);
	SetWindowLong(hWnd, GWL_STYLE,WS_CHILD);
	// 	DWORD Style = ::GetWindowLong(hWnd,GWL_EXSTYLE);
	// 	Style = WS_EX_TOOLWINDOW;
	// 	::ShowWindow(hWnd, FALSE);
	// 	::SetWindowLong(hWnd, GWL_EXSTYLE,Style);
	// 	::ShowWindow(hWnd, TRUE);
	// 
	//SetTimer(TIMER_NOTOPMOST, 100, NULL);
	pThis->setCNDPCDVisible(TRUE);
	//::SetForegroundWindow(hWnd);
	//::SetActiveWindow(hWnd);
	::SetParent(hWnd, pThis->m_hWnd);
	ProgressBar_Stop();

	return 1;
}

BOOL CMainFrame::startServer()
{
	if (m_pCurServer == NULL)
	{
		m_pCurServer = new CSocketManager(0);
		//CVisualADSView *pView = (CVisualADSView*) pThis->GetActiveView();
		//pSocketManager->SetMessageWindow(&(pThis->pView->m_editConsole));
		m_pCurServer->SetServerState(TRUE);
	}

	BOOL bSuccess = FALSE;

	if (m_pCurServer != NULL)
	{
		// no smart addressing - we use connection oriented
		m_pCurServer->SetSmartAddressing(FALSE);
		bSuccess = m_pCurServer->CreateSocket(m_strServerIP, m_strPort, AF_INET, SOCK_STREAM, 0); // TCP
		
		if (bSuccess && m_pCurServer->WatchComm())
		{
			/*
			GetDlgItem(IDC_BTN_SEND)->EnableWindow( TRUE );
			GetDlgItem(IDC_BTN_STOP)->EnableWindow( TRUE );
			NextDlgCtrl();
			GetDlgItem(IDC_BTN_START)->EnableWindow( FALSE );
			GetDlgItem(IDC_TCP)->EnableWindow( FALSE );
			GetDlgItem(IDC_UDP)->EnableWindow( FALSE );
			*/
// 			CString strServer, strAddr;
// 			m_pCurServer->GetLocalName(strServer.GetBuffer(256), 256);
// 			strServer.ReleaseBuffer();
// 			if (m_pCurServer->strSelfIP == _T(""))
// 			{
// 				m_pCurServer->GetLocalAddress(strAddr.GetBuffer(256), 256);
// 				strAddr.ReleaseBuffer();
// 			}
// 			else
// 			{
// 				strAddr = m_pCurServer->strSelfIP;
// 			}
// 			CString strMsg = _T("Server: ") + strServer;
// 			strMsg += _T(", No. ") + itos(m_pCurServer->m_iIndex);
// 			strMsg += _T(", @Address: ") + strAddr;
// 			strMsg += _T(" is running on port ") + m_strPort + CString("\r\n");
// 			m_pCurServer->AppendMessage(strMsg);
		}
	}
	
	return bSuccess;
}

void CMainFrame::stopServer()
{
	m_pCurServer->StopComm();
	delete m_pCurServer;
	m_pCurServer = NULL;
}

void CMainFrame::updateServer()
{
	stopServer();
	startServer();
}

void CMainFrame::OnMove(int x, int y) 
{
	CBCGPFrameWnd::OnMove(x, y);
	return;
// 	if (m_hWndJava)
// 	{
// 		CView *pView = (CView *) GetActiveView();
// 		RECT rect, rect2;
// 		pView->GetClientRect(&rect);
// 		pView->ClientToScreen (&rect);
// 		GetWindowRect(&rect2);
// 		rect.left = rect.left - rect2.left - 8;
// 		rect.bottom = rect.bottom - rect2.top + 0;
// 		rect.right = rect.right - rect2.left - 8;
// 		rect.top = rect.top - rect2.top + 0;
// 		int nWidht = GetSystemMetrics(SM_CXVSCROLL);
// 		int nHeight = GetSystemMetrics(SM_CYVSCROLL);
// 		::MoveWindow(m_hWndJava, rect.left, rect.top, rect.right - rect.left + nWidht, rect.bottom - rect.top + nHeight, TRUE);
// 		//::SetWindowPos(m_hWndJava, m_hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
// 	}
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPFrameWnd::OnSize(nType, cx, cy);
	if (m_hWndJava)
	{
		CView *pView = (CView *) GetActiveView();
		RECT rect, rect2;
		pView->GetClientRect(&rect);
		pView->ClientToScreen (&rect);
		GetWindowRect(&rect2);
		rect.left = rect.left - rect2.left - 8;
		rect.bottom = rect.bottom - rect2.top + 0;
		rect.right = rect.right - rect2.left - 8;
		rect.top = rect.top - rect2.top + 0;
		int nWidht = GetSystemMetrics(SM_CXVSCROLL);
		int nHeight = GetSystemMetrics(SM_CYVSCROLL);
		::MoveWindow(m_hWndJava, rect.left, rect.top, rect.right - rect.left + nWidht, rect.bottom - rect.top + nHeight, TRUE);
		//::SetWindowPos(m_hWndJava, NULL, rect.left, rect.top, rect.right - rect.left + nWidht, rect.bottom - rect.top + nHeight, SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CBCGPFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	if (!m_hWndJava)
	{
		return;
	}
	return;

// 	if (WA_ACTIVE == nState || WA_CLICKACTIVE == nState)
// 	{
// 		::SetWindowPos(m_hWndJava, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
// 		::SetActiveWindow(m_hWnd);
// 		iLastToken = iToken;
// 		iToken = 1;
// 	}
// 	else if (WA_INACTIVE == nState)
// 	{
// 		::SetWindowPos(m_hWndJava, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
// 		::SetActiveWindow(m_hWnd);
// 		iLastToken = iToken;
// 		iToken = 0;
// 	}
// 	else
// 	{
// 		
// 	}
}

