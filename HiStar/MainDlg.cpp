// MainDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "UserMsg.h"
extern BOOL g_bLoginCtpT;
// CMainDlg 对话框
extern double AvailCtp;
IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMainDlg::IDD, pParent)
{
}

CMainDlg::~CMainDlg()
{
	if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
		((CHiStarApp*)AfxGetApp())->m_pIBClient->eDisconnect();
		delete ((CHiStarApp*)AfxGetApp())->m_pIBClient;
		((CHiStarApp*)AfxGetApp())->m_pIBClient = NULL;
	}
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_MESSAGE(WM_ORDER_STATUS, &CMainDlg::OnOrderStatus)
	ON_MESSAGE(WM_ERRORS, &CMainDlg::OnErrors)
	ON_MESSAGE_VOID(WM_KICKIDLE,OnKickIdle)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CMainDlg::OnOk)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CMainDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CMainDlg 消息处理程序


BOOL CMainDlg::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();
	//额外初始化代码
	m_tab.InsertItem(0, _T("基本"));
	m_tab.InsertItem(1, _T("账户"));
	m_tab.InsertItem(2,_T("状态"));
	//创建两个对话框
	m_accountPage.Create(IDD_ACCOUNT_PAGE, &m_tab);
	m_basicPage.Create(IDD_BASIC_PAGE, &m_tab);
	m_statusPage.Create(IDD_STATUS_PAGE, &m_tab);

	//设定在Tab内显示的范围
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_basicPage.MoveWindow(&rc);
	m_accountPage.MoveWindow(&rc);
	m_statusPage.MoveWindow(&rc);
	//把对话框对象指针保存起来
	m_pDialog[0] = &m_basicPage;
	m_pDialog[1] = &m_accountPage;
	m_pDialog[2] = &m_statusPage;
	//显示初始页面
	m_pDialog[0]->ShowWindow(SW_SHOW);
	m_pDialog[1]->ShowWindow(false);
	m_pDialog[2]->ShowWindow(false);
	m_tab.SetCurSel(0);
	//保存当前选择
	m_CurSelTab = 0;
	return 0;
}

afx_msg LRESULT CMainDlg::OnOrderStatus(WPARAM wParam, LPARAM lParam)
{
	int i = m_basicPage.m_orderStatus.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_basicPage.m_orderStatus.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}
afx_msg LRESULT CMainDlg::OnErrors(WPARAM wParam, LPARAM lParam)
{
	int i = m_basicPage.m_orderStatus.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_basicPage.m_orderStatus.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}

void CMainDlg::OnOk()
{
	CDialogEx::OnOK();
}

void CMainDlg::addCombInst(void)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	BOOL bRes = FALSE;
	m_basicPage.m_CombInst.ResetContent();//充值清空内容
	AcquireSRWLockShared(&g_srwLock_Insinf);
	for (UINT i=0; i < pApp->m_cT->m_InsinfVec.size();i++)
	{
		if(pApp->m_cT->m_InsinfVec[i].iinf.InstrumentID[0] == 'I' 
			&& pApp->m_cT->m_InsinfVec[i].iinf.InstrumentID[1] == 'F'){
				m_basicPage.m_CombInst.AddString(CString(pApp->m_cT->m_InsinfVec[i].iinf.InstrumentID));
				m_basicPage.m_CombInst.SetWindowText(CString(pApp->m_cT->m_InsinfVec[i].iinf.InstrumentID));
		}
	}
	ReleaseSRWLockShared(&g_srwLock_Insinf);
}


void CMainDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int CurSel = m_tab.GetCurSel();
	switch(CurSel)
	{
	case 0:
		m_CurSelTab = 0;
		m_basicPage.ShowWindow(true);
		m_accountPage.ShowWindow(false);
		m_statusPage.ShowWindow(false);
		break;
	case 1:
		m_CurSelTab = 1;
		m_accountPage.ShowWindow(true);
		m_basicPage.ShowWindow(false);
		m_statusPage.ShowWindow(false);
		break;
	case 2:
		m_CurSelTab = 2;
		m_statusPage.ShowWindow(true);
		m_accountPage.ShowWindow(false);
		m_basicPage.ShowWindow(false);
	}
	*pResult = 0;
}

afx_msg LRESULT CMainDlg::OnRefreshMdPane(WPARAM wParam, LPARAM lParam)
{
	m_basicPage.RefreshMdPane();
	m_accountPage.UpdateAccountPage();
	return 0;
}

void CMainDlg::OnKickIdle()
{
	//static int i;TRACE("i,%d\n",++i);
}
