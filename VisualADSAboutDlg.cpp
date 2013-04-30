// VisualADSAboutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VisualADS.h"
#include "VisualADSAboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVisualADSAboutDlg dialog


CVisualADSAboutDlg::CVisualADSAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVisualADSAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVisualADSAboutDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVisualADSAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVisualADSAboutDlg)
	DDX_Control(pDX, IDC_ABOUT_EDIT, m_edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVisualADSAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CVisualADSAboutDlg)
	ON_BN_CLICKED(IDC_COMPANY_URL, OnCompanyUrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVisualADSAboutDlg message handlers

BOOL CVisualADSAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	SetWindowText(_T("����VisualADS"));
	CString strText;
	strText += _T("VisualADS v1.1.5.15389����Visual C++ 6.0���������BCGControlBar Pro 18.0��E-XD++ 15.80��Graphviz 2.2.8��RacerPro 1.9.2������ɡ�\r\n");
	strText += _T("֧��Windows�汾������汾����Windows XP/Vista/7���������������Windows Server 2003/2008/2008 R2��Windows 2000/NT4/9x�Ȼ����ᱻ�Զ����ԡ�\r\n");
	strText += _T("ʹ�ù������������360����HOSTS�޸ĵ���ʾ��������\r\n");
	strText += _T("\r\n");
	strText += _T("ʹ�÷�����\r\n");
	strText += _T("��ѡ��һ̨������ͬһ�������ļ������װ����������Բ��������ڼ������������������\r\n");
	strText += _T("���ڡ������Ա��¼��������������������ơ������Ա�˻�����󣬵������¼����ť���ȴ�����Ϣ��ͼ�η�ʽ��ʾ����������������϶���\r\n");
	strText += _T("�۵�����������桱��ť�����������߼��������棻\r\n");
	strText += _T("�ܵ������ͻ��⡱��ť���ȴ�һ��ʱ�䣬�����Գ�ͻ����������򽫻���ʾ���д��ڵľ����ͻ������ϸ˵����\r\n");
	strText += _T("�����ͼ�η����ڵ����������й��ڽ��ܣ������á����ģʽ������ġ��༭ͼ��Ԫ�ء����ܣ���ʱ��������ͼ�ε�λ�úʹ�С��\r\n");
	strText += _T("���ڷǱ༭ģʽ�µ����û����顢�������Ԫ��ʱ���Ҳ�������������ʾ�������Ϣ����չ��Ϣ��\r\n");
	strText += _T("�������Ҫ������һ�ε�¼�ͼ�⣬һ��Ҫ�ȵ������ջ��桱��ť������һ�ε���Ϣȫ�������\r\n");
	strText += _T("\r\n");
	strText += _T("����֧������ϵ��veotax@sae.buaa.edu.cn\r\n");
	strText += _T("                                                                                                                  2012/10/31");
	m_edit.SetWindowText(strText);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVisualADSAboutDlg::OnCompanyUrl() 
{
	ShellExecute(NULL, _T("open"), _T("http://www.buaa.edu.cn"), NULL, NULL, SW_MAXIMIZE);
}
